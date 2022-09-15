#ifndef LOGENCY_INCLUDE_LOGENCY_LOGGER_HPP_
#define LOGENCY_INCLUDE_LOGENCY_LOGGER_HPP_

#include "logency/core/exception.hpp"
#include "logency/detail/message_pack.hpp"
#include "logency/sink.hpp"

#include <algorithm>
#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

namespace logency
{

template <typename MessageType>
class manager;

template <typename MessageType>
class dispatcher;

template <typename MessageType>
class logger : public std::enable_shared_from_this<logger<MessageType>>
{
public:
    using message_type = MessageType;
    using string_type = typename message_type::string_type;
    using string_view_type = typename message_type::string_view_type;

    using message_pack_type = logency::message_pack<message_type>;
    using sink_type = sink<message_type>;
    using sink_pointer_type = std::shared_ptr<sink_type>;
    using sink_pointers_type = std::vector<sink_pointer_type>;
    using dispatcher_type = dispatcher<message_type>;

    using error_handler_type = std::function<void(const std::exception &)>;
    using filter_type =
        std::function<bool(string_view_type, const message_type &)>;

    explicit logger(string_type &&name,
                    std::weak_ptr<dispatcher_type> dispatcher);
    ~logger();

    logger(const logger &other) = delete;
    logger(logger &&other) noexcept = delete;
    auto operator=(const logger &other) -> logger & = delete;
    auto operator=(logger &&other) noexcept -> logger & = delete;

    template <typename... Args>
    void log(Args &&...args);

    [[nodiscard]] auto name() const noexcept -> string_type;

    void add_sink(sink_pointer_type sink);
    [[nodiscard]] auto find_sink(string_view_type name) -> sink_pointer_type;
    void delete_sink(string_view_type name);
    void delete_sink(sink_pointer_type sink);

    void set_filter(filter_type filter);
    void set_error_handler(error_handler_type handler);

private:
    using mutex_type = std::mutex;
    template <typename MutexT>
    using lock_type = std::scoped_lock<MutexT>;

    friend manager<message_type>;
    friend dispatcher<message_type>;

    void mark_as_destroy() noexcept;
    bool should_log(const message_pack_type &pack);

    template <typename... Args>
    void log_inner(Args &&...args);

    template <typename Iterator>
    void delete_sink_inner(Iterator where);

    template <typename Iterator>
    auto find_sink_location(sink_pointer_type sink) -> Iterator;
    template <typename Iterator>
    auto find_sink_location(string_view_type name) -> Iterator;

    template <typename Iterator>
    void dispatch_message_to_sinks(Iterator begin, Iterator end);

    // It will be used in message pack for multiple thread reasons.
    std::shared_ptr<string_type> name_;

    std::weak_ptr<dispatcher_type> dispatcher_;

    sink_pointers_type sinks_{};
    mutex_type sink_mutex_;

    /**
     * No need to use mutex since it is in critical path (for user);
     * Warn the user about it.
     */
    filter_type filter_;

    error_handler_type error_handler_{};
    mutex_type error_handler_mutex_;

    std::atomic<bool> mark_as_destroy_{false};
};

template <typename MessageType>
inline logger<MessageType>::logger(string_type &&name,
                                   std::weak_ptr<dispatcher_type> dispatcher)
    : name_{std::make_shared<string_type>(std::move(name))},
      dispatcher_{std::move(dispatcher)}
{
}

template <typename MessageType>
inline logger<MessageType>::~logger() = default;

template <typename MessageType>
void logger<MessageType>::add_sink(sink_pointer_type sink)
{
    lock_type<mutex_type> lock{sink_mutex_};

    if (find_sink_location<typename sink_pointers_type::iterator>(sink) !=
        sinks_.end())
    {
        throw logency::runtime_error(
            "This logger has connected to the sink already.");
    }

    sinks_.push_back(std::move(sink));
}

template <typename MessageType>
void logger<MessageType>::delete_sink(string_view_type name)
{
    lock_type<mutex_type> lock{sink_mutex_};
    auto where{find_sink_location<typename sink_pointers_type::iterator>(name)};

    if (where == sinks_.end())
    {
        throw logency::runtime_error(
            "Cannot found the sink requested in the logger.");
    }

    delete_sink_inner(std::move(where));
}

template <typename MessageType>
void logger<MessageType>::delete_sink(sink_pointer_type sink)
{
    lock_type<mutex_type> lock{sink_mutex_};

    auto where{find_sink_location<typename sink_pointers_type::iterator>(sink)};

    if (where == sinks_.end())
    {
        throw logency::runtime_error(
            "Cannot found the sink requested in the logger.");
    }

    delete_sink_inner(std::move(where));
}

template <typename MessageType>
template <typename Iterator>
void logger<MessageType>::dispatch_message_to_sinks(Iterator begin,
                                                    Iterator end)
{
    if (begin == end)
    {
        return;
    }

    lock_type<mutex_type> lock{sink_mutex_};

    for (auto &sink : sinks_)
    {
        sink->log(begin, end);
    }
}

template <typename MessageType>
template <typename Iterator>
void logger<MessageType>::delete_sink_inner(Iterator where)
{
    sinks_.erase(where);
}

template <typename MessageType>
auto logger<MessageType>::find_sink(string_view_type name) -> sink_pointer_type
{
    lock_type<mutex_type> lock{sink_mutex_};

    if (auto where{
            find_sink_location<typename sink_pointers_type::iterator>(name)};
        where != sinks_.end())
    {
        return *where;
    }
    return nullptr;
}

template <typename MessageType>
template <typename Iterator>
auto logger<MessageType>::find_sink_location(sink_pointer_type sink) -> Iterator
{
    return std::find(sinks_.begin(), sinks_.end(), sink);
}

template <typename MessageType>
template <typename Iterator>
auto logger<MessageType>::find_sink_location(string_view_type name) -> Iterator
{
    return std::find_if(sinks_.begin(), sinks_.end(),
                        [&](const sink_pointer_type &sink) -> bool
                        { return sink->name() == name; });
}

template <typename MessageType>
void logger<MessageType>::mark_as_destroy() noexcept
{
    mark_as_destroy_.store(true, std::memory_order::memory_order_relaxed);
}

template <typename MessageType>
template <typename... Args>
void logger<MessageType>::log(Args &&...args)
{
    try
    {
        log_inner(std::forward<Args>(args)...);
    }
    catch (const std::exception &e)
    {
        lock_type<mutex_type> lock{error_handler_mutex_};

        if (error_handler_)
        {
            error_handler_(e);
        }
        else
        {
            throw;
        }
    }
}

template <typename MessageType>
template <typename... Args>
void logger<MessageType>::log_inner(Args &&...args)
{
    if (mark_as_destroy_.load(std::memory_order::memory_order_relaxed))
    {
        throw logency::runtime_error(
            "This logger is marked as destroyed. "
            "It is illegal to log this logger anymore.");
    }

    auto dispatcher{dispatcher_.lock()};

    if (!dispatcher)
    {
        throw logency::runtime_error("Dispatcher does not exist.");
    }

    auto message_pack{logency::make_message_pack<message_type>(
        name_, message_type{std::forward<Args>(args)...})};

    if (!should_log(message_pack))
    {
        return;
    }

    dispatcher->enqueue(this->shared_from_this(), std::move(message_pack));
}

template <typename MessageType>
auto logger<MessageType>::name() const noexcept -> string_type
{
    return *name_;
}

template <typename MessageType>
void logger<MessageType>::set_error_handler(error_handler_type handler)
{
    lock_type<mutex_type> lock{error_handler_mutex_};
    error_handler_ = std::move(handler);
}

template <typename MessageType>
void logger<MessageType>::set_filter(filter_type filter)
{
    filter_ = std::move(filter);
}

template <typename MessageType>
bool logger<MessageType>::should_log(const message_pack_type &pack)
{
    return (filter_ ? filter_(name(), pack->message) : true);
}

} // namespace logency

#endif // LOGENCY_INCLUDE_LOGENCY_LOGGER_HPP_
