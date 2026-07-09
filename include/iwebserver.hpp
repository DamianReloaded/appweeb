#pragma once

#include <filesystem>
#include <string>
#include <string_view>

namespace appweeb
{
    class Socket;

    class IWebServer
    {
    public:

        virtual ~IWebServer() = default;

        virtual std::string GetHeaderValue(
            std::string_view request,
            std::string_view headerName) = 0;

        virtual std::filesystem::path ResolvePath(
            std::string_view url) = 0;

        virtual bool WriteBinaryFile(
            const std::filesystem::path& path,
            const void* data,
            size_t size) = 0;

        virtual void SendJsonResponse(
            Socket* client,
            bool success,
            std::string_view error) = 0;

        virtual void LogLine(const std::string& text) = 0;
    };
}