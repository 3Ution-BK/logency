#ifndef LOGENCY_INCLUDE_LOGENCY_DISPATCHER_HPP_
#define LOGENCY_INCLUDE_LOGENCY_DISPATCHER_HPP_

#include "logency/core/exception.hpp"
#include "logency/detail/message_pack.hpp"
#include "logency/detail/thread/blocking_pair_queue.hpp"
#include "logency/detail/thread/thread_pool.hpp"
#include "logency/logger.hpp"
#include "logency/sink.hpp"

#include <cstddef>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>

namespace logency
{

template <typename MessageType>
class dispatcher : public std::enable_shared_from_this<dispatcher<MessageType>>
{
public:
    using message_type = MessageType;

    using message_pack_type = logency::message_pack<message_type>;
    using logger_type = logger<message_type>;

    using thread_pool_type = detail::thread::thread_pool;

    using size_type = std::size_t;

    explicit dispatcher(std::weak_ptr<thread_pool_type> thread_pool);
    ~dispatcher();

    dispatcher(const dispatcher &other) = delete;
    dispatcher(dispatcher &&other) noexcept = delete;
    auto operator=(const dispatcher &other) -> dispatcher & = delete;
    auto operator=(dispatcher &&other) noexcept -> dispatcher & = delete;

    void reserve(size_type size);
    void shrink_to_fit();

    void enqueue(std::shared_ptr<logger_type> &&logger,
                 message_pack_type &&message);

    [[nodiscard]] auto queue_capacity() -> size_type;
    [[nodiscard]] auto queue_size() -> size_type;
    [[nodiscard]] bool is_queue_empty();

private:
    using mutex_type = std::mutex;

    template <typename Mutex>
    using lock_type = std::scoped_lock<Mutex>;

    using logger_value_type = std::shared_ptr<logger_type>;

    using queue_type =
        logency::detail::thread::blocking_pair_queue<logger_value_type,
                                                     message_pack_type>;

    template <typename T>
    using tray_type = typename queue_type::template container_type<T>;

    class thread_unit_token
        : public logency::detail::thread::thread_unit_interface
    {
    public:
        using me_type = dispatcher<MessageType>;
        explicit thread_unit_token(std::shared_ptr<me_type> myself);

    protected:
        void operate_by_thread() final;

    private:
        std::shared_ptr<me_type> myself_{};
    };

    void notify_thread_pool();

    void dispatch();
    void dispatch_message_from_tray(tray_type<logger_value_type> &loggers,
                                    tray_type<message_pack_type> &messages);

    queue_type queue_{};

    // Keep it here to prevent deallocation
    tray_type<logger_value_type> logger_tray_;
    // Keep it here to prevent deallocation
    tray_type<message_pack_type> message_tray_;

    std::weak_ptr<thread_pool_type> thread_pool_;
    mutex_type operate_mutex_{};
};

template <typename MessageType>
dispatcher<MessageType>::dispatcher(std::weak_ptr<thread_pool_type> thread_pool)
    : thread_pool_{std::move(thread_pool)}
{
}

template <typename MessageType>
dispatcher<MessageType>::~dispatcher() = default;

template <typename MessageType>
void dispatcher<MessageType>::dispatch()
{
    lock_type<mutex_type> lock{operate_mutex_};

    /**
     * Push remaining message in tray.
     * The tray should be clean normally unless it throws in previous operation.
     */
    dispatch_message_from_tray(logger_tray_, message_tray_);

    if (!queue_.try_swap_bulk(logger_tray_, message_tray_))
    {
        return;
    }

    dispatch_message_from_tray(logger_tray_, message_tray_);
}

template <typename MessageType>
void dispatcher<MessageType>::dispatch_message_from_tray(
    tray_type<logger_value_type> &loggers,
    tray_type<message_pack_type> &messages)
{
    assert(loggers.size() == messages.size());

    /**
     * Content of the tray *will not* deallocated since it might need to copy
     * the content into multiple queue(sink).
     */
    if (loggers.empty())
    {
        return;
    }

    auto destination{loggers.begin()};
    auto current_logger{std::next(destination)};

    auto message_head{messages.begin()};
    auto message_tail{std::next(message_head)};

    try
    {
        for (; current_logger != loggers.end();
             ++current_logger, ++message_tail)
        {
            assert(current_logger->get() != nullptr);

            if (destination->get() != current_logger->get())
            {
                (*destination)
                    ->dispatch_message_to_sinks(message_head, message_tail);

                destination = current_logger;
                message_head = message_tail;
            }
        }

        (*destination)->dispatch_message_to_sinks(message_head, message_tail);
    }
    catch (const std::exception &e)
    {
        /*
         * If it throws:
         * 1. Erase the sink message in tray and keep the remaining one.
         * 2. Tell the thread pool that it still have some message here.
         * 3. Throw the exception to prevent the tray being cleared.
         */
        loggers.erase(loggers.begin(), current_logger);
        messages.erase(messages.begin(), message_tail);

        notify_thread_pool();

        throw;
    }

    loggers.clear();
    messages.clear();
}

template <typename MessageType>
void dispatcher<MessageType>::enqueue(logger_value_type &&logger,
                                      message_pack_type &&message)
{
    if (!queue_.enqueue(std::move(logger), std::move(message)))
    {
        return;
    }

    notify_thread_pool();
}

template <typename MessageType>
bool dispatcher<MessageType>::is_queue_empty()
{
    return queue_.is_empty();
}

template <typename MessageType>
void dispatcher<MessageType>::notify_thread_pool()
{
    if (auto where{thread_pool_.lock()})
    {
        where->enqueue(
            std::make_unique<thread_unit_token>(this->shared_from_this()));

        return;
    }

    throw logency::runtime_error("Thread pool does not exist any longer.");
}

template <typename MessageType>
auto dispatcher<MessageType>::queue_capacity() -> size_type
{
    return queue_.capacity();
}

template <typename MessageType>
auto dispatcher<MessageType>::queue_size() -> size_type
{
    return queue_.size();
}

template <typename MessageType>
void dispatcher<MessageType>::reserve(size_type size)
{
    queue_.reserve(size);

    {
        lock_type<mutex_type> lock{operate_mutex_};

        logger_tray_.reserve(size);
        message_tray_.reserve(size);
    }
}

template <typename T>
void dispatcher<T>::shrink_to_fit()
{
    queue_.shrink_to_fit();

    {
        lock_type<mutex_type> lock{operate_mutex_};

        logger_tray_.shrink_to_fit();
        message_tray_.shrink_to_fit();
    }
}

template <typename MessageType>
dispatcher<MessageType>::thread_unit_token::thread_unit_token(
    std::shared_ptr<me_type> myself)
    : myself_{std::move(myself)}
{
}

template <typename MessageType>
void dispatcher<MessageType>::thread_unit_token::operate_by_thread()
{
    myself_->dispatch();
}

} // namespace logency

#endif // LOGENCY_INCLUDE_LOGENCY_DISPATCHER_HPP_
