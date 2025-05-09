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

/**
 * Emits N events then completes.
 */
class FixedCountWatcher : public Watcher<int>
{
private:

    size_t mCount;
    size_t mCurrent;
    std::atomic<bool> mDone;

public:

    FixedCountWatcher(size_t count)
        : mCount(count)
        , mCurrent(0)
        , mDone(false)
    {}

    auto WaitForEvent() -> std::optional<int> override
    {
        if (mCurrent++ >= mCount)
        {
            mDone = true;
            mDone.notify_one();
            return std::nullopt;
        }
        return 0;
    }

    auto Wait() -> void
    {
        mDone.wait(false);
    }
};
