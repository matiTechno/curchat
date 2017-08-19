#include "msg_pool.hpp"
#include "curses_client.hpp"
#include "tcp_client.hpp"
#include <future>
#include <iostream>

int main()
{
    try
    {
        MsgPool msgPool;

        asio::io_service ioService;
        tcp::resolver resolver(ioService);
        auto endpointIt = resolver.resolve({DEFAULT_IP, DEFAULT_PORT});

        TcpClient tcpClient(ioService, endpointIt, msgPool, "dummy2");

        auto future = std::async(std::launch::async, [&ioService](){ioService.run();});

        CursesClient cursesClient(msgPool, tcpClient);

        cursesClient.run();

        future.get();
    }
    catch(const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
    return 0;
}
