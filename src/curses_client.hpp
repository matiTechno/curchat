#pragma once

#include <string>
#include <vector>

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
    std::string inputBuff;
    static bool quit;

    void drawLineH(int Y, int winX);
    int getInputBuffSizeY(const std::string& str, int winX);
    int getMsgSizeY(const std::string& str, int winX);
    static void quitCallback(int);
};
