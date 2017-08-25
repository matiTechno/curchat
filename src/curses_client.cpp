#include "curses_client.hpp"
#include "tcp_client.hpp"
#include "msg_pool.hpp"
#include <ncurses.h>
#include <iostream>
#include <csignal>
#include <chrono>

bool CursesClient::quit = false;

CursesClient::CursesClient(MsgPool& msgPool, TcpClient& tcpClient):
    msgPool(msgPool),
    tcpClient(tcpClient)
{
    initscr();
    std::signal(SIGINT, quitCallback);
    noecho();
    halfdelay(1);
    start_color();
    use_default_colors();

    for(int i = COLOR_BLACK; i <= COLOR_WHITE; ++i)
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
    drawLineH(winY - 2, winX, false);
    move(winY - 1, 0);
    refresh();

    std::string inputBuff;

    double timeElapsed = 0;
    auto timeStart = std::chrono::steady_clock::now();

    while(!quit)
    {
        auto timeEnd = std::chrono::steady_clock::now();
        timeElapsed += std::chrono::duration<double>(timeEnd - timeStart).count();
        timeStart = timeEnd;

        bool repaint = false;
        if(timeElapsed >= 2)
        {
            repaint = true;
            timeElapsed = 0;
            if(!tcpClient.isConnected())
                tcpClient.connect();
        }

        char c = getch();

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
            if(!msgPool.wasModified() && !repaint)
                continue;
            break;

        case '\n':
            if(tcpClient.isConnected())
                // might be invoked right after ioService stops
                // fix this!
                tcpClient.send(inputBuff);

            inputBuff.clear();
            break;

        case 127: // backspace
            // ncurses glitch ?
            //
            // when the cursor goes up one line and the previous character
            // was not a space, it disappears until next input
            //
            // it does not happen if clear() is called instead of erase()
            // or if curchat runs without X session

            if(inputBuff.size())
                inputBuff.pop_back();
            break;

        case '\t':
            for(int i = 0; i < 4; ++i)
                add(inputBuff, maxChars, ' ');
            break;

        default: // printable ascii
            if(c >= 32 && c <= 126)
                add(inputBuff, maxChars, c);
        }

        int inputBuffY = winY - getInputBuffSizeY(inputBuff, winX);

        erase();

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
                    if(isMod(c))
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

        drawLineH(inputBuffY - 1, winX, repaint);

        move(inputBuffY, 0);
        addstr(inputBuff.c_str());

        refresh();
    }
}

void CursesClient::drawLineH(int Y, int winX, bool animate)
{
    move(Y, 0);

    int flags = COLOR_PAIR(COLOR_GREEN);
    char c = '=';
    bool connected = tcpClient.isConnected();

    if(!connected)
    {
        flags = COLOR_PAIR(COLOR_RED) | A_BOLD;
        c = '-';
    }

    attron(flags);
    for(int i = 0; i < winX; ++i)
        addch(c);
    standend();

    if(!connected)
    {
        static int pos = 0, dir = 1;

        if(animate)
            pos += dir;

        if(pos >= winX - 1)
        {
            pos = winX - 1;
            dir = -1;
        }
        else if(pos == 0)
            dir = 1;

        move(Y, pos);
        attron(COLOR_PAIR(COLOR_GREEN) | A_BOLD);
        addch('?');
        standend();
    }
}

int CursesClient::getInputBuffSizeY(const std::string& str, int winX) const
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

int CursesClient::getMsgSizeY(const std::string& str, int winX) const
{
    int x = 0, y = 1;
    for(std::size_t i = 0; i < str.size(); ++i)
    {
        if(isMod(&str[i]))
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

bool CursesClient::isMod(const char* str) const
{
    if(*str == '\\' && *(++str) != '\0' && (*str == 'r' || *str == 'b'
                                            || (*str >= 48 + COLOR_BLACK && *str <= 48 + COLOR_WHITE)))
        return true;
    return false;
}

void CursesClient::add(std::string& inputBuff, std::size_t maxChars, char c)
{
    if(inputBuff.size() < maxChars)
        inputBuff.push_back(c);
}
