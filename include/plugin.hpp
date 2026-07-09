#pragma once

#include "endpoint.hpp"

#include <memory>

namespace appweeb
{
    using CreateEndpointFunction = IEndpoint* (*)();

    using DestroyEndpointFunction = void (*)(IEndpoint*);


    struct EndpointDeleter
    {
        DestroyEndpointFunction Destroy = nullptr;

        void operator()(IEndpoint* endpoint) const
        {
            if (Destroy && endpoint)
            {
                Destroy(endpoint);
            }
        }
    };
}