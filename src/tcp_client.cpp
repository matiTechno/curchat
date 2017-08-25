#include "tcp_client.hpp"
#include "msg_pool.hpp"

TcpClient::TcpClient(MsgPool& msgPool, std::string name, const char* host, const char* port):
    msgPool(msgPool),
    name(std::move(name)),
    socket(ioService)
{
    tcp::resolver resolver(ioService);
    endpointIt = resolver.resolve({host, port});
}

TcpClient::~TcpClient()
{
    ioService.post([this](){closeSocket();});

    if(future.valid())
        future.get();
}

void TcpClient::send(Message msg)
{
    ioService.post([this, msg = std::move(msg)]()
    {
        auto busy = msgsToWrite.size();
        msgsToWrite.push_back(std::move(msg));
        if(!busy)
            write();
    });
}

bool TcpClient::isConnected() const {return !ioService.stopped() && connected;}

void TcpClient::connect()
{
    if(future.valid())
        future.get();

    ioService.restart();

    socket = tcp::socket(ioService);
    asio::async_connect(socket, endpointIt,
                        [this](asio::error_code ec, tcp::resolver::iterator)
    {
        if(!ec)
        {
            send(name);
            readHeader();
        }
    });

    future = std::async(std::launch::async, [this](){ioService.run();});
}

void TcpClient::readHeader()
{
    asio::async_read(socket, asio::buffer(readMsg.getData(), Message::headerLength),
                     [this](asio::error_code ec, std::size_t)
    {
        if(!ec && readMsg.decodeHeader())
        {
            connected = true;
            readBody();
        }
        else
            closeSocket();
    });
}

void TcpClient::readBody()
{
    asio::async_read(socket, asio::buffer(readMsg.getBody(), readMsg.getBodyLength()),
                     [this](asio::error_code ec, std::size_t)
    {
        if(!ec)
        {
            msgPool.add(readMsg.getStdString());
            readHeader();
        }
        else
            closeSocket();
    });
}

void TcpClient::write()
{
    auto& msg = msgsToWrite.front();
    asio::async_write(socket, asio::buffer(msg.getData(), msg.getDataLength()),
                      [this](asio::error_code ec, std::size_t)
    {
        if(!ec)
        {
            msgsToWrite.erase(msgsToWrite.begin());
            if(msgsToWrite.size())
                write();
        }
        else
            closeSocket();
    });
}

void TcpClient::closeSocket()
{
    connected = false;
    socket.close();
}
