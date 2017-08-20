#pragma once

#include <string>

class Message
{
public:
    enum {headerLength = 4};

    Message() = default;
    Message(const std::string& str);

    // returns true if the msg header is correct
    bool decodeHeader();

    const char* getBody() const {return data + headerLength;}

    char* getBody() {return data + headerLength;}

    std::size_t getBodyLength() const {return bodyLength;}

    char* getData() {return data;}

    std::size_t getDataLength() const {return headerLength + bodyLength;}

    std::string getStdString(){return std::string(getBody(), bodyLength);}

private:
    enum {maxBodyLength = 512};
    char data[headerLength + maxBodyLength];
    std::size_t bodyLength;
};
