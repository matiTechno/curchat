#include <iostream>
#include "tcp_server.hpp"

int main()
{
    try
    {
        asio::io_service ioService;
        tcp::endpoint endpoint(tcp::v4(), std::atoi(DEFAULT_PORT));
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
