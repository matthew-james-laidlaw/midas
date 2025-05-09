#pragma once

#include "Observable.hpp"

#include <optional>

template <typename Event>
class Watcher
{
protected:

    Observable<Event> mOnEvent;
    std::atomic<bool> mRunning;
    std::thread mThread;

public:

    virtual ~Watcher()
    {
        Stop();
    }

    virtual auto WaitForEvent() -> std::optional<Event> = 0;

    auto Start() -> void
    {
        mRunning = true;
        mThread = std::thread([this]()
        {
            this->Main();
        });
    }

    auto Stop() -> void
    {
        mRunning = false;
        if (mThread.joinable())
        {
            mThread.join();
        }
    }

    auto Subscribe(std::function<void(Event const&)>&& callback) -> size_t
    {
        return mOnEvent.Subscribe(std::move(callback));
    }

    auto Unsubscribe(size_t id) -> void
    {
        mOnEvent.Unsubscribe(id);
    }

protected:

    auto Main() -> void
    {
        while (mRunning)
        {
            auto event = WaitForEvent();
            if (!mRunning || !event.has_value())
            {
                break;
            }
            mOnEvent.Notify(event.value());
        }
    }
};
