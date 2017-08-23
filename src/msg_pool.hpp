#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <functional>

class MsgPool
{
public:
    void add(std::string str)
    {
        std::lock_guard<std::mutex> lock(mutex);
        (void)lock;

        if(msgs.size() == maxMsgs)
            msgs.erase(msgs.begin());
        msgs.push_back(std::move(str));

        modified = true;
    }

    void use(std::function<void(const std::vector<std::string>&)> fun)
    {
        std::lock_guard<std::mutex> lock(mutex);
        (void)lock;

        fun(msgs);

        modified = false;
    }

    bool wasModified() const
    {
        std::lock_guard<std::mutex> lock(mutex);
        (void)lock;
        return modified;
    }

private:
    enum {maxMsgs = 100};
    std::vector<std::string> msgs;
    mutable std::mutex mutex;
    bool modified = false;
};
