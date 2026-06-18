#include "webserver.hpp"

#include <atomic>
#include <csignal>
#include <iostream>

static std::atomic_bool g_stopRequested = false;

void SignalHandler(
    int)
{
    g_stopRequested.store(
        true,
        std::memory_order_relaxed);
}

int main()
{
    std::signal(
        SIGINT,
        SignalHandler);

    std::signal(
        SIGTERM,
        SignalHandler);

    appweeb::WebServer server(8080);

    server.LoadConfig();
    if (!server.GetRootPath().string().length())
    {
        server.SetRootPath("wwwroot");
    }
    std::cout <<"Root Path: " << server.GetRootPath() <<std::endl;

    std::cout << "Open this URL in your browser: " << "http://localhost:" << server.GetHttpPort() <<"\n";

    while (!g_stopRequested.load(std::memory_order_relaxed))
    {
        server.Run();
    }

    server.Stop();

    return 0;
}