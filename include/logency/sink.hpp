#ifndef LOGENCY_INCLUDE_LOGENCY_SINK_HPP_
#define LOGENCY_INCLUDE_LOGENCY_SINK_HPP_

#include "logency/core/exception.hpp"
#include "logency/detail/message_pack.hpp"
#include "logency/detail/thread/blocking_queue.hpp"
#include "logency/detail/thread/thread_pool.hpp"
#include "logency/message/log_level.hpp"
#include "logency/sink_module/module_interface.hpp"

#include <algorithm>
#include <atomic>
#include <cassert>

#include <exception>
#include <functional>
#include <memory>
#include <mutex>
#include <utility>

namespace logency
{

template <typename MessageType>
class sink final : public std::enable_shared_from_this<sink<MessageType>>
{
public:
    using message_type = MessageType;
    using value_type = typename message_type::value_type;
    using string_type = typename message_type::string_type;
    using string_view_type = typename message_type::string_view_type;

    using message_pack_type = logency::message_pack<message_type>;
    using sink_module_type =
        logency::sink_module::module_interface<message_type>;

    using thread_pool_type = detail::thread::thread_pool;
    using queue_type = detail::thread::blocking_queue<message_pack_type>;

    using size_type = typename std::size_t;

    using filter_type =
        std::function<bool(string_view_type, const message_type &)>;
    using flusher_type =
        std::function<bool(string_view_type, const message_type &)>;

    explicit sink(string_type &&name,
                  std::unique_ptr<sink_module_type> sink_module,
                  std::weak_ptr<thread_pool_type> thread_pool);

    explicit sink(string_type &&name,
                  std::unique_ptr<sink_module_type> sink_module,
                  size_type reserve_size,
                  std::weak_ptr<thread_pool_type> thread_pool);

    sink(const sink &other) = delete;
    sink(sink &&other) noexcept = delete;
    auto operator=(const sink &other) -> sink & = delete;
    auto operator=(sink &&other) noexcept -> sink & = delete;

    /**
     * \brief Destroy the instance of the base sink class.
     */
    virtual ~sink();

    void reserve(size_type size);
    void shrink_to_fit();

    /**
     * \brief Sets the filter of the sink
     *
     * \param filter Specified filter
     *
     *  \sa set_filter
     */
    void set_filter(filter_type filter);

    /**
     * \brief Sets the flusher of the sink
     *
     * \param filter Specified flusher
     *
     *  \sa set_filter
     */
    void set_flusher(flusher_type flusher);

    template <typename Iterator>
    void log(Iterator begin, Iterator end);

    [[nodiscard]] auto sink_module() const noexcept -> const sink_module_type &;
    [[nodiscard]] auto sink_module() noexcept -> sink_module_type &;

    [[nodiscard]] auto name() const noexcept -> string_type;

    [[nodiscard]] auto queue_capacity() -> size_type;
    [[nodiscard]] auto queue_size() -> size_type;
    [[nodiscard]] bool is_queue_empty();

private:
    using mutex_type = std::mutex;
    template <typename MutexT>
    using lock_type = std::scoped_lock<MutexT>;

    template <typename T>
    using tray_type = typename queue_type::template container_type<T>;

    class thread_unit_token
        : public logency::detail::thread::thread_unit_interface
    {
    public:
        using me_type = sink<MessageType>;
        explicit thread_unit_token(std::shared_ptr<me_type> &&myself);

    protected:
        void operate_by_thread() final;

    private:
        std::shared_ptr<me_type> myself_{};
    };

    template <typename Iterator>
    void log_message(Iterator begin, Iterator end);

    [[nodiscard]] bool should_flush(const message_pack_type &pack);
    [[nodiscard]] bool should_log(const message_pack_type &pack);

    void notify_thread_pool();

    void sink_message();
    void sink_message_from_tray(tray_type<message_pack_type> &tray);

    queue_type queue_;

    //!< Keep it here to prevent deallocation
    tray_type<message_pack_type> queue_output_tray_;

    const string_type name_;

    filter_type filter_;
    flusher_type flusher_;

    std::unique_ptr<sink_module_type> sink_module_;
    std::weak_ptr<thread_pool_type> thread_pool_;

    mutex_type queue_tray_mutex_{};
};

template <typename MessageType>
sink<MessageType>::sink(string_type &&name,
                        std::unique_ptr<sink_module_type> sink_module,
                        std::weak_ptr<thread_pool_type> thread_pool)
    : sink{std::move(name), std::move(sink_module), 0U, std::move(thread_pool)}
{
}

template <typename MessageType>
sink<MessageType>::sink(string_type &&name,
                        std::unique_ptr<sink_module_type> sink_module,
                        size_type reserve_size,
                        std::weak_ptr<thread_pool_type> thread_pool)
    : queue_{reserve_size}, name_{std::move(name)},
      sink_module_{std::move(sink_module)}, thread_pool_{std::move(thread_pool)}
{
    if (!sink_module_)
    {
        throw logency::runtime_error("No sink_module assigned.");
    }
}

template <typename MessageType>
sink<MessageType>::~sink()
{
    sink_module_->flush();
}

template <typename MessageType>
bool sink<MessageType>::is_queue_empty()
{
    return queue_.is_empty();
}

template <typename MessageType>
template <typename Iterator>
void sink<MessageType>::log(Iterator begin, Iterator end)
{
    if (begin == end)
    {
        return;
    }

    auto head{begin};
    auto tail{head};

    while (tail != end)
    {
        if (!should_log((*tail)))
        {
            log_message(head, tail);

            head = std::next(tail);
        }
        ++tail;
    }

    log_message(head, tail);
}

template <typename MessageType>
template <typename Iterator>
void sink<MessageType>::log_message(Iterator begin, Iterator end)
{
    if (begin == end)
    {
        return;
    }

    if (queue_.enqueue_bulk(begin, end))
    {
        notify_thread_pool();
    }
}

template <typename MessageType>
auto sink<MessageType>::name() const noexcept -> string_type
{
    return name_;
}

template <typename MessageType>
void sink<MessageType>::notify_thread_pool()
{
    using token_type = thread_unit_token;

    auto pool{thread_pool_.lock()};

    if (!pool)
    {
        throw logency::runtime_error("Thread pool does not exist any longer.");
    }

    pool->enqueue(std::make_unique<token_type>(this->shared_from_this()));
}

template <typename MessageType>
auto sink<MessageType>::queue_capacity() -> size_type
{
    return queue_.capacity();
}

template <typename MessageType>
auto sink<MessageType>::queue_size() -> size_type
{
    return queue_.size();
}

template <typename MessageType>
void sink<MessageType>::reserve(size_type size)
{
    queue_.reserve(size);

    {
        lock_type<mutex_type> lock{queue_tray_mutex_};
        queue_output_tray_.reserve(size);
    }
}

template <typename MessageType>
void sink<MessageType>::set_filter(filter_type filter)
{
    filter_ = std::move(filter);
}

template <typename MessageType>
void sink<MessageType>::set_flusher(filter_type flusher)
{
    flusher_ = std::move(flusher);
}

template <typename MessageType>
void sink<MessageType>::shrink_to_fit()
{
    queue_.shrink_to_fit();

    {
        lock_type<mutex_type> lock{queue_tray_mutex_};
        queue_output_tray_.shrink_to_fit();
    }
}

template <typename MessageType>
void sink<MessageType>::sink_message()
{
    lock_type<mutex_type> lock{queue_tray_mutex_};

    /**
     * Push remaining message in tray.
     * The tray should be clean normally unless it throws in previous operation.
     */
    sink_message_from_tray(queue_output_tray_);

    if (!queue_.try_swap_bulk(queue_output_tray_)) // No message.
    {
        return;
    }

    sink_message_from_tray(queue_output_tray_);
}

template <typename MessageType>
void sink<MessageType>::sink_message_from_tray(
    tray_type<message_pack_type> &tray)
{
    auto pack{tray.begin()};

    try
    {
        for (; pack != tray.end(); ++pack)
        {
            const auto instance{*pack};

            sink_module_->log_message(*(instance->logger_name),
                                      instance->message);

            if (should_flush(*pack))
            {
                sink_module_->flush();
            }
        }
    }
    catch (const std::exception &e)
    {
        /*
         * If it throws:
         * 1. Erase the sink message in tray and keep the remaining one.
         * 2. Tell the thread pool that it still have some message here.
         * 3. Throw the exception to prevent the tray being cleared.
         */
        tray.erase(tray.begin(), pack + 1);
        notify_thread_pool();

        throw;
    }

    tray.clear();
}

template <typename MessageType>
auto sink<MessageType>::sink_module() const noexcept -> const sink_module_type &
{
    assert(sink_module_);
    return *(sink_module_.get());
}

template <typename MessageType>
auto sink<MessageType>::sink_module() noexcept -> sink_module_type &
{
    assert(sink_module_);
    return *(sink_module_.get());
}

template <typename MessageType>
bool sink<MessageType>::should_flush(const message_pack_type &pack)
{
    return (flusher_ ? flusher_(*(pack->logger_name), pack->message) : false);
}

template <typename MessageType>
bool sink<MessageType>::should_log(const message_pack_type &pack)
{
    return (filter_ ? filter_(*(pack->logger_name), pack->message) : true);
}

template <typename MessageType>
sink<MessageType>::thread_unit_token::thread_unit_token(
    std::shared_ptr<me_type> &&myself)
    : myself_{std::move(myself)}
{
}

template <typename MessageType>
void sink<MessageType>::thread_unit_token::operate_by_thread()
{
    myself_->sink_message();
}

} // namespace logency

#endif // LOGENCY_INCLUDE_LOGENCY_SINK_HPP_
