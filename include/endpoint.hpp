#pragma once

#include <memory>
#include <string_view>

namespace appweeb
{
    class IWebServer;
    class Socket;

    class IEndpoint
    {
    public:

        virtual ~IEndpoint() = default;

        virtual const char* GetMethod() const = 0;
        virtual const char* GetPath() const = 0;
        
        virtual bool HandleRequest(
            IWebServer* webServer,
            Socket* client,
            std::string_view request,
            std::string_view body) = 0;
    };


    using CreateEndpointFunction =
        IEndpoint* (*)();


    using DestroyEndpointFunction =
        void (*)(IEndpoint*);


    using EndpointPtr =
        std::unique_ptr<IEndpoint, DestroyEndpointFunction>;
}