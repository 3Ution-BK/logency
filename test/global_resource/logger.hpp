#ifndef LOGENCY_TEST_GLOBAL_RESOURCE_LOGGER_HPP_
#define LOGENCY_TEST_GLOBAL_RESOURCE_LOGGER_HPP_

#include "logency/logger.hpp"

#include "utils/test_message.hpp"

#include <memory>

namespace logency::unit_test::global_resource::logger
{

auto normal() -> std::shared_ptr<logency::logger<utils::message<char>>>;

}

#endif // LOGENCY_TEST_GLOBAL_RESOURCE_LOGGER_HPP_
