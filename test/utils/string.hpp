#ifndef LOGENCY_TEST_TEST_HELP_MODULES_STRING_HPP_
#define LOGENCY_TEST_TEST_HELP_MODULES_STRING_HPP_

#include <array>
#include <string>
#include <string_view>
#include <type_traits>

namespace logency::unit_test::utils::string
{

bool string_equal(std::string_view lhs, std::string_view rhs);

bool start_with(std::string_view message, std::string_view contain);

inline bool string_equal(std::string_view lhs, std::string_view rhs)
{
    return lhs.compare(rhs) == 0;
}

inline bool start_with(std::string_view message, std::string_view contain)
{
    return message.rfind(contain, 0) == 0;
}

} // namespace logency::unit_test::utils::string

#endif // LOGENCY_TEST_TEST_HELP_MODULES_STRING_HPP_
