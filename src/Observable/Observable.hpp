#pragma once

#include <functional>
#include <mutex>
#include <type_traits>
#include <unordered_map>
#include <vector>

/*
 * Thread-safe, push-based observable for event-driven architectures.
 */
template <typename T>
class Observable
{
private:

    using Callback = std::function<void(T const&)>;

    mutable std::mutex mMutex;
    std::unordered_map<size_t, Callback> mObservers;
    size_t mNextId;

public:

    Observable()
        : mNextId(0)
    {}

    /**
     * Register a callback to be fired when this observable notifies its subscribers.
     */
    auto Subscribe(Callback&& callback) -> size_t
    {
        std::lock_guard lock(mMutex);
        size_t id = mNextId++;
        mObservers.emplace(id, std::move(callback));
        return id;
    }

    /**
     * Unregister a callback so that it will no longer be fired when this observable notifies its
     * subscribers.
     */
    auto Unsubscribe(size_t id) -> void
    {
        std::lock_guard lock(mMutex);
        mObservers.erase(id);
    }

    /**
     * Fire observer callbacks.
     */
    auto Notify(T const& payload) const -> void
    {
        for (auto const& callback : GetSnapshot())
        {
            callback(payload);
        }
    }

private:

    /**
     * Return a copy of the current list of callbacks in a thread-safe manner.
     */
    auto GetSnapshot() const -> std::vector<Callback>
    {
        std::vector<Callback> snapshot;
        {
            std::lock_guard lock(mMutex);
            snapshot.reserve(mObservers.size());
            for (auto const& [id, callback] : mObservers)
            {
                snapshot.push_back(callback);
            }
        }
        return snapshot;
    }
};
