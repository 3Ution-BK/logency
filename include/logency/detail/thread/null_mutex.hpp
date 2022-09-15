#ifndef LOGENCY_INCLUDE_LOGENCY_DETAIL_THREAD_NULL_MUTEX_HPP_
#define LOGENCY_INCLUDE_LOGENCY_DETAIL_THREAD_NULL_MUTEX_HPP_

#include <mutex>

namespace logency::detail::thread
{

class null_mutex
{
public:
    void lock() const noexcept {}
    void unlock() const noexcept {}

    // NOLINTNEXTLINE(*-convert-member-functions-to-static)
    [[nodiscard]] bool try_lock() const noexcept { return true; }
};

} // namespace logency::detail::thread

#endif // LOGENCY_INCLUDE_LOGENCY_DETAIL_THREAD_NULL_MUTEX_HPP_
