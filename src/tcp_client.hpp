#pragma once

#include "message.hpp"
#include <string>
#include <vector>
#include <asio.hpp>
#include <future>

using asio::ip::tcp;
class MsgPool;

// warning: possible multiple close() calls on the same socket
class TcpClient
{
public:
    TcpClient(MsgPool& msgPool, std::string name, const char* host, const char* port);

    ~TcpClient();

    void send(Message msg);
    void connect();
    bool isConnected() const;

private:
    MsgPool& msgPool;
    std::string name;
    asio::io_service ioService;
    tcp::socket socket;
    tcp::resolver::iterator endpointIt;
    Message readMsg;
    std::vector<Message> msgsToWrite;
    std::future<void> future;

    void readHeader();
    void readBody();
    void write();
};
