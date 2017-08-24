#include "tcp_server.hpp"
#include <iostream>
#include <cstdlib>

int main(int argc, const char** argv)
{
    try
    {
        if(argc > 2)
        {
            std::cout << "usage: curserver\n"
                         "       curserver <port>"
                      << std::endl;
            return 0;
        }

        int port;
        if(argc == 2)
            port = std::atoi(argv[1]);
        else
            port = DEFAULT_PORT;

        asio::io_service ioService;
        tcp::endpoint endpoint(tcp::v4(), port);
        TcpServer server(ioService, endpoint);
        (void)server;
        ioService.run();
    }
    catch(const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
    return 0;
}
