#pragma once

#include "socket.hpp"
#include "pluginloader.hpp"
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <iwebserver.hpp>

namespace appweeb
{
    class IEndpoint;
    
    class WebServer : public IWebServer
    {
    public:

        explicit WebServer(uint16_t port);

        void Run();

        void Stop();

        void LoadConfig();

        void SetRootPath(std::filesystem::path rootPath);

        std::filesystem::path GetRootPath();
        std::filesystem::path GetPluginDirectory();

        uint16_t GetHttpPort();

        std::filesystem::path ResolvePath(std::string_view url);

        bool WriteBinaryFile(
            const std::filesystem::path& path,
            const void* data,
            size_t size);

        std::string GetHeaderValue(
            std::string_view request,
            std::string_view headerName);

        void SendJsonResponse(
            Socket* client,
            bool success,
            std::string_view error = {});

        static std::vector<std::byte> ReadFile(const std::filesystem::path& path);

        static std::string GetMimeType(const std::filesystem::path& path);

        static void SendResponse(
            Socket* client,
            int statusCode,
            std::string_view contentType,
            const void* data,
            size_t size);

        void HandleClient(Socket client);

        std::string ReceiveRequest(Socket* client);

        std::string ReadTextFile(const std::filesystem::path& path);

        void LogLine(const std::string& text) override;

    private:

        struct EndpointRegistration
        {
            std::string Method;
            std::string Path;
            IEndpoint* Endpoint;
        };

    private:

        uint16_t m_port;
        Socket m_listener;
        bool m_running = true;
        std::filesystem::path m_rootPath;
        std::filesystem::path m_pluginDirectory;

        PluginLoader m_pluginLoader;
        std::vector<EndpointPtr> m_loadedPlugins;
        std::vector<EndpointRegistration> m_endpoints;
    };
}