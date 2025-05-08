#pragma once

#include "Observable.hpp"

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

    virtual auto WaitForEvent() -> Event = 0;

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
            if (!mRunning)
            {
                break;
            }
            mOnEvent.Notify(event);
        }
    }
};

class ConcreteWatcher : public Watcher<int>
{
public:

    auto WaitForEvent() -> int override
    {
        return 1;
    }
};

// // test concrete watcher that emits 3 events then stops
// class ConcreteWatcher : public Watcher<int> {
//     public:
//       ConcreteWatcher() : mEmitted(0) {}
    
//     protected:
//       std::optional<int> WaitForEvent() override {
//         if (mEmitted < 3) {
//           ++mEmitted;
//           // pace it so loop doesn’t spin too fast
//           std::this_thread::sleep_for(std::chrono::milliseconds(10));
//           return 1;
//         }
//         // signal “no more events” → loop breaks
//         return std::nullopt;
//       }
    
//     private:
//       int mEmitted;
//     };

// // watcher.hpp  
// protected:
//   // return std::nullopt to break the loop
//   virtual std::optional<Event> WaitForEvent() = 0;

// private:
//   void Main() {
//     while (mRunning) {
//       auto maybe = WaitForEvent();
//       if (!mRunning || !maybe) break;
//       mOnEvent.Notify(*maybe);
//     }
//   }