#pragma once

#include "message.hpp"
#include <string>
#include <vector>
#include <asio.hpp>
#include <future>
#include <atomic>

using asio::ip::tcp;
class MsgPool;

// warnings:
//          * possible multiple close() calls on the same socket
//          * send might be invoked right after ioService stops
//            (ioService will be restarted with pending send)
class TcpClient
{
public:
    TcpClient(MsgPool& msgPool, std::string name, const char* host, const char* port);

    ~TcpClient();

    void send(Message msg); // use only when isConnected() == true
    void connect();         // use only when isStopped() == true
    bool isConnected() const {return connected;}
    bool isStopped()   const {return ioService.stopped();}

private:
    MsgPool& msgPool;
    std::string name;
    asio::io_service ioService;
    tcp::socket socket;
    tcp::resolver::iterator endpointIt;
    Message readMsg;
    std::vector<Message> msgsToWrite;
    std::future<void> future;
    std::atomic_bool connected{false};

    void readHeader();
    void readBody();
    void write();
    void closeSocket();
};
