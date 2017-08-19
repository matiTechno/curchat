#pragma once

#include <string>
#include <vector>

class MsgPool;
class TcpClient;

class CursesClient
{
public:
    CursesClient(MsgPool& msgPool, TcpClient& tcpClient, const std::string& name);

    ~CursesClient();

    void run();

private:
    MsgPool& msgPool;
    TcpClient& tcpClient;
    std::string inputBuff;
    const std::string name;
    static bool quit;

    void drawLineH(int Y, int winX);
    int getInputBuffSizeY(const std::string& str, int winX);
    int getMsgSizeY(const std::string& str, int winX);
    static void quitCallback(int);
};
