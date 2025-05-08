#include <gtest/gtest.h>

#include <Observable.hpp>

#include <unordered_set>

template <typename T>
auto IsUnique(std::vector<T> const& list) -> bool
{
    return std::unordered_set<T>(list.begin(), list.end()).size() == list.size();
}

TEST(ObservableTests, MultipleNotifications)
{
    Observable<void> observable;

    int count = 0;

    observable.Subscribe([&]{ ++count; });

    observable.Notify();
    observable.Notify();

    EXPECT_EQ(count, 2);
}

TEST(ObservableTests, MultipleSubscribers)
{
    Observable<void> observable;

    int countA = 0;
    int countB = 0;

    observable.Subscribe([&]{ ++countA; });
    observable.Subscribe([&]{ ++countB; });

    observable.Notify();

    EXPECT_EQ(countA, 1);
    EXPECT_EQ(countB, 1);
}

TEST(ObservableTests, Unsubscribe)
{
    Observable<void> observable;

    int countA = 0;
    int countB = 0;

    auto a = observable.Subscribe([&]{ ++countA; });
    auto b = observable.Subscribe([&]{ ++countB; });

    observable.Unsubscribe(b);

    observable.Notify();

    EXPECT_EQ(countA, 1);
    EXPECT_EQ(countB, 0);
}

TEST(ObservableTests, UniqueIdentifiers)
{
    Observable<void> observable;

    std::vector<size_t> ids;
    std::vector<std::thread> threads;
    std::mutex threadsMutex;

    for (size_t i = 0; i < 10; ++i)
    {
        threads.emplace_back(std::thread([&]
        {
            size_t id = observable.Subscribe([&]{});
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

TEST(ObservableTests, ThreadSafety)
{
    Observable<void> observable;

    std::vector<size_t> ids;
    std::vector<std::thread> threads;
    std::mutex threadsMutex;

    std::atomic<int> count = 0;

    for (size_t i = 0; i < 10; ++i)
    {
        threads.emplace_back(std::thread([&]
        {
            observable.Subscribe([&]{ count.fetch_add(1); });
        }));
    }

    for (auto& thread : threads)
    {
        thread.join();
    }
    
    observable.Notify();

    EXPECT_EQ(count.load(), 10);
}
