#include "webserver.hpp"

#define WIN32_LEAN_AND_MEAN

#include <windows.h>

#include <fstream>
#include <sstream>

WebServer::WebServer(
    uint16_t port)
    : m_port(port)
{
}

void WebServer::Stop()
{
    m_running = false;

    m_listener.Close();
}

void WebServer::Run()
{
    if (!m_listener.Listen(m_port))
    {
        return;
    }

    m_running = true;

    while (m_running)
    {
        Socket client =
            m_listener.Accept();

        if (!m_running)
        {
            break;
        }

        if (!client.IsValid())
        {
            continue;
        }

        HandleClient(
            std::move(client));
    }
}

std::string WebServer::ReceiveRequest(
    Socket& client)
{
    std::string request;

    constexpr size_t ChunkSize =
        16384;

    size_t contentLength = 0;
    bool headersParsed = false;

    for (;;)
    {
        char buffer[ChunkSize];

        int bytes =
            client.Receive(
                buffer,
                static_cast<int>(
                    sizeof(buffer)));

        if (bytes <= 0)
        {
            break;
        }

        request.append(
            buffer,
            bytes);

        if (!headersParsed)
        {
            auto headerEnd =
                request.find(
                    "\r\n\r\n");

            if (
                headerEnd ==
                std::string::npos)
            {
                continue;
            }

            headersParsed = true;

            auto contentLengthPos =
                request.find(
                    "Content-Length:");

            if (
                contentLengthPos !=
                std::string::npos)
            {
                contentLengthPos += 15;

                while (
                    contentLengthPos <
                    request.size() &&
                    request[
                        contentLengthPos] == ' ')
                {
                    ++contentLengthPos;
                }

                auto lineEnd =
                    request.find(
                        "\r\n",
                        contentLengthPos);

                contentLength =
                    static_cast<size_t>(
                        std::stoull(
                            request.substr(
                                contentLengthPos,
                                lineEnd -
                                contentLengthPos)));
            }
            else
            {
                return request;
            }
        }

        if (headersParsed)
        {
            auto bodyStart =
                request.find(
                    "\r\n\r\n");

            bodyStart += 4;

            size_t bodySize =
                request.size() -
                bodyStart;

            if (
                bodySize >=
                contentLength)
            {
                break;
            }
        }
    }

    return request;
}

std::string WebServer::ExtractJsonString(
    std::string_view json,
    std::string_view property)
{
    std::string key =
        "\"" +
        std::string(property) +
        "\"";

    auto keyPos =
        json.find(key);

    if (
        keyPos ==
        std::string_view::npos)
    {
        return {};
    }

    auto colonPos =
        json.find(
            ':',
            keyPos);

    if (
        colonPos ==
        std::string_view::npos)
    {
        return {};
    }

    auto quotePos =
        json.find(
            '"',
            colonPos);

    if (
        quotePos ==
        std::string_view::npos)
    {
        return {};
    }

    ++quotePos;

    std::string result;

    bool escape = false;

    for (
        size_t i = quotePos;
        i < json.size();
        ++i)
    {
        char c =
            json[i];

        if (escape)
        {
            switch (c)
            {
                case '\\':
                    result += '\\';
                    break;

                case '"':
                    result += '"';
                    break;

                case 'n':
                    result += '\n';
                    break;

                case 'r':
                    result += '\r';
                    break;

                case 't':
                    result += '\t';
                    break;

                default:
                    result += c;
                    break;
            }

            escape = false;
            continue;
        }

        if (c == '\\')
        {
            escape = true;
            continue;
        }

        if (c == '"')
        {
            break;
        }

        result += c;
    }

    return result;
}

bool WebServer::WriteTextFile(
    const std::filesystem::path& path,
    std::string_view content)
{
    std::filesystem::create_directories(
        path.parent_path());

    std::ofstream file(
        path,
        std::ios::binary);

    if (!file)
    {
        return false;
    }

    file.write(
        content.data(),
        static_cast<std::streamsize>(
            content.size()));

    return file.good();
}

void WebServer::SendJsonResponse(
    Socket& client,
    bool success,
    std::string_view error)
{
    std::string json;

    if (success)
    {
        json =
            R"({"success":true})";
    }
    else
    {
        json =
            "{\"success\":false,"
            "\"error\":\"" +
            std::string(error) +
            "\"}";
    }

    SendResponse(
        client,
        200,
        "application/json",
        json.data(),
        json.size());
}

void WebServer::HandleClient(
    Socket client)
{
    std::string request =
        ReceiveRequest(client);

    if (request.empty())
    {
        return;
    }

    auto lineEnd =
        request.find("\r\n");

    if (lineEnd == std::string::npos)
    {
        return;
    }

    std::string firstLine =
        request.substr(
            0,
            lineEnd);

    auto firstSpace =
        firstLine.find(' ');

    auto secondSpace =
        firstLine.find(
            ' ',
            firstSpace + 1);

    if (
        firstSpace == std::string::npos ||
        secondSpace == std::string::npos)
    {
        return;
    }

    std::string method =
        firstLine.substr(
            0,
            firstSpace);

    std::string url =
        firstLine.substr(
            firstSpace + 1,
            secondSpace - firstSpace - 1);

    auto bodyPos =
        request.find("\r\n\r\n");

    std::string body;

    if (bodyPos != std::string::npos)
    {
        body =
            request.substr(
                bodyPos + 4);
    }

    if (
        method == "POST" &&
        url == "/api/write-json")
    {
        auto relativePath =
            ExtractJsonString(
                body,
                "path");

        auto jsonText =
            ExtractJsonString(
                body,
                "json");

        auto path =
            ResolvePath(
                relativePath);

        if (path.empty())
        {
            SendJsonResponse(
                client,
                false,
                "Invalid path");

            return;
        }

        if (
            WriteTextFile(
                path,
                jsonText))
        {
            SendJsonResponse(
                client,
                true);

            return;
        }

        SendJsonResponse(
            client,
            false,
            "Failed to write file");

        return;
    }

    auto path =
        ResolvePath(url);

    if (
        path.empty() ||
        !std::filesystem::exists(path))
    {
        constexpr char text[] =
            "404 Not Found";

        SendResponse(
            client,
            404,
            "text/plain",
            text,
            sizeof(text) - 1);

        return;
    }

    auto data =
        ReadFile(path);

    SendResponse(
        client,
        200,
        GetMimeType(path),
        data.data(),
        data.size());
}

std::filesystem::path WebServer::GetRootPath()
{
    wchar_t buffer[MAX_PATH];

    GetModuleFileNameW(
        nullptr,
        buffer,
        MAX_PATH);

    return
        std::filesystem::path(buffer)
        .parent_path();
}

std::filesystem::path WebServer::ResolvePath(
    std::string_view url)
{
    auto root =
        std::filesystem::weakly_canonical(
            GetRootPath());

    std::string relative(url);

    if (
        relative.empty() ||
        relative == "/")
    {
        relative =
            "/index.html";
    }

    while (
        !relative.empty() &&
        relative.front() == '/')
    {
        relative.erase(
            relative.begin());
    }

    auto candidate =
        std::filesystem::weakly_canonical(
            root / relative);

    auto candidateString =
        candidate.wstring();

    auto rootString =
        root.wstring();

    if (
        candidateString.rfind(
            rootString,
            0) != 0)
    {
        return {};
    }

    return candidate;
}

std::vector<std::byte> WebServer::ReadFile(
    const std::filesystem::path& path)
{
    std::ifstream file(
        path,
        std::ios::binary);

    if (!file)
    {
        return {};
    }

    file.seekg(
        0,
        std::ios::end);

    auto size =
        static_cast<size_t>(
            file.tellg());

    file.seekg(
        0,
        std::ios::beg);

    std::vector<std::byte> data(
        size);

    file.read(
        reinterpret_cast<char*>(
            data.data()),
        static_cast<std::streamsize>(
            size));

    return data;
}

std::string WebServer::GetMimeType(
    const std::filesystem::path& path)
{
    auto ext =
        path.extension().string();

    if (ext == ".html")
    {
        return "text/html";
    }

    if (ext == ".css")
    {
        return "text/css";
    }

    if (ext == ".js")
    {
        return "application/javascript";
    }

    if (ext == ".json")
    {
        return "application/json";
    }

    if (ext == ".png")
    {
        return "image/png";
    }

    if (ext == ".jpg" ||
        ext == ".jpeg")
    {
        return "image/jpeg";
    }

    if (ext == ".gif")
    {
        return "image/gif";
    }

    if (ext == ".svg")
    {
        return "image/svg+xml";
    }

    if (ext == ".ico")
    {
        return "image/x-icon";
    }

    return "application/octet-stream";
}

void WebServer::SendResponse(
    Socket& client,
    int statusCode,
    std::string_view contentType,
    const void* data,
    size_t size)
{
    std::ostringstream header;

    header
        << "HTTP/1.1 "
        << statusCode
        << (statusCode == 200
            ? " OK"
            : " Not Found")
        << "\r\n";

    header
        << "Content-Type: "
        << contentType
        << "\r\n";

    header
        << "Content-Length: "
        << size
        << "\r\n";

    header
        << "Connection: close\r\n";

    header
        << "\r\n";

    auto headerText =
        header.str();

    client.Send(
        headerText.data(),
        static_cast<int>(
            headerText.size()));

    if (size > 0)
    {
        client.Send(
            data,
            static_cast<int>(
                size));
    }
}