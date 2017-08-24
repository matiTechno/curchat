#include "msg_pool.hpp"
#include "curses_client.hpp"
#include "tcp_client.hpp"
#include <iostream>
#include <fstream>
#include <cstdlib>

int main(int argc, char** argv)
{
    try
    {
        if(argc != 1 && argc != 2 && argc != 4)
        {
            std::cout << "usage: curchat\n"
                         "       curchat <name>\n"
                         "       curchat <name> <host> <port>"
                      << std::endl;
            return 0;
        }

        std::string name = "dummy";
        if(argc > 1)
            name = argv[1];
        else
        {
            const char* homeDir = std::getenv("HOME");
            if(homeDir)
            {
                std::ifstream file(homeDir + std::string("/.curchatrc"));
                if(file)
                    file >> name;
            }
        }

        MsgPool msgPool;

        // move this to TcpClient
        asio::io_service ioService;
        tcp::resolver resolver(ioService);
        tcp::resolver::iterator endpointIt;
        if(argc == 4)
            endpointIt = resolver.resolve({argv[2], argv[3]});
        else
            endpointIt = resolver.resolve({DEFAULT_HOST_STR, DEFAULT_PORT_STR});

        TcpClient tcpClient(ioService, endpointIt, msgPool, name);

        CursesClient cursesClient(msgPool, tcpClient);

        cursesClient.run();
    }
    catch(const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
    return 0;
}
