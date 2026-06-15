#include "webserver.hpp"
#include <iostream>
#include <windows.h>

static WebServer* g_server = nullptr;

BOOL WINAPI ConsoleHandler(
    DWORD signal)
{
    switch (signal)
    {
        case CTRL_C_EVENT:
        case CTRL_BREAK_EVENT:
        case CTRL_CLOSE_EVENT:
        case CTRL_SHUTDOWN_EVENT:
        {
            if (g_server)
            {
                g_server->Stop();
            }

            return TRUE;
        }
    }

    return FALSE;
}

int main()
{
    std::cout
        << "Open this URL in your browser: "
        << "http://localhost:8080/\n";

    WebServer server(8080);

    g_server = &server;

    SetConsoleCtrlHandler(
        ConsoleHandler,
        TRUE);

    server.Run();

    return 0;
}