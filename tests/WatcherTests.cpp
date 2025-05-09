#include <gtest/gtest.h>

#include <Watcher.hpp>

#include <chrono>

using namespace std::chrono_literals;

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

TEST(WatcherTests, Subscribe)
{
    FixedCountWatcher watcher(3);

    size_t countA = 0;
    size_t countB = 0;

    watcher.Subscribe([&](int)
    {
        ++countA;
    });

    watcher.Subscribe([&](int)
    {
        ++countB;
    });

    watcher.Start();
    watcher.Wait();

    EXPECT_EQ(countA, 3);
    EXPECT_EQ(countB, 3);
}

TEST(WatcherTests, Unsubscribe)
{
    FixedCountWatcher watcher(3);

    size_t countA = 0;
    size_t countB = 0;

    auto a = watcher.Subscribe([&](int)
    {
        ++countA;
    });

    auto b = watcher.Subscribe([&](int)
    {
        ++countB;
    });

    watcher.Unsubscribe(b);

    watcher.Start();
    watcher.Wait();

    EXPECT_EQ(countA, 3);
    EXPECT_EQ(countB, 0);
}

TEST(WatcherTests, Stop)
{
    FixedCountWatcher watcher(10);

    size_t count = 0;

    watcher.Subscribe([&](int)
    {
        ++count;
        std::this_thread::sleep_for(10us);
    });

    // start and allow the watcher enough time to emit roughly half of its events
    watcher.Start();
    std::this_thread::sleep_for(50us);
    watcher.Stop();

    // ample time for the watcher to emit all 10 of its events
    std::this_thread::sleep_for(200us);

    EXPECT_TRUE(count > 0 && count < 10);
}
