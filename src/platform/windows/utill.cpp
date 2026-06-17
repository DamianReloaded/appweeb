#include "../../util.hpp"
#include <windows.h>
#define WIN32_LEAN_AND_MEAN

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

