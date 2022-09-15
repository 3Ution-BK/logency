#ifndef LOGENCY_INCLUDE_LOGENCY_MANAGER_HPP_
#define LOGENCY_INCLUDE_LOGENCY_MANAGER_HPP_

#include "logency/core/exception.hpp"
#include "logency/detail/thread/thread_pool.hpp"
#include "logency/dispatcher.hpp"
#include "logency/logger.hpp"
#include "logency/sink.hpp"
#include "logency/sink_module/module_interface.hpp"

#include <memory>
#include <mutex>
#include <type_traits>
#include <unordered_map>

namespace logency
{

/**
 * \brief This class represent the logency manager and engine.
 *
 * It is the main manager of the library. It initialize, manipulate, destruct
 * the resource of the library. User can instantiate the engine, and the engine
 * will control the resource it assign to.
 *
 * This class is meant to be thread safe.
 *
 * \tparam MessageType Specified message type. It is used for passing required
 * message information inside the manager.
 */
template <typename MessageType>
class manager
{
public:
    using message_type = MessageType;

    using string_type = typename message_type::string_type;
    using string_view_type = typename message_type::string_view_type;

    using logger_type = logger<message_type>;
    using sink_type = sink<message_type>;
    using sink_module_type = sink_module::module_interface<message_type>;

    using error_handler_type = std::function<void(const std::exception &)>;

    /**
     * \brief Initializes a new instance of the empty manager class.
     *
     * It will instantiate 1 thread in the thread pool.
     */
    explicit manager();

    /**
     * \brief Initializes a new instance of the manager class with specified
     * \a thread_number.
     *
     * It will instantiate \a thread_number threads in the thread pool.
     *
     * \param thread_number Specified thread.
     */
    explicit manager(size_t thread_number);

    /**
     * \brief Destroy the instance of the manager class.
     *
     * \note
     * All the resource it assigned to will be destroy. No need to destruct them
     * manually.
     *
     * \par Destruction order
     * 1. MessageType instance.
     * 2. Logger instance.
     * 3. Sink instance.
     * 4. Thread pool instance & dispatcher instance.
     *
     * The destruction will be executed when the thread pool finish all of the
     * message operation process.
     */
    ~manager();

    manager(const manager &other) = delete;
    manager(manager &&other) noexcept = delete;
    auto operator=(const manager &other) -> manager & = delete;
    auto operator=(manager &&other) noexcept -> manager & = delete;

    /**
     * \brief Construct a new logger to this object.
     *
     * It will construct the logger based on the arguments which has the
     * same argument order as the constructor of the logency::logger.
     *
     * The logger will assign a new \a name and store it inside the manager
     * object. Each logger it assign to should have *1 and exactly 1* name.
     * Hence, every logger in this manager have 1 unique name.
     *
     * \note If the \a name has already assigned to an existing logger, it
     * will not get constructed. Instead, it will throw the exception
     * logency::runtime_error to indicate that the name has already taken.
     *
     * \note Be aware that the return pointer are point to the logger it
     * initalized. The manager(logency object) control the logger resource,
     * you do not need to manage the life cycle of the logger.
     *
     * \note While each individual logger should have its unique name, it is
     * acceptable to let the logger and the sink have the same name. Because
     * this class use different table to record the loggers and the sinks.
     *
     * \param name Specified name for the logger.
     * \return [[nodiscard]] Requested pointer point to the logger.
     * \throw logency::runtime_error If the name has already assigned.
     */
    [[nodiscard]] auto new_logger(string_view_type name)
        -> std::shared_ptr<logger_type>;

    /**
     * \brief Check if the logger container has the requested logger named \a
     * name.
     *
     * \param name Specified logger name.
     * \return Registered logger with same name, or \c nullptr if not found.
     */
    [[nodiscard]] auto find_logger(string_view_type name)
        -> std::shared_ptr<logger_type>;

    /**
     * \brief Delete the logger named \a name.
     *
     * It will try to find the instance is available or not. If the specified
     * logger object is not found, do nothing and returns. Otherwise, do the
     * following procedure:
     *
     * 1. Notify the thread pool to pause.
     * 2. Wait for the thread pool to pause completely.
     * 3. Delete the logger instance.
     * 4. Notify the thread pool to resumed.
     *
     * \warning Once the user called this function, the specified logger
     * instances should not be accessed by any thread. It is undefined behaviour
     * if the user access the logger instance and we strongly against to do
     * that. Make sure that there is no other thread accessing the logger before
     * calling this function.
     *
     * fixme: renew this statement. It is not UB anymore. It will throw
     * instead.
     *
     * \note User can still access other available logger instance and log
     * the message. The dispatcher will dispatch the message for at least 1 time
     * before the thread pool is paused. If the message does not manage
     * to catch up on time, the dispatcher will keep the message inside itself
     * and wait for the thread pool to resumed.
     *
     * \param name Specified name of the logger
     */
    void delete_logger(string_view_type name);

    /**
     * \brief Construct the new sync sink to this object.

     * It will construct the sink based on the arguments which has the
     * same argument order as the constructor of the \a SinkModule.
     *
     * The sink will assign a new \a name and store it inside the manager
     * object. Each sink it assign to should have *1 and exactly 1* name.
     * Hence, every sink in this manager have 1 unique name.
     *
     * \note If the \a name has already assigned to an existing sink, it
     * will not get constructed. Instead, it will throw the exception
     * logency::runtime_error to indicate that the name has already taken.
     *
     * \note Be aware that the return pointer are point to the sink it
     * initalized. The manager(logency object) control the sink resource,
     * you do not need to manage the life cycle of the sink.
     *
     * \note While each individual sink should have its unique name, it is
     * acceptable to let the sink and the logger have the same name. Because
     * this class use different table to record the sinks and the loggers.
     *
     * \tparam Sink Any sink type.
     * \tparam SinkModule Any sink module type.
     * \tparam Args Required argument type for instantiate sink.
     * \param name Specified name for the sink.
     * \param args Required argument for instantiate sink.
     * \return Requested pointer point to the sink.
     * \throw logency::runtime_error If the name has already assigned.
     *
     * \sa new_sink
     */
    template <typename SinkModule, typename... Args>
    [[nodiscard]] auto new_sink(string_view_type name, Args... args)
        -> std::shared_ptr<sink_type>;

    [[nodiscard]] auto new_sink(string_view_type name,
                                std::unique_ptr<sink_module_type> module)
        -> std::shared_ptr<sink_type>;

    /**
     * \brief Check if the sink container has the requested sink named \a name.
     *
     * \param name Specified sink name.
     * \return Registered sink with same name, or \c nullptr if not found.
     */
    [[nodiscard]] auto find_sink(string_view_type name)
        -> std::shared_ptr<sink_type>;

    /**
     * \brief Delete the sink named \a name.
     *
     * It will try to find the instance is available or not. If the specified
     * logger object is not found, do nothing and returns. Otherwise, do the
     * following procedure:
     *
     * 1. Notify the thread pool to pause.
     * 2. Wait for the thread pool to pause completely.
     * 3. Delete the logger instance.
     * 4. Notify the thread pool to resumed.
     *
     * \warning Once the user called this function, the specified sink instances
     * should not be accessed by any thread. It is undefined behaviour if the
     * user access the sink instance and we strongly against to do that. Make
     * sure that there is no other thread accessing the sink before calling
     * this function.
     *
     * \warning Before deleting the sink, you need to check whether the other
     * logger instance has the pointer point to this sink. Remove those
     * pointers before calling this function. Since it is likely that the thread
     * will access the delete address once the thread pool is resumed.
     *
     * \note User can still access other available logger instance and log
     * the message. The dispatcher will dispatch the message for at least 1 time
     * before the thread pool is paused. If the message does not manage
     * to catch up on time, the dispatcher will keep the message inside itself
     * and wait for the thread pool to resumed.
     *
     * \param name Specified name of the logger
     */
    void delete_sink(string_view_type name);

    void set_error_handler(error_handler_type handler);

    void wait_until_idle();

private:
    using dispatcher_type = dispatcher<message_type>;
    using thread_pool_type = detail::thread::thread_pool;
    using logger_map =
        std::unordered_map<string_type, std::shared_ptr<logger_type>>;
    using sink_map =
        std::unordered_map<string_type, std::shared_ptr<sink_type>>;
    using queue_type =
        detail::thread::blocking_queue<std::shared_ptr<message_type>>;
    using mutex_type = std::mutex;
    template <typename MutexType>
    using lock_type = std::scoped_lock<MutexType>;

    std::shared_ptr<thread_pool_type> thread_pool_;
    std::shared_ptr<dispatcher_type> dispatcher_;

    logger_map logger_map_{};
    mutex_type logger_map_mutex_{};

    sink_map sink_map_{};
    mutex_type sink_map_mutex_{};

    error_handler_type error_handler_{};
    mutex_type error_handler_mutex_;
};

template <typename MessageType>
inline manager<MessageType>::manager() : manager{1U}
{
}

template <typename MessageType>
inline manager<MessageType>::manager(size_t thread_number)
    : thread_pool_{std::make_shared<thread_pool_type>(thread_number)},
      dispatcher_{std::make_shared<dispatcher_type>(thread_pool_)}
{
}

template <typename MessageType>
inline manager<MessageType>::~manager()
{
    wait_until_idle();

    for (auto &logger : logger_map_)
    {
        logger.second->mark_as_destroy();
    }

    logger_map_.clear();
    sink_map_.clear();
}

template <typename MessageType>
inline void manager<MessageType>::delete_logger(string_view_type name)
{
    lock_type<mutex_type> lock{logger_map_mutex_};

    auto where = logger_map_.find(string_type{name});

    if (where == logger_map_.end())
    {
        throw logency::runtime_error("No such name in the manager.");
    }

    where->second->mark_as_destroy();
    logger_map_.erase(where);
}

template <typename MessageType>
inline void manager<MessageType>::delete_sink(string_view_type name)
{
    lock_type<mutex_type> lock{sink_map_mutex_};

    auto where = sink_map_.find(string_type{name});

    if (where == sink_map_.end())
    {
        throw logency::runtime_error("No such name in the manager.");
    }

    sink_map_.erase(where);
}

template <typename MessageType>
inline auto manager<MessageType>::find_logger(string_view_type name)
    -> std::shared_ptr<logger_type>
{
    lock_type<mutex_type> lock{logger_map_mutex_};

    if (auto where{logger_map_.find(string_type{name})};
        where != logger_map_.end())
    {
        return where->second;
    }

    return nullptr;
}

template <typename MessageType>
inline auto manager<MessageType>::find_sink(string_view_type name)
    -> std::shared_ptr<sink_type>
{
    lock_type<mutex_type> lock{sink_map_mutex_};

    if (auto where{sink_map_.find(string_type{name})}; where != sink_map_.end())
    {
        return where->second;
    }

    return nullptr;
}

template <typename MessageType>
inline auto manager<MessageType>::new_logger(string_view_type name)
    -> std::shared_ptr<logger_type>
{
    lock_type<mutex_type> lock{logger_map_mutex_};

    auto result{logger_map_.try_emplace(
        string_type{name},
        std::make_unique<logger_type>(string_type{name}, dispatcher_))};

    if (!result.second)
    {
        throw logency::runtime_error(
            "Already assigned a logger with the same name.");
    }

    if (error_handler_)
    {
        result.first->second->set_error_handler(error_handler_);
    }

    return result.first->second;
}

template <typename MessageType>
template <typename SinkModule, typename... Args>
inline auto manager<MessageType>::new_sink(string_view_type name, Args... args)
    -> std::shared_ptr<sink_type>
{
    static_assert(std::is_base_of<sink_module_type, SinkModule>::value,
                  "SinkModule is not derived from "
                  "sink::module_interface<message_type>");

    return new_sink(name,
                    std::make_unique<SinkModule>(std::forward<Args>(args)...));
}

template <typename MessageType>
inline auto
manager<MessageType>::new_sink(string_view_type name,
                               std::unique_ptr<sink_module_type> module)
    -> std::shared_ptr<sink_type>
{
    lock_type<mutex_type> lock{sink_map_mutex_};

    auto result{sink_map_.try_emplace(
        string_type{name},
        std::make_unique<sink<message_type>>(string_type{name},
                                             std::move(module), thread_pool_))};

    if (!result.second)
    {
        throw logency::runtime_error(
            "Already assigned a sink with the same name.");
    }

    return result.first->second;
}

template <typename MessageType>
inline void manager<MessageType>::set_error_handler(error_handler_type handler)
{
    lock_type<mutex_type> lock{error_handler_mutex_};
    error_handler_ = std::move(handler);

    thread_pool_->set_error_handler(error_handler_);

    for (auto &logger : logger_map_)
    {
        logger.second->set_error_handler(error_handler_);
    }
}

template <typename MessageType>
inline void manager<MessageType>::wait_until_idle()
{
    thread_pool_->wait_until_queue_empty();
}

} // namespace logency

#endif // LOGENCY_INCLUDE_LOGENCY_MANAGER_HPP_
