#include "AppConfig.h"
#include "Server.h"

#include <cstdlib>
#include <exception>
#include <iostream>

int main()
{
    try
    {
        Server server(AppConfig::fromEnvironment());
        return server.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Fatal startup error: " << e.what() << '\n';
        return EXIT_FAILURE;
    }
}
