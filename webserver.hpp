#pragma once

#include "socket.hpp"

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

class WebServer
{
public:

    explicit WebServer(
        uint16_t port);

    void Run();

private:

    static std::filesystem::path GetRootPath();

    static std::filesystem::path ResolvePath(
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

    static std::string ReceiveRequest(
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

    uint16_t m_port;
};