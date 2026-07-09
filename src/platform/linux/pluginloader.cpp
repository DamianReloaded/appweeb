#ifndef _WIN32

#include "../../pluginloader.hpp"

#include <dlfcn.h>

namespace appweeb
{
    PluginLoader::~PluginLoader()
    {
        for (void* handle : m_handles)
        {
            dlclose(handle);
        }
    }

    std::vector<EndpointPtr> PluginLoader::LoadPlugins(
        const std::filesystem::path& directory)
    {
        std::vector<EndpointPtr> endpoints;

        if (!std::filesystem::exists(directory))
        {
            return endpoints;
        }

        for (const auto& entry : std::filesystem::directory_iterator(directory))
        {
            if (!entry.is_regular_file())
            {
                continue;
            }

            if (entry.path().extension() != ".so")
            {
                continue;
            }

            void* module = dlopen(
                entry.path().c_str(),
                RTLD_NOW);

            if (!module)
            {
                continue;
            }

            auto createEndpoint =
                reinterpret_cast<CreateEndpointFunction>(
                    dlsym(
                        module,
                        "CreateEndpoint"));

            if (!createEndpoint)
            {
                dlclose(module);
                continue;
            }

            auto destroyEndpoint =
                reinterpret_cast<DestroyEndpointFunction>(
                    dlsym(
                        module,
                        "DestroyEndpoint"));

            if (!destroyEndpoint)
            {
                dlclose(module);
                continue;
            }

            EndpointPtr endpoint(
                createEndpoint(),
                destroyEndpoint);

            if (!endpoint)
            {
                dlclose(module);
                continue;
            }

            m_handles.push_back(
                module);

            endpoints.push_back(
                std::move(endpoint));
        }

        return endpoints;
    }
}

#endif