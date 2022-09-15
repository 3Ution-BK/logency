#ifndef LOGENCY_TEST_GLOBAL_RESOURCE_THREAD_POOL_HPP_
#define LOGENCY_TEST_GLOBAL_RESOURCE_THREAD_POOL_HPP_

#include "logency/detail/thread/thread_pool.hpp"

#include <memory>

namespace logency::unit_test::global_resource::thread_pool
{

auto normal() -> std::shared_ptr<logency::detail::thread::thread_pool>;

}

#endif // LOGENCY_TEST_GLOBAL_RESOURCE_THREAD_POOL_HPP_
