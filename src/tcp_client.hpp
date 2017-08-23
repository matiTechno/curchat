#pragma once

#include "message.hpp"
#include <string>
#include <vector>
#include <asio.hpp>

using asio::ip::tcp;
class MsgPool;

class TcpClient
{
public:
    TcpClient(asio::io_service& ioService, tcp::resolver::iterator endpointIt,
              MsgPool& msgPool, std::string name);

    void send(Message msg);
    void close();

private:
    asio::io_service& ioService;
    tcp::socket socket;
    Message readMsg;
    std::vector<Message> msgsToWrite;
    MsgPool& msgPool;

    void connect(tcp::resolver::iterator endpointIt, std::string name);
    void readHeader();
    void readBody();
    void write();
};
