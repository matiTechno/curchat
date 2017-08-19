#include "tcp_server.hpp"

ChatSession::ChatSession(tcp::socket socket, ChatRoom& room):
    socket(std::move(socket)),
    room(room)
{}

void ChatSession::start()
{
    room.join(shared_from_this());
    readHeader();
}

void ChatSession::deliver(const Message& msg)
{
    auto busy = msgsToWrite.size();
    msgsToWrite.push_back(msg);
    if(!busy)
        write();
}

void ChatSession::readHeader()
{
    auto self = shared_from_this();

    asio::async_read(socket, asio::buffer(readMsg.getData(), Message::headerLength),
                     [this, self](asio::error_code ec, std::size_t)
    {
        if(!ec && readMsg.decodeHeader())
            readBody();
        else
            room.leave(shared_from_this());
    });
}

void ChatSession::readBody()
{
    auto self = shared_from_this();

    asio::async_read(socket, asio::buffer(readMsg.getBody(), readMsg.getBodyLength()),
                     [this, self](asio::error_code ec, std::size_t)
    {
        if(!ec)
        {
            if(name.empty())
            {
                name = readMsg.getStdString();
                room.deliver(name + " has joined the chat. Welcome!", nullptr);
            }
            else
                room.deliver(name + ": " + readMsg.getStdString(), this);

            readHeader();
        }
        else
            room.leave(shared_from_this());
    });
}

void ChatSession::write()
{
    auto self = shared_from_this();

    auto& msg = msgsToWrite.front();
    asio::async_write(socket, asio::buffer(msg.getData(), msg.getDataLength()),
                      [this, self](asio::error_code ec, std::size_t)
    {
        if(!ec)
        {
            msgsToWrite.erase(msgsToWrite.begin());
            if(msgsToWrite.size())
                write();
        }
        else
            room.leave(shared_from_this());
    });
}

// ------------------------------------------------

void ChatRoom::join(ChatSessionPtr session)
{
    sessions.insert(session);
    for(auto& msg: recentMsgs)
        session->deliver(msg);
}

void ChatRoom::leave(ChatSessionPtr session)
{
    sessions.erase(session);
    deliver(session->getName() + " has left the chat.", nullptr);
}

void ChatRoom::deliver(const Message& msg, const ChatSession* sender)
{
    recentMsgs.push_back(msg);

    while(recentMsgs.size() > maxRecentMsgs)
        recentMsgs.erase(recentMsgs.begin());

    for(auto session: sessions)
        if(&*session != sender)
            session->deliver(msg);
}

// ------------------------------------------------

TcpServer::TcpServer(asio::io_service& ioService, const tcp::endpoint& endpoint):
    acceptor(ioService, endpoint),
    socket(ioService)
{
    accept();
}

void TcpServer::accept()
{
    acceptor.async_accept(socket, [this](asio::error_code ec)
    {
        if(!ec)
            std::make_shared<ChatSession>(std::move(socket), room)->start();

        accept();
    });
}
