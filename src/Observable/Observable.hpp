#pragma once

#include <atomic>
#include <functional>
#include <mutex>
#include <type_traits>
#include <unordered_map>
#include <vector>

/*
 * Thread-safe, generic observable container. Clients may subscribe to this object with a custom
 * callback, and they will be notified when this object is updated via that callback.
 */
template <typename T> class Observable
{
private:

    using Payload = std::conditional_t<std::is_void_v<T>, std::monostate, T>;
    using Callback = std::function<void(Payload const&)>;

    mutable std::mutex mMutex;
    std::unordered_map<size_t, Callback> mObservers;
    std::atomic<size_t> mNextId;

public:

    Observable()
        : mNextId(0)
    {}

    /**
     * Register a callback to be fired when this observable updates.
     */
    auto Subscribe(Callback&& callback) -> size_t
    {
        std::lock_guard lock(mMutex);
        size_t id = mNextId.fetch_add(1);
        mObservers.emplace(id, std::move(callback));
        return id;
    }

    /**
     * Register a callback to be fired when this observable updates.
     */
    auto Subscribe(std::function<void()>&& callback) -> size_t
    {
        return Subscribe([callback = std::move(callback)](std::monostate const&)
        {
            callback();
        });
    }

    /**
     * Unregister a callback so that it will no longer be fired when this observable updates.
     */
    auto Unsubscribe(size_t id) -> void
    {
        std::lock_guard lock(mMutex);
        mObservers.erase(id);
    }

    /**
     * Fire observer callbacks with a payload.
     */
    auto Notify(Payload const& payload) const -> void
    {
        for (auto const& callback : GetSnapshot())
        {
            callback(payload);
        }
    }

    /**
     * Fire observer callbacks.
     */
    auto Notify() const -> void { Notify(std::monostate{}); }

private:

    /**
     * Return a copy of the current list of callbacks
     */
    auto GetSnapshot() const -> std::vector<Callback>
    {
        std::vector<Callback> snapshot;
        snapshot.reserve(mObservers.size());
        {
            std::lock_guard lock(mMutex);
            for (auto const& [id, callback] : mObservers)
            {
                snapshot.push_back(callback);
            }
        }
        return snapshot;
    }
};
