#ifdef _WIN32

#include "../../pluginloader.hpp"

#include <windows.h>
#include <iostream>

namespace appweeb
{
    PluginLoader::~PluginLoader()
    {
        for (void* handle : m_handles)
        {
            FreeLibrary(
                static_cast<HMODULE>(handle));
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

            if (entry.path().extension() != ".dll")
            {
                continue;
            }

            HMODULE module = LoadLibraryW(
                entry.path().wstring().c_str());

            if (!module)
            {
                continue;
            }

            auto createEndpoint =
                reinterpret_cast<CreateEndpointFunction>(
                    reinterpret_cast<void*>(GetProcAddress(
                        module,
                        "CreateEndpoint")));

            if (!createEndpoint)
            {
                FreeLibrary(module);
                continue;
            }

            auto destroyEndpoint =
                reinterpret_cast<DestroyEndpointFunction>(
                    reinterpret_cast<void*>(GetProcAddress(
                        module,
                        "DestroyEndpoint")));

            if (!destroyEndpoint)
            {
                FreeLibrary(module);
                continue;
            }

            EndpointPtr endpoint(
                createEndpoint(),
                destroyEndpoint);

            if (!endpoint)
            {
                FreeLibrary(module);
                continue;
            }

            m_handles.push_back(
                reinterpret_cast<void*>(module));

            endpoints.push_back(
                std::move(endpoint));
        }

        return endpoints;
    }
}

#endif