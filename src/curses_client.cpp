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

    for(int i = COLOR_BLACK; i < COLOR_WHITE; ++i)
        init_pair(i, i, -1);
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
        // * line separator
        // * min one line for input
        if(winY < 3)
            throw std::runtime_error("error: too small terminal window");

        std::size_t maxChars = winX * (winY - 2) - 1; // -1 because of the cursor

        switch(c)
        {
        case ERR: // timeout
            if(!msgPool.wasModified())
                continue;
            break;

        case '\n':
            msgPool.add("\\b\\6" + name + ":\\r " + inputBuff); // it could be done by tcpClient
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
                for(const char* c = it->first; *c != '\0'; ++c)
                {
                    if(isSpecial(c))
                    {
                        ++c;
                        if(*c == 'r')
                            standend();
                        else if(*c == 'b')
                            attron(A_BOLD);
                        else
                            attron(COLOR_PAIR(*c - 48));
                    }
                    else
                        addch(*c);
                }
                chunkY += it->second;
                standend();
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
    attron(COLOR_PAIR(COLOR_GREEN));
    for(int i = 0; i < winX; ++i)
        addch('=');
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
    // + 1 because we don't count the cursor as in getInputBuffSizeY
    //return getInputBuffSizeY(str, winX + 1);

    int x = 0, y = 1;
    for(std::size_t i = 0; i < str.size(); ++i)
    {
        if(isSpecial(&str[i]))
        {
            ++i;
            continue;
        }
        ++x;
        if(x == winX + 1)
        {
            ++y;
            x = 0;
        }
    }
    return y;
}

void CursesClient::quitCallback(int)
{
    quit = true;
}

bool CursesClient::isSpecial(const char* str) const
{
    if(*str == '\\' && *(++str) != '\0' && (*str == 'r' || *str == 'b'
                                            || (*str >= 48 + COLOR_BLACK && *str <= 48 + COLOR_WHITE)))
        return true;
    return false;
}
