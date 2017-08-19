#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <functional>

class MsgPool
{
public:
    void add(const std::string& str)
    {
        std::unique_lock<std::mutex> lock(mutex);
        (void)lock;

        if(msgs.size() == maxMsgs)
            msgs.erase(msgs.begin());
        msgs.push_back(str);

        modified = true;
    }

    void use(std::function<void(const std::vector<std::string>)> fun)
    {
        std::unique_lock<std::mutex> lock;
        (void)lock;

        fun(msgs);

        modified = false;
    }

    bool wasModified() const
    {
        std::unique_lock<std::mutex> lock;
        (void)lock;
        return modified;
    }

private:
    enum {maxMsgs = 100};
    std::vector<std::string> msgs;
    std::mutex mutex;
    bool modified = false;
};
