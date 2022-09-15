#ifndef LOGENCY_INCLUDE_LOGENCY_DETAIL_THREAD_CONSOLE_MUTEX_HPP_
#define LOGENCY_INCLUDE_LOGENCY_DETAIL_THREAD_CONSOLE_MUTEX_HPP_

#include "null_mutex.hpp"

#include <mutex>

namespace logency::detail::thread
{

template <typename MutexType>
class console_mutex_base
{
public:
    using mutex_type = MutexType;

    static auto mutex() -> mutex_type &;
};

template <typename MutexType>
auto console_mutex_base<MutexType>::mutex() -> mutex_type &
{
    static mutex_type mutex;
    return mutex;
}

using console_mutex = console_mutex_base<std::mutex>;
using null_console_mutex = console_mutex_base<null_mutex>;

} // namespace logency::detail::thread

#endif // LOGENCY_INCLUDE_LOGENCY_DETAIL_THREAD_CONSOLE_MUTEX_HPP_
