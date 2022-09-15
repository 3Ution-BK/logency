#ifndef LOGENCY_TEST_GLOBAL_RESOURCE_DISPATCHER_HPP_
#define LOGENCY_TEST_GLOBAL_RESOURCE_DISPATCHER_HPP_

#include "logency/dispatcher.hpp"

#include "utils/test_message.hpp"

#include <memory>

namespace logency::unit_test::global_resource::dispatcher
{

auto invalid_thread_pool()
    -> std::shared_ptr<logency::dispatcher<utils::message<char>>>;

auto normal() -> std::shared_ptr<logency::dispatcher<utils::message<char>>>;

} // namespace logency::unit_test::global_resource::diapatcher

#endif // LOGENCY_TEST_GLOBAL_RESOURCE_DISPATCHER_HPP_
