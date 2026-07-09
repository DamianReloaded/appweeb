#include "upload.hpp"
#include <iwebserver.hpp>
#include <socket.hpp>
namespace appweeb
{
    const char* UploadEndpoint::GetMethod() const 
    {
        return "POST";
    }

    const char* UploadEndpoint::GetPath() const 
    {
        return "/api/upload";
    }

    bool UploadEndpoint::HandleRequest(
        IWebServer* webServer,
        Socket* client,
        std::string_view request,
        std::string_view body)
    {
        auto relativePath = webServer->GetHeaderValue(
            request,
            "X-Path");

        auto path = webServer->ResolvePath(relativePath);

        if (path.empty())
        {
            webServer->SendJsonResponse(
                client,
                false,
                "Invalid path");

            return true;
        }

        bool success = webServer->WriteBinaryFile(
            path,
            body.data(),
            body.size());

        webServer->SendJsonResponse(
            client,
            success,
            success ? "" : "Failed to write file");

        return true;
    }
}