#ifndef LOGENCY_INCLUDE_LOGENCY_DETAIL_THREAD_THREAD_POOL_HPP_
#define LOGENCY_INCLUDE_LOGENCY_DETAIL_THREAD_THREAD_POOL_HPP_

#include "logency/core/exception.hpp"
#include "logency/detail/thread/thread_unit_interface.hpp"

#include <cassert>

#include <atomic>
#include <condition_variable>
#include <exception>
#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

namespace logency::detail::thread
{

class thread_pool
{
    using pool_type = std::vector<std::thread>;

public:
    using size_type = pool_type::size_type;
    using value_type = std::unique_ptr<thread_unit_interface>;

    using error_handler_type = std::function<void(const std::exception &)>;

    explicit thread_pool(size_type thread_number);
    ~thread_pool();

    thread_pool(const thread_pool &other) = delete;
    thread_pool(thread_pool &&other) noexcept = delete;
    auto operator=(const thread_pool &other) -> thread_pool & = delete;
    auto operator=(thread_pool &&other) noexcept -> thread_pool & = delete;

    void enqueue(value_type task);

    [[nodiscard]] auto pool_size() const noexcept -> size_type;

    void set_error_handler(error_handler_type handler);

    void wait_until_queue_empty();

private:
    using mutex_type = std::mutex;
    using condition_variable_type = std::condition_variable;
    template <class Mutex>
    using lock_type = std::unique_lock<Mutex>;
    using task_queue_type = std::queue<value_type>;

    static auto is_thread_number_valid(size_type thread_number) -> size_type;

    void tidy();
    void tidy_threads();

    void thread_loop();
    void stand_by(lock_type<mutex_type> &lock);

    [[nodiscard]] int running_threads() const noexcept;
    [[nodiscard]] bool is_pending() const noexcept;

    pool_type threads_;
    task_queue_type task_queue_;

    mutex_type mutex_{};
    condition_variable_type task_variable_{};
    condition_variable_type pending_variable_{};

    error_handler_type error_handler_{};

    std::atomic<int> running_counter_;
    std::atomic<bool> mark_as_destroy_{false};
};

inline thread_pool::thread_pool(size_t thread_number)
    : threads_(is_thread_number_valid(thread_number)),
      running_counter_{static_cast<int>(thread_number)}
{
    assert(threads_.size() == thread_number);

    try
    {
        for (auto &thread : threads_)
        {
            thread = std::thread{&thread_pool::thread_loop, this};
        }
    }
    catch (const std::system_error &e)
    {
        tidy();
        throw logency::system_error(e.code(), "Fail to create thread pool");
    }
}

inline thread_pool::~thread_pool() { tidy(); }

inline void thread_pool::enqueue(value_type task)
{
    {
        lock_type<mutex_type> lock{mutex_};
        task_queue_.push(std::move(task));
    }
    task_variable_.notify_one();
}

inline bool thread_pool::is_pending() const noexcept
{
    return (running_threads() == 0) && task_queue_.empty();
}

inline auto thread_pool::is_thread_number_valid(size_type thread_number)
    -> size_type
{
    if (thread_number == 0)
    {
        throw logency::runtime_error("'thread_number' is invalid.");
    }

    return thread_number;
}

inline auto thread_pool::pool_size() const noexcept -> size_type
{
    return threads_.size();
}

inline int thread_pool::running_threads() const noexcept
{
    return running_counter_.load(std::memory_order::memory_order_relaxed);
}

inline void thread_pool::set_error_handler(error_handler_type handler)
{
    lock_type<mutex_type> lock{mutex_};
    error_handler_ = std::move(handler);
}

inline void thread_pool::stand_by(lock_type<mutex_type> &lock)
{
    running_counter_.fetch_sub(1);

    if (is_pending())
    {
        pending_variable_.notify_all();
    }

    task_variable_.wait(lock, [&]
                        { return !task_queue_.empty() || mark_as_destroy_; });

    running_counter_.fetch_add(1);
}

inline void thread_pool::thread_loop()
{
    for (;;)
    {
        try
        {
            value_type task{nullptr};

            {
                lock_type<mutex_type> lock{mutex_};

                stand_by(lock);

                if (task_queue_.empty() && mark_as_destroy_)
                {
                    break;
                }

                task = std::move(task_queue_.front());
                task_queue_.pop();
            }

            task->operate_by_thread();
        }
        catch (const std::exception &e)
        {
            lock_type<mutex_type> lock{mutex_};

            // Cannot throw in thread. Or the library will break.
            if (error_handler_)
            {
                error_handler_(e);
            }
        }
    }

    assert(task_queue_.empty());
}

inline void thread_pool::tidy()
{
    mark_as_destroy_ = true;

    // Clean it up *after* the `mark_as_destroy_` flag is raised.
    tidy_threads();

    assert(task_queue_.empty());
}

inline void thread_pool::tidy_threads()
{
    task_variable_.notify_all();

    for (auto &thread : threads_)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
}

inline void thread_pool::wait_until_queue_empty()
{
    lock_type<mutex_type> lock{mutex_};
    pending_variable_.wait(lock, [this] { return is_pending(); });
}

} // namespace logency::detail::thread

#endif // LOGENCY_INCLUDE_LOGENCY_DETAIL_THREAD_THREAD_POOL_HPP_
