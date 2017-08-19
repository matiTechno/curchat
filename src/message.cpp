#include "message.hpp"
#include <cstring>
#include <cstdio>
#include <cstdlib>

Message::Message(const std::string& str)
{
    if(str.size() <= maxBodyLength)
        bodyLength = str.size();
    else
        bodyLength = maxBodyLength;

    std::memcpy(getBody(), str.c_str(), bodyLength);

    char header[headerLength + 1];
    std::sprintf(header, "%4d", static_cast<int>(bodyLength));
    std::memcpy(data, header, headerLength);
}

bool Message::decodeHeader()
{
    char header[headerLength + 1] = "";
    std::strncat(header, data, headerLength);
    bodyLength = std::atoi(header);

    if(bodyLength > maxBodyLength)
        return false;
    return true;
}
