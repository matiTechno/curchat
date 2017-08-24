#pragma once

#include "message.hpp"
#include <string>
#include <vector>
#include <asio.hpp>
#include <atomic>
#include <future>

using asio::ip::tcp;
class MsgPool;

class TcpClient
{
public:
    TcpClient(asio::io_service& ioService, tcp::resolver::iterator endpointIt,
              MsgPool& msgPool, std::string name);

    ~TcpClient();

    void send(Message msg);
    void close();
    void reconnect();
    bool isConnected() const {return connected;}

private:
    asio::io_service& ioService;
    tcp::socket socket;
    Message readMsg;
    std::vector<Message> msgsToWrite;
    MsgPool& msgPool;
    std::atomic_bool connected{false};
    std::future<void> future;
    std::string name;
    tcp::resolver::iterator endpointIt;

    void readHeader();
    void readBody();
    void write();
    void shutdown();
};
