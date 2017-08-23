#pragma once

#include "message.hpp"
#include <asio.hpp>
#include <memory>
#include <string>
#include <vector>
#include <set>

using asio::ip::tcp;
class ChatRoom;

class ChatSession: public std::enable_shared_from_this<ChatSession>
{
public:
    ChatSession(tcp::socket socket, ChatRoom& room);

    void start();
    void deliver(Message msg);
    const std::string& getName() const {return name;}

private:
    tcp::socket socket;
    ChatRoom& room;
    std::string name;
    Message readMsg;
    std::vector<Message> msgsToWrite;

    void readHeader();
    void readBody();
    void write();
};

using ChatSessionPtr = std::shared_ptr<ChatSession>;

class ChatRoom
{
public:
    void join(ChatSessionPtr session);
    void leave(ChatSessionPtr session);
    // if sender != nullptr msg won't be delivered
    // to the corresponding client
    void deliver(Message msg, const ChatSession* sender);

private:
    enum {maxRecentMsgs = 100};
    std::set<ChatSessionPtr> sessions;
    std::vector<Message> recentMsgs;
};

class TcpServer
{
public:
    TcpServer(asio::io_service& ioService, const tcp::endpoint& endpoint);

private:
    tcp::acceptor acceptor;
    tcp::socket socket;
    ChatRoom room;

    void accept();
};
