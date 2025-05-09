#include <gtest/gtest.h>

#include <Observable.hpp>
#include <Watcher.hpp>

#include <chrono>
#include <unordered_set>

using namespace std::chrono_literals;

template <typename T>
auto IsUnique(std::vector<T> const& list) -> bool
{
    return std::unordered_set<T>(list.begin(), list.end()).size() == list.size();
}

TEST(ObservableTests, MultipleNotifications)
{
    Observable<int> observable;

    int count = 0;

    observable.Subscribe([&](int)
    {
        ++count;
    });

    observable.Notify(1);
    observable.Notify(1);

    EXPECT_EQ(count, 2);
}

TEST(ObservableTests, MultipleSubscribers)
{
    Observable<int> observable;

    int countA = 0;
    int countB = 0;

    observable.Subscribe([&](int)
    {
        ++countA;
    });
    observable.Subscribe([&](int)
    {
        ++countB;
    });

    observable.Notify(1);

    EXPECT_EQ(countA, 1);
    EXPECT_EQ(countB, 1);
}

TEST(ObservableTests, Unsubscribe)
{
    Observable<int> observable;

    int countA = 0;
    int countB = 0;

    auto a = observable.Subscribe([&](int)
    {
        ++countA;
    });
    auto b = observable.Subscribe([&](int)
    {
        ++countB;
    });

    observable.Unsubscribe(b);

    observable.Notify(1);

    EXPECT_EQ(countA, 1);
    EXPECT_EQ(countB, 0);
}

TEST(ObservableTests, UniqueIdentifiers)
{
    Observable<int> observable;

    std::vector<size_t> ids;
    std::vector<std::thread> threads;
    std::mutex threadsMutex;

    for (size_t i = 0; i < 10; ++i)
    {
        threads.emplace_back(std::thread([&]
        {
            size_t id = observable.Subscribe([&](int) {});
            std::lock_guard lock(threadsMutex);
            ids.push_back(id);
        }));
    }

    for (auto& thread : threads)
    {
        thread.join();
    }

    EXPECT_EQ(ids.size(), 10);
    EXPECT_TRUE(IsUnique(ids));
}

TEST(ObservableTests, ConcurrentUse)
{
    Observable<int> observable;

    std::vector<size_t> ids;
    std::vector<std::thread> threads;
    std::mutex threadsMutex;

    std::atomic<int> count = 0;

    for (size_t i = 0; i < 10; ++i)
    {
        threads.emplace_back(std::thread([&]
        {
            observable.Subscribe([&](int)
            {
                count.fetch_add(1);
            });
        }));
    }

    for (auto& thread : threads)
    {
        thread.join();
    }

    observable.Notify(1);

    EXPECT_EQ(count.load(), 10);
}

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

