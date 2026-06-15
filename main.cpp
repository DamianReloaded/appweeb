#include "webserver.hpp"
#include <iostream>
int main()
{
	std::cout <<"Open this URL in your browser: http://localhost:8080/";
    WebServer server(8080);

    server.Run();

    return 0;
}