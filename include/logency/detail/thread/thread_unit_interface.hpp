#ifndef LOGENCY_INCLUDE_LOGENCY_DETAIL_THREAD_THREAD_UNIT_INTERFACE_HPP_
#define LOGENCY_INCLUDE_LOGENCY_DETAIL_THREAD_THREAD_UNIT_INTERFACE_HPP_

namespace logency::detail::thread
{

class thread_pool;

class thread_unit_interface
{
public:
    virtual ~thread_unit_interface() = default;

protected:
    friend thread_pool;
    virtual void operate_by_thread() = 0;
};

} // namespace logency::detail::thread

#endif // LOGENCY_INCLUDE_LOGENCY_DETAIL_THREAD_THREAD_UNIT_INTERFACE_HPP_
