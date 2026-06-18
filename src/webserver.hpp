#pragma once

#include "socket.hpp"

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace appweeb
{
    class WebServer
    {
    public:

        explicit WebServer(
            uint16_t port);

        void Run();
        void Stop();
        
        void SetRootPath(std::filesystem::path rootPath);

    private:

        std::filesystem::path ResolvePath(
            std::string_view url);

        static std::vector<std::byte> ReadFile(
            const std::filesystem::path& path);

        static std::string GetMimeType(
            const std::filesystem::path& path);

        static void SendResponse(
            Socket& client,
            int statusCode,
            std::string_view contentType,
            const void* data,
            size_t size);

        void HandleClient(
            Socket client);

        std::string ReceiveRequest(
            Socket& client);

        static std::string ExtractJsonString(
            std::string_view json,
            std::string_view property);

        static bool WriteTextFile(
            const std::filesystem::path& path,
            std::string_view content);

        static void SendJsonResponse(
            Socket& client,
            bool success,
            std::string_view error = {});

        std::filesystem::path LoadRootPath();

        std::string ReadTextFile(
            const std::filesystem::path& path);

        bool WriteBinaryFile(
            const std::filesystem::path& path,
            const void* data,
            size_t size);

        std::string GetHeaderValue(
            std::string_view request,
            std::string_view headerName);

        uint16_t m_port;
        Socket m_listener;
        bool m_running = true;
        std::filesystem::path m_rootPath;
    };
}