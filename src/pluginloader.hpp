#pragma once

#include "endpoint.hpp"

#include <filesystem>
#include <memory>
#include <vector>

namespace appweeb
{
    class PluginLoader
    {
    public:

        ~PluginLoader();

        std::vector<EndpointPtr> LoadPlugins(
            const std::filesystem::path& directory);

    private:

        std::vector<void*> m_handles;
    };
}