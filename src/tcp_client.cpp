#include "tcp_client.hpp"
#include "msg_pool.hpp"

TcpClient::TcpClient(asio::io_service& ioService, tcp::resolver::iterator endpointIt,
                     MsgPool& msgPool, std::string name):

    ioService(ioService),
    socket(ioService),
    msgPool(msgPool),
    name(std::move(name)),
    endpointIt(endpointIt)
{
    reconnect();
}

TcpClient::~TcpClient()
{
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

void TcpClient::close()
{
    ioService.post([this](){socket.close();});
}

void TcpClient::reconnect()
{
    if(future.valid()) // when reconnect() is called for the first time
        //                future does not have a valid state
        future.get();

    ioService.reset();

    asio::async_connect(socket, endpointIt,
                        [this](asio::error_code ec, tcp::resolver::iterator)
    {
        if(!ec)
        {
            connected = true;
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
            readBody();
        else
            shutdown();
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
            shutdown();
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
            shutdown();
    });
}

void TcpClient::shutdown()
{
    connected = false;
    socket.close();
}
