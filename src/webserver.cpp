#include "webserver.hpp"
#include "util.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <wchar.h>

namespace appweeb
{

    constexpr int MAX_PATH = 260;

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
        m_rootPath = LoadRootPath();

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
        std::cout
            << "Waiting for recv...\n";

        char buffer[ChunkSize];

        int bytes =
            client.Receive(
                buffer,
                static_cast<int>(
                    sizeof(buffer)));

        std::cout
            << "recv returned "
            << bytes
            << '\n';

        if (bytes <= 0)
        {
            break;
        }

        request.append(
            buffer,
            bytes);

        std::cout
            << "Request size: "
            << request.size()
            << '\n';

        std::cout << request <<std::endl;

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

            std::cout
                << "Headers complete\n";

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

                std::cout
                    << "Content-Length: "
                    << contentLength
                    << '\n';
            }
            else
            {
                std::cout
                    << "No Content-Length header\n";

                return request;
            }

            auto bodyStart =
                headerEnd + 4;

            size_t bodySize =
                request.size() -
                bodyStart;

            std::cout
                << "Initial body size: "
                << bodySize
                << '\n';

            if (bodySize==0)
            {
                
            }

            if (
                bodySize >=
                contentLength)
            {
                std::cout
                    << "Request complete\n";

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

            std::cout
                << "Body size: "
                << bodySize
                << " / "
                << contentLength
                << '\n';

            if (
                bodySize >=
                contentLength)
            {
                std::cout
                    << "Request complete\n";

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
            ReceiveRequest(
                client);

        if (request.empty())
        {
            return;
        }

        auto lineEnd =
            request.find(
                "\r\n");

        if (lineEnd == std::string::npos)
        {
            return;
        }

        std::string firstLine =
            request.substr(
                0,
                lineEnd);

        std::cout
            << "Request: "
            << firstLine
            << '\n';

        auto firstSpace =
            firstLine.find(
                ' ');

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
            request.find(
                "\r\n\r\n");

        std::string body;

        if (bodyPos != std::string::npos)
        {
            body =
                request.substr(
                    bodyPos + 4);
        }

        //
        // POST /api/write-json
        //
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

        //
        // POST /api/write-file
        //
        if (
            method == "POST" &&
            url == "/api/write-file")
        {
            auto relativePath =
                GetHeaderValue(
                    request,
                    "X-Path");

            std::cout
                << "Upload path: ["
                << relativePath
                << "]\n";

            std::cout
                << "Upload size: "
                << body.size()
                << " bytes\n";

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

            std::cout
                << "Resolved path: "
                << path.string()
                << '\n';

            if (
                WriteBinaryFile(
                    path,
                    body.data(),
                    body.size()))
            {
                std::cout
                    << "Sending upload response\n";

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

        //
        // Static file handling
        //
        auto path =
            ResolvePath(
                url);

        if (
            path.empty() ||
            !std::filesystem::exists(
                path))
        {
            constexpr char text[] =
                "{\"HttpError\":\"404 Not Found\"}";

            SendResponse(
                client,
                404,
                "application/json",
                text,
                sizeof(text) - 1);

            return;
        }

        auto data =
            ReadFile(
                path);

        SendResponse(
            client,
            200,
            GetMimeType(
                path),
            data.data(),
            data.size());
    }

    std::filesystem::path WebServer::ResolvePath(
        std::string_view url)
    {
        auto root =
            std::filesystem::weakly_canonical(
                m_rootPath);

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

        auto relativePath =
            std::filesystem::relative(
                candidate,
                root);

        if (
            relativePath.empty() ||
            relativePath.string().starts_with(".."))
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

    std::string WebServer::ReadTextFile(
        const std::filesystem::path& path)
    {
        std::ifstream file(path);

        if (!file)
        {
            return {};
        }

        return
            std::string(
                std::istreambuf_iterator<char>(file),
                std::istreambuf_iterator<char>());
    }

    std::filesystem::path WebServer::LoadRootPath()
    {
        auto root =
            m_rootPath.empty()
                ? std::filesystem::weakly_canonical(
                    GetRootPath())
                : std::filesystem::weakly_canonical(
                    m_rootPath);

        auto configPath =
            root /
            "config.json";

        if (!std::filesystem::exists(configPath))
        {
            return root;
        }

        auto configText =
            ReadTextFile(
                configPath);

        auto wwwroot =
            ExtractJsonString(
                configText,
                "wwwroot");

        if (wwwroot.empty())
        {
            return root;
        }

        auto configuredRoot =
            std::filesystem::weakly_canonical(
                root /
                wwwroot);

        if (!std::filesystem::exists(configuredRoot))
        {
            return root;
        }

        return configuredRoot;
    }

    void WebServer::SetRootPath(
        std::filesystem::path rootPath)
    {
        m_rootPath =
            std::filesystem::weakly_canonical(
                std::move(rootPath));
    }

    bool WebServer::WriteBinaryFile(
        const std::filesystem::path& path,
        const void* data,
        size_t size)
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
            static_cast<const char*>(data),
            static_cast<std::streamsize>(size));

        return file.good();
    }

    std::string WebServer::GetHeaderValue(
        std::string_view request,
        std::string_view headerName)
    {
        auto headerEnd =
            request.find(
                "\r\n\r\n");

        if (headerEnd == std::string_view::npos)
        {
            return {};
        }

        std::string target;

        target.reserve(
            headerName.size());

        for (char c : headerName)
        {
            target.push_back(
                static_cast<char>(
                    std::tolower(
                        static_cast<unsigned char>(c))));
        }

        size_t lineStart = 0;

        while (lineStart < headerEnd)
        {
            auto lineEnd =
                request.find(
                    "\r\n",
                    lineStart);

            if (
                lineEnd == std::string_view::npos ||
                lineEnd > headerEnd)
            {
                break;
            }

            auto colonPos =
                request.find(
                    ':',
                    lineStart);

            if (
                colonPos != std::string_view::npos &&
                colonPos < lineEnd)
            {
                std::string currentName;

                currentName.reserve(
                    colonPos - lineStart);

                for (
                    size_t i = lineStart;
                    i < colonPos;
                    ++i)
                {
                    currentName.push_back(
                        static_cast<char>(
                            std::tolower(
                                static_cast<unsigned char>(
                                    request[i]))));
                }

                if (currentName == target)
                {
                    size_t valueStart =
                        colonPos + 1;

                    while (
                        valueStart < lineEnd &&
                        std::isspace(
                            static_cast<unsigned char>(
                                request[valueStart])))
                    {
                        ++valueStart;
                    }

                    return std::string(
                        request.substr(
                            valueStart,
                            lineEnd - valueStart));
                }
            }

            lineStart =
                lineEnd + 2;
        }

        return {};
    }
}