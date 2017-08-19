#include "tcp_client.hpp"
#include "msg_pool.hpp"

TcpClient::TcpClient(asio::io_service& ioService, tcp::resolver::iterator endpointIt,
                     MsgPool& msgPool, const std::string& name):

    ioService(ioService),
    socket(ioService),
    msgPool(msgPool)
{
    connect(endpointIt, name);
}

void TcpClient::write(const Message& msg)
{
    ioService.post([this, msg]()
    {
        bool busy = msgsToWrite.size();
        msgsToWrite.push_back(msg);
        if(!busy)
            asioWrite();
    });
}

void TcpClient::close()
{
    ioService.post([this](){socket.close();});
}

void TcpClient::connect(tcp::resolver::iterator endpointIt, const std::string& name)
{
    asio::async_connect(socket, endpointIt,
                        [this, name](asio::error_code ec, tcp::resolver::iterator)
    {
        if(!ec)
        {
            write(name);
            readHeader();
        }
    });
}

void TcpClient::readHeader()
{
    asio::async_read(socket, asio::buffer(readMsg.getData(), Message::headerLength),
                     [this](asio::error_code ec, std::size_t)
    {
        if(!ec && readMsg.decodeHeader())
            readBody();
        else
            socket.close();
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
            socket.close();
    });
}

void TcpClient::asioWrite()
{
    auto& msg = msgsToWrite.front();
    asio::async_write(socket, asio::buffer(msg.getData(), msg.getDataLength()),
                      [this](asio::error_code ec, std::size_t)
    {
        if(!ec)
        {
            msgsToWrite.erase(msgsToWrite.begin());
            if(msgsToWrite.size())
                asioWrite();
        }
        else
            socket.close();
    });
}
