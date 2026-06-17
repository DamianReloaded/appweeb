// src/platform/linux/util.cpp

#include "../../util.hpp"

#include <filesystem>
#include <unistd.h>

namespace appweeb
{
    std::filesystem::path GetRootPath()
    {
        char buffer[4096];

        ssize_t length =
            readlink(
                "/proc/self/exe",
                buffer,
                sizeof(buffer) - 1);

        if (length < 0)
        {
            return {};
        }

        buffer[length] = '\0';

        return
            std::filesystem::path(buffer)
            .parent_path();
    }
}