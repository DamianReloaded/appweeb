#include "webserver.hpp"
#include "util.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <format>
#include <wchar.h>
#include <endpoint.hpp>
#include "pluginloader.hpp"

namespace appweeb
{
    constexpr int MAX_PATH = 260;

    WebServer::WebServer(uint16_t port)
        : m_port(port)
    {

    }

    void WebServer::Stop()
    {
        m_running = false;
        m_listener.Close();
    }

    bool WebServer::Run()
    {
        LogLine(std::format( "Web root: {}", m_rootPath.string()));
        LogLine(std::format( "Plugin directory: {}", m_pluginDirectory.string()));
        LogLine(std::format( "HTTP port: {}", m_port));        

        auto plugins =
            m_pluginLoader.LoadPlugins(m_pluginDirectory);

        for (auto& plugin : plugins)
        {
            EndpointRegistration registration;
            registration.Method = plugin->GetMethod();
            registration.Path = plugin->GetPath();
            registration.Endpoint = plugin.get();

            LogLine(std::format(
                "Plugin endpoint registered: {} {}",
                registration.Method,
                registration.Path));

            m_endpoints.push_back(
                std::move(registration));

            m_loadedPlugins.push_back(
                std::move(plugin));                
        }

        LogLine(std::format(
            "Starting HTTP server on port {}.",
                m_port));

        if (!m_listener.Listen(m_port))
        {
            LogLine("Failed to start listener. Already running?");
            return false;
        }

        m_running = true;

        LogLine("Server is listening.");

        while (m_running)
        {
            Socket client = m_listener.Accept();

            if (!client.IsValid())
            {
                continue;
            }

            LogLine("Client connected.");

            std::thread(
                &WebServer::HandleClient,
                this,
                std::move(client)).detach();
        }

        return true;
    }

    std::string WebServer::ReceiveRequest(Socket* client)
    {
        constexpr size_t ChunkSize = 16384;
        std::string request;
        size_t contentLength = 0;
        bool headersParsed = false;

        for (;;)
        {
            char buffer[ChunkSize];
            int bytes = client->Receive(buffer, static_cast<int>(sizeof(buffer)));

            if (bytes <= 0)
            {
                break;
            }

            request.append(buffer, bytes);

            if (!headersParsed)
            {
                auto headerEnd = request.find("\r\n\r\n");
                if (headerEnd == std::string::npos)
                {
                    continue;
                }

                headersParsed = true;
                auto contentLengthPos = request.find("Content-Length:");
                if (contentLengthPos != std::string::npos)
                {
                    contentLengthPos += 15;
                    while (contentLengthPos < request.size() && request[contentLengthPos] == ' ')
                    {
                        ++contentLengthPos;
                    }
                    auto lineEnd = request.find("\r\n", contentLengthPos);
                    contentLength =static_cast<size_t>(std::stoull(request.substr(contentLengthPos, lineEnd - contentLengthPos)));
                }
                else
                {
                    return request;
                }

                auto bodyStart = headerEnd + 4;
                size_t bodySize = request.size() - bodyStart;
                if (bodySize >= contentLength)
                {
                    return request;
                }
            }

            if (headersParsed)
            {
                auto bodyStart = request.find("\r\n\r\n");
                bodyStart += 4;
                size_t bodySize = request.size() - bodyStart;
                if (bodySize >= contentLength)
                {
                    break;
                }
            }
        }

        return request;
    }

    static size_t FindCaseInsensitive(
        const std::string& text,
        const std::string& value,
        size_t startPosition = 0)
    {
        if (value.empty())
        {
            return startPosition;
        }

        if (text.size() < value.size())
        {
            return std::string::npos;
        }

        for (size_t i = startPosition; i <= text.size() - value.size(); ++i)
        {
            bool matches = true;

            for (size_t j = 0; j < value.size(); ++j)
            {
                if (std::tolower(static_cast<unsigned char>(text[i + j])) !=
                    std::tolower(static_cast<unsigned char>(value[j])))
                {
                    matches = false;
                    break;
                }
            }

            if (matches)
            {
                return i;
            }
        }

        return std::string::npos;
    }

    static std::string ExtractJsonValue(const std::string& text, const std::string& key)
    {
        std::string pattern = "\"" + key + "\"";
        auto pos = FindCaseInsensitive(text, pattern);
        if (pos == std::string::npos)
        {
            return {};
        }

        pos = text.find(':', pos);
        if (pos == std::string::npos)
        {
            return {};
        }

        pos++;

        // skip whitespace
        while (pos < text.size() &&
            (text[pos] == ' ' || text[pos] == '\t' || text[pos] == '\n' || text[pos] == '\r'))
        {
            pos++;
        }

        bool quoted = false;

        if (pos < text.size() && text[pos] == '"')
        {
            quoted = true;
            pos++;
        }

        size_t end = pos;

        if (quoted)
        {
            end = text.find('"', pos);
        }
        else
        {
            while (end < text.size())
            {
                char c = text[end];

                if (c == ',' || c == '}' || c == '\n' || c == '\r')
                {
                    break;
                }

                end++;
            }
        }

        if (end == std::string::npos || end <= pos)
        {
            return {};
        }

        return text.substr(pos, end - pos);
    }

    void WebServer::SendJsonResponse(Socket* client, bool success, std::string_view error)
    {
        std::string json;

        if (success)
        {
            json = R"({"success":true})";
        }
        else
        {
            json = "{\"success\":false," "\"error\":\"" + std::string(error) + "\"}";
        }

        SendResponse(client, 200, "application/json", json.data(), json.size());
    }

    void WebServer::HandleClient(Socket client)
    {
        std::string request = ReceiveRequest(&client);

        if (request.empty())
        {
            return;
        }

        auto lineEnd = request.find("\r\n");

        if (lineEnd == std::string::npos)
        {
            return;
        }

        std::string firstLine = request.substr(0, lineEnd);

        auto firstSpace = firstLine.find(' ');
        auto secondSpace = firstLine.find(' ', firstSpace + 1);

        if (firstSpace == std::string::npos ||
            secondSpace == std::string::npos)
        {
            return;
        }

        std::string method = firstLine.substr(0, firstSpace);
        std::string url = firstLine.substr(firstSpace + 1, secondSpace - firstSpace - 1);

        auto bodyPos = request.find("\r\n\r\n");

        std::string body;

        if (bodyPos != std::string::npos)
        {
            body = request.substr(bodyPos + 4);
        }

        //
        // Give every registered endpoint a chance to handle the request.
        //
        for (auto& endpoint : m_endpoints)
        {
            if (endpoint.Method != method)
            {
                continue;
            }

            if (endpoint.Path != url)
            {
                continue;
            }

            if (endpoint.Endpoint->HandleRequest(this, &client, request, body))
            {
                return;
            }
        }

        //
        // Static file handling
        //
        auto path = ResolvePath(url);

        if (path.empty() || !std::filesystem::exists(path))
        {
            constexpr char text[] = "{\"HttpError\":\"404 Not Found\"}";

            SendResponse(
                &client,
                404,
                "application/json",
                text,
                sizeof(text) - 1);

            return;
        }

        auto data = ReadFile(path);

        SendResponse(
            &client,
            200,
            GetMimeType(path),
            data.data(),
            data.size());
    }

    std::filesystem::path WebServer::ResolvePath(std::string_view url)
    {
        auto root = std::filesystem::weakly_canonical(m_rootPath);

        std::string relative(url);

        if (relative.empty() || relative == "/")
        {
            relative = "/index.html";
        }

        while (!relative.empty() && relative.front() == '/')
        {
            relative.erase(relative.begin());
        }

        auto candidate = std::filesystem::weakly_canonical(root / relative);
        auto relativePath = std::filesystem::relative(candidate, root);
        if (relativePath.empty() || relativePath.string().starts_with(".."))
        {
            return {};
        }

        return candidate;
    }

    std::vector<std::byte> WebServer::ReadFile(const std::filesystem::path& path)
    {
        std::ifstream file(path, std::ios::binary);
        if (!file)
        {
            return {};
        }
        file.seekg(0, std::ios::end);
        auto size = static_cast<size_t>(file.tellg());
        file.seekg(0, std::ios::beg);
        std::vector<std::byte> data(size);
        file.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(size));
        return data;
    }

    std::string WebServer::GetMimeType(const std::filesystem::path& path)
    {
        auto ext = path.extension().string();

        if (ext == ".html")
        {
            return "text/html; charset=utf-8";
        }

        if (ext == ".css")
        {
            return "text/css; charset=utf-8";
        }

        if (ext == ".js")
        {
            return "application/javascript; charset=utf-8";
        }

        if (ext == ".json")
        {
            return "application/json; charset=utf-8";
        }

        if (ext == ".png")
        {
            return "image/png";
        }

        if (ext == ".jpg" || ext == ".jpeg")
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

    void WebServer::SendResponse(Socket* client, int statusCode, std::string_view contentType, const void* data, size_t size)
    {
        std::ostringstream header;
        header << "HTTP/1.1 " << statusCode << (statusCode == 200 ? " OK" : " Not Found") << "\r\n";
        header << "Content-Type: " << contentType << "\r\n";
        header << "Content-Length: " << size << "\r\n";
        header << "Connection: close\r\n";
        header << "\r\n";
        auto headerText = header.str();
        client->Send(headerText.data(),static_cast<int>(headerText.size()));
        if (size > 0)
        {
            client->Send(data, static_cast<int>(size));
        }
    }

    std::string WebServer::ReadTextFile(const std::filesystem::path& path)
    {
        std::ifstream file(path);
        if (!file)
        {
            return {};
        }
        return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    }

    void WebServer::LoadConfig()
    {
        auto root =
            m_rootPath.empty()
            ? std::filesystem::weakly_canonical(GetRootPath())
            : std::filesystem::weakly_canonical(m_rootPath);

        auto configPath = root / "config.json";

        if (!std::filesystem::exists(configPath))
        {
            return;
        }

        auto configText = ReadTextFile(configPath);

        //
        // wwwroot
        //
        auto wwwroot = ExtractJsonValue(
            configText,
            "wwwroot");

        if (!wwwroot.empty())
        {
            auto configuredRoot =
                std::filesystem::weakly_canonical(root / wwwroot);

            std::error_code ec;

            std::filesystem::create_directories(
                configuredRoot,
                ec);

            if (ec)
            {
                m_rootPath = root;

                LogLine(std::format("Failed to create wwwroot: {} | using application directory instead.", configuredRoot.string()));
            }
            else
            {
                m_rootPath = configuredRoot;
            }
        }

        //
        // plugindir
        //
        auto pluginDir = ExtractJsonValue(
            configText,
            "plugindir");

        if (!pluginDir.empty())
        {
            m_pluginDirectory =
                std::filesystem::weakly_canonical(root / pluginDir);

            std::error_code ec;

            std::filesystem::create_directories(
                m_pluginDirectory,
                ec);

            if (ec)
            {
                LogLine(std::format("Failed to create plugin directory: {}.", m_pluginDirectory.string()));
            }
        }

        //
        // httpport
        //
        auto portStr = ExtractJsonValue(
            configText,
            "httpport");

        if (!portStr.empty())
        {
            try
            {
                int port = std::stoi(portStr);

                if (port > 0 && port <= 65535)
                {
                    m_port = static_cast<uint16_t>(port);
                }
            }
            catch (...)
            {
                // ignore invalid values
            }
        }
    }

    void WebServer::SetRootPath(std::filesystem::path rootPath)
    {
        m_rootPath = std::filesystem::weakly_canonical(std::move(rootPath));
    }

    std::filesystem::path WebServer::GetPluginDirectory()
    {
        return m_pluginDirectory;
    }

    std::filesystem::path WebServer::GetRootPath()
    {
        return m_rootPath;
    }

    uint16_t WebServer::GetHttpPort()
    {
        return m_port;
    }

    bool WebServer::WriteBinaryFile(const std::filesystem::path& path, const void* data, size_t size)
    {
        std::filesystem::create_directories(path.parent_path());
        std::ofstream file(path, std::ios::binary);
        if (!file)
        {
            return false;
        }
        file.write(static_cast<const char*>(data), static_cast<std::streamsize>(size));
        return file.good();
    }

    std::string WebServer::GetHeaderValue(std::string_view request, std::string_view headerName)
    {
        auto headerEnd = request.find("\r\n\r\n");
        if (headerEnd == std::string_view::npos)
        {
            return {};
        }

        std::string target;
        target.reserve(headerName.size());
        for (char c : headerName)
        {
            target.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
        }

        size_t lineStart = 0;
        while (lineStart < headerEnd)
        {
            auto lineEnd = request.find("\r\n", lineStart);
            if (lineEnd == std::string_view::npos || lineEnd > headerEnd)
            {
                break;
            }

            auto colonPos = request.find(':', lineStart);
            if (colonPos != std::string_view::npos && colonPos < lineEnd)
            {
                std::string currentName;
                currentName.reserve(colonPos - lineStart);
                for (size_t i = lineStart; i < colonPos; ++i)
                {
                    currentName.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(request[i]))));
                }

                if (currentName == target)
                {
                    size_t valueStart = colonPos + 1;
                    while (valueStart < lineEnd && std::isspace(static_cast<unsigned char>(request[valueStart])))
                    {
                        ++valueStart;
                    }
                    return std::string(request.substr(valueStart, lineEnd - valueStart));
                }
            }
            lineStart = lineEnd + 2;
        }

        return {};
    }

    void WebServer::LogLine(const std::string& text)
    {
        std::cout << text << "\n";
    }
}