#ifndef LOGENCY_TEST_UTILS_CHECK_EXCEPTION_HPP_
#define LOGENCY_TEST_UTILS_CHECK_EXCEPTION_HPP_

#include "string.hpp"

#include <functional>

namespace logency::unit_test::utils::check
{

template <typename ExceptionType>
void throw_start_with_as(std::function<void()> act, const char *start_with);

template <typename ExceptionType>
// NOLINTNEXTLINE(performance-unnecessary-value-param) Capture is needed.
void throw_start_with_as(std::function<void()> act, const char *start_with)
{
    try
    {
        act();
    }
    catch (const ExceptionType &e)
    {
        CHECK(logency::unit_test::utils::string::start_with(e.what(),
                                                            start_with));
    }
    catch (const std::exception &e)
    {
        FAIL_CHECK("Catch not requested exception.\n",
                   "Expect: ", typeid(ExceptionType).name(), "\n",
                   "Actual: ", typeid(e).name());
    }
}

} // namespace logency::unit_test::utils::check

#endif // LOGENCY_TEST_UTILS_CHECK_EXCEPTION_HPP_
