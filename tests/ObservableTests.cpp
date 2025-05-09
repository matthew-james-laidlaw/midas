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
