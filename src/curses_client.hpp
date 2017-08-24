#pragma once

#include <string>

class MsgPool;
class TcpClient;

class CursesClient
{
public:
    CursesClient(MsgPool& msgPool, TcpClient& tcpClient);

    ~CursesClient();

    void run();

private:
    MsgPool& msgPool;
    TcpClient& tcpClient;
    static bool quit;

    void drawLineH(int Y, int winX, bool animate);
    int getInputBuffSizeY(const std::string& str, int winX) const;
    int getMsgSizeY(const std::string& str, int winX) const;
    static void quitCallback(int);
    bool isMod(const char* str) const;
    void add(std::string& inputBuff, std::size_t maxChars, char c);
};
