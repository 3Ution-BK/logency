#ifndef LOGENCY_INCLUDE_LOGENCY_DETAIL_THREAD_B_Q_HPP_
#define LOGENCY_INCLUDE_LOGENCY_DETAIL_THREAD_B_Q_HPP_

#include <cassert>

#include <cstddef>
#include <mutex>
#include <vector>

namespace logency::detail::thread
{

template <typename T, typename U>
class blocking_pair_queue
{
public:
    using size_type = typename std::size_t;
    using first_type = T;
    using second_type = U;

    template <typename ItemType>
    using container_type = std::vector<ItemType>;

    explicit blocking_pair_queue(size_type reserve_size = 0);
    ~blocking_pair_queue();

    blocking_pair_queue(const blocking_pair_queue &other) = delete;
    blocking_pair_queue(blocking_pair_queue &&other) noexcept = delete;
    auto operator=(const blocking_pair_queue &other)
        -> blocking_pair_queue & = delete;
    auto operator=(blocking_pair_queue &&other) noexcept
        -> blocking_pair_queue & = delete;

    void reserve(size_type size);
    void shrink_to_fit();

    [[nodiscard]] bool enqueue(const first_type &first,
                               const second_type &second);
    [[nodiscard]] bool enqueue(first_type &&first, second_type &&second);

    template <typename TIterator, typename UIterator>
    [[nodiscard]] bool enqueue_bulk(TIterator first_begin, TIterator first_end,
                                    UIterator second_begin,
                                    UIterator second_end);

    [[nodiscard]] bool try_swap_bulk(container_type<first_type> &first,
                                     container_type<second_type> &second);

    [[nodiscard]] auto capacity() -> size_type;
    [[nodiscard]] auto size() -> size_type;
    [[nodiscard]] bool is_empty();

private:
    using mutex_type = std::mutex;

    template <typename MutexLock>
    using lock_type = std::scoped_lock<MutexLock>;

    std::vector<first_type> first_buffer_{};
    std::vector<second_type> second_buffer_{};

    mutex_type buffer_mutex_{};
};

template <typename T, typename U>
blocking_pair_queue<T, U>::blocking_pair_queue(size_type reserve_size)
{
    first_buffer_.reserve(reserve_size);
    second_buffer_.reserve(reserve_size);

    assert(first_buffer_.size() == second_buffer_.size());
}

template <typename T, typename U>
blocking_pair_queue<T, U>::~blocking_pair_queue() = default;

template <typename T, typename U>
auto blocking_pair_queue<T, U>::capacity() -> size_type
{
    lock_type<mutex_type> buffer_lock{buffer_mutex_};
    assert(first_buffer_.size() == second_buffer_.size());

    return first_buffer_.capacity();
}

template <typename T, typename U>
bool blocking_pair_queue<T, U>::is_empty()
{
    lock_type<mutex_type> buffer_lock{buffer_mutex_};
    assert(first_buffer_.size() == second_buffer_.size());

    return first_buffer_.empty();
}

template <typename T, typename U>
bool blocking_pair_queue<T, U>::enqueue(const first_type &first,
                                        const second_type &second)
{
    return enqueue(first_type{first}, second_type{second});
}

template <typename T, typename U>
bool blocking_pair_queue<T, U>::enqueue(first_type &&first,
                                        second_type &&second)
{
    lock_type<mutex_type> buffer_lock{buffer_mutex_};
    assert(first_buffer_.size() == second_buffer_.size());

    const auto should_notify{first_buffer_.empty()};

    first_buffer_.push_back(std::move(first));
    second_buffer_.push_back(std::move(second));

    assert(first_buffer_.size() == second_buffer_.size());

    return should_notify;
}

template <typename T, typename U>
template <typename TIterator, typename UIterator>
bool blocking_pair_queue<T, U>::enqueue_bulk(TIterator first_begin,
                                             TIterator first_end,
                                             UIterator second_begin,
                                             UIterator second_end)
{
    lock_type<mutex_type> buffer_lock{buffer_mutex_};
    assert(first_buffer_.size() == second_buffer_.size());

    const auto should_notify{first_buffer_.empty()};

    first_buffer_.insert(std::end(first_buffer_), first_begin, first_end);
    second_buffer_.insert(std::end(second_buffer_), second_begin, second_end);

    assert(first_buffer_.size() == second_buffer_.size());

    return should_notify;
}

template <typename T, typename U>
void blocking_pair_queue<T, U>::reserve(size_type size)
{
    lock_type<mutex_type> buffer_lock{buffer_mutex_};
    assert(first_buffer_.size() == second_buffer_.size());

    first_buffer_.reserve(size);
    second_buffer_.reserve(size);

    assert(first_buffer_.size() == second_buffer_.size());
}

template <typename T, typename U>
void blocking_pair_queue<T, U>::shrink_to_fit()
{
    lock_type<mutex_type> buffer_lock{buffer_mutex_};
    assert(first_buffer_.size() == second_buffer_.size());

    first_buffer_.shrink_to_fit();
    second_buffer_.shrink_to_fit();

    assert(first_buffer_.size() == second_buffer_.size());
}

template <typename T, typename U>
auto blocking_pair_queue<T, U>::size() -> size_type
{
    lock_type<mutex_type> buffer_lock{buffer_mutex_};
    assert(first_buffer_.size() == second_buffer_.size());

    return first_buffer_.size();
}

template <typename T, typename U>
bool blocking_pair_queue<T, U>::try_swap_bulk(
    container_type<first_type> &first, container_type<second_type> &second)
{
    if (first.size() != second.size())
    {
        return false;
    }

    lock_type<mutex_type> buffer_lock{buffer_mutex_};

    assert(first_buffer_.size() == second_buffer_.size());

    if (first_buffer_.empty())
    {
        return false;
    }

    first_buffer_.swap(first);
    second_buffer_.swap(second);

    assert(first_buffer_.size() == second_buffer_.size());

    return true;
}

} // namespace logency::detail::thread

#endif // LOGENCY_INCLUDE_LOGENCY_DETAIL_THREAD_B_Q_HPP_
