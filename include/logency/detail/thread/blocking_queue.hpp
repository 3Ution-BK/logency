#ifndef LOGENCY_INCLUDE_LOGENCY_DETAIL_THREAD_BLOCKING_QUEUE_HPP_
#define LOGENCY_INCLUDE_LOGENCY_DETAIL_THREAD_BLOCKING_QUEUE_HPP_

#include <cassert>

#include <mutex>
#include <vector>

namespace logency::detail::thread
{

template <typename T>
class blocking_queue
{
public:
    using size_type = typename std::size_t;
    using value_type = T;

    template <typename ItemType>
    using container_type = std::vector<ItemType>;

    explicit blocking_queue(size_type reserve_size = 0);
    ~blocking_queue();

    blocking_queue(const blocking_queue &other) = delete;
    blocking_queue(blocking_queue &&other) noexcept = delete;
    auto operator=(const blocking_queue &other) -> blocking_queue & = delete;
    auto operator=(blocking_queue &&other) noexcept
        -> blocking_queue & = delete;

    void reserve(size_type size);
    void shrink_to_fit();

    [[nodiscard]] bool enqueue(const value_type &value);
    [[nodiscard]] bool enqueue(value_type &&value);

    template <typename Iterator>
    [[nodiscard]] bool enqueue_bulk(Iterator begin, Iterator end);

    [[nodiscard]] bool try_swap_bulk(container_type<value_type> &out);

    [[nodiscard]] auto capacity() -> size_type;
    [[nodiscard]] auto size() -> size_type;
    [[nodiscard]] bool is_empty();

private:
    using mutex_type = std::mutex;

    template <typename MutexLock>
    using lock_type = std::scoped_lock<MutexLock>;

    container_type<value_type> buffer_{};
    mutex_type buffer_mutex_{};
};

template <typename T>
blocking_queue<T>::blocking_queue(size_type reserve_size)
{
    buffer_.reserve(reserve_size);
}

template <typename T>
blocking_queue<T>::~blocking_queue() = default;

template <typename T>
auto blocking_queue<T>::capacity() -> size_type
{
    lock_type<mutex_type> buffer_lock{buffer_mutex_};
    return buffer_.capacity();
}

template <typename T>
bool blocking_queue<T>::is_empty()
{
    lock_type<mutex_type> buffer_lock{buffer_mutex_};
    return buffer_.empty();
}

template <typename T>
bool blocking_queue<T>::enqueue(const value_type &value)
{
    return enqueue(value_type{value});
}

template <typename T>
bool blocking_queue<T>::enqueue(value_type &&value)
{
    lock_type<mutex_type> buffer_lock{buffer_mutex_};

    const auto should_notify{buffer_.empty()};

    buffer_.push_back(std::move(value));

    return should_notify;
}

template <typename T>
template <typename Iterator>
bool blocking_queue<T>::enqueue_bulk(Iterator begin, Iterator end)
{
    lock_type<mutex_type> buffer_lock{buffer_mutex_};

    const auto should_notify{buffer_.empty()};

    buffer_.insert(std::end(buffer_), begin, end);

    return should_notify;
}

template <typename T>
void blocking_queue<T>::reserve(size_type size)
{
    lock_type<mutex_type> buffer_lock{buffer_mutex_};
    buffer_.reserve(size);
}

template <typename T>
void blocking_queue<T>::shrink_to_fit()
{
    lock_type<mutex_type> buffer_lock{buffer_mutex_};
    buffer_.shrink_to_fit();
}

template <typename T>
auto blocking_queue<T>::size() -> size_type
{
    lock_type<mutex_type> buffer_lock{buffer_mutex_};
    return buffer_.size();
}

template <typename T>
bool blocking_queue<T>::try_swap_bulk(container_type<value_type> &out)
{
    lock_type<mutex_type> buffer_lock{buffer_mutex_};
    if (buffer_.empty())
    {
        return false;
    }

    buffer_.swap(out);

    return true;
}

} // namespace logency::detail::thread

#endif // LOGENCY_INCLUDE_LOGENCY_DETAIL_THREAD_BLOCKING_QUEUE_HPP_
