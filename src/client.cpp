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

        const char* host = DEFAULT_HOST_STR;
        const char* port = DEFAULT_PORT_STR;
        if(argc == 4)
        {
            host = argv[2];
            port = argv[3];
        }

        TcpClient tcpClient(msgPool, name, host, port);
        tcpClient.connect();

        CursesClient cursesClient(msgPool, tcpClient);

        cursesClient.run();
    }
    catch(const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
    return 0;
}
