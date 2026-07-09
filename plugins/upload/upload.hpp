#pragma once

#include <endpoint.hpp>

namespace appweeb
{
    class UploadEndpoint : public IEndpoint
    {
    public:

        const char* GetMethod() const override;
        const char* GetPath() const override;

        bool HandleRequest(
            IWebServer* webServer,
            Socket* client,
            std::string_view request,
            std::string_view body) override;
    };
}

extern "C"
{
    appweeb::IEndpoint* CreateEndpoint()
    {
        auto endpoint = new appweeb::UploadEndpoint();
        return endpoint;
    }

    void DestroyEndpoint(appweeb::IEndpoint* endpoint)
    {
        delete endpoint;
    }
}