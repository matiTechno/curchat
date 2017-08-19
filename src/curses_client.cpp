#include "curses_client.hpp"
#include "tcp_client.hpp"
#include "msg_pool.hpp"
#include <ncurses.h>
#include <iostream>
#include <csignal>

bool CursesClient::quit = false;

CursesClient::CursesClient(MsgPool& msgPool, TcpClient& tcpClient, const std::string& name):
    msgPool(msgPool),
    tcpClient(tcpClient),
    name(name)
{
    initscr();
    std::signal(SIGINT, quitCallback);
    noecho();
    halfdelay(1);
    start_color();
    use_default_colors();
    init_pair(1, COLOR_GREEN, -1);
}

CursesClient::~CursesClient()
{
    endwin();
    std::cout << "\033[1mbye!\033[0m" << std::endl;
}

void CursesClient::run()
{
    int winX, winY;
    getmaxyx(stdscr, winY, winX);
    drawLineH(winY - 2, winX);
    move(winY - 1, 0);
    refresh();

    while(!quit)
    {
        char c = getch();

        int winY, winX;
        getmaxyx(stdscr, winY, winX);

        // * min one line for msgs
        // * line
        // * min one line for input
        if(winY < 3)
            throw std::runtime_error("error: too small terminal window");

        std::size_t maxChars = winX * (winY - 2) - 1; // -1 because of cursor

        switch(c)
        {
        case ERR: // timeout
            if(!msgPool.wasModified())
                continue;
            break;

        case '\n':
            msgPool.add(name + ": " + inputBuff); // it could be done by tcpClient
            // but wouldn't feel so smooth, because of the delay
            tcpClient.write(inputBuff);
            inputBuff.clear();
            break;

        case 127: // backspace
            if(inputBuff.size())
                inputBuff.pop_back();
            break;

        case '\t':
            for(int i = 0; i < 4; ++i)
            {
                if(inputBuff.size() < maxChars)
                    inputBuff.push_back(' ');
                else
                    break;
            }
            break;

        default: // printable ascii
            if(c >= 32 && c <= 126 && inputBuff.size() < maxChars)
                inputBuff.push_back(c);
        }

        int inputBuffY = winY - getInputBuffSizeY(inputBuff, winX);

        clear();

        // print msgPool
        msgPool.use([this, inputBuffY, winX](const std::vector<std::string>& msgs)
        {
            //                    msg          sizeY
            std::vector<std::pair<const char*, int>> chunks;

            auto availableY = inputBuffY - 1;

            auto sizeY = 0;

            for(auto it = msgs.rbegin(); it != msgs.rend(); ++it)
            {
                auto msgSizeY = getMsgSizeY(*it, winX);
                sizeY += msgSizeY;
                auto leftY = availableY - sizeY;

                if(leftY <= 0)
                {
                    auto msgNewSizeY = msgSizeY - (-leftY);
                    auto numCToSkip = (msgSizeY - msgNewSizeY) * winX;
                    chunks.emplace_back(it->c_str() + numCToSkip, msgNewSizeY);
                    break;
                }
                else
                    chunks.emplace_back(it->c_str(), msgSizeY);
            }

            auto chunkY = 0;

            for(auto it = chunks.rbegin(); it != chunks.rend(); ++it)
            {
                move(chunkY, 0);
                printw(it->first);
                chunkY += it->second;
            }
        });

        drawLineH(inputBuffY - 1, winX);

        move(inputBuffY, 0);
        printw(inputBuff.c_str());

        refresh();
    }

    tcpClient.close();
}

void CursesClient::drawLineH(int Y, int winX)
{
    move(Y, 0);
    attron(COLOR_PAIR(1));
    for(int i = 0; i < winX; ++i)
        addch(ACS_HLINE);
    attroff(COLOR_PAIR(1));
}

int CursesClient::getInputBuffSizeY(const std::string& str, int winX)
{
    int x = 0, y = 1;
    for(std::size_t i = 0; i < str.size(); ++i)
    {
        ++x;
        if(x == winX)
        {
            ++y;
            x = 0;
        }
    }
    return y;
}

int CursesClient::getMsgSizeY(const std::string& str, int winX)
{
    // + 1 because we don't count cursor as in getInputBuffSizeY
    return getInputBuffSizeY(str, winX + 1);
}

void CursesClient::quitCallback(int)
{
    quit = true;
}
