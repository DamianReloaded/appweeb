#include "../../util.hpp"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace appweeb
{
    std::filesystem::path GetRootPath()
    {
        wchar_t buffer[MAX_PATH];

        GetModuleFileNameW(
            nullptr,
            buffer,
            MAX_PATH);

        return
            std::filesystem::path(buffer)
            .parent_path();
    }
}

#endif