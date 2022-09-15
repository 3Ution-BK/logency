#ifndef LOGENCY_TEST_UTILS_LITERALS_HPP_
#define LOGENCY_TEST_UTILS_LITERALS_HPP_

#include <cstdint>

namespace logency::unit_test::utils::literals
{

// NOLINTNEXTLINE(*-runtime-int)
constexpr auto operator"" _byte(unsigned long long int value) -> std::uintmax_t;

// NOLINTNEXTLINE(*-runtime-int)
inline constexpr auto operator"" _byte(unsigned long long int value)
    -> std::uintmax_t
{
    return static_cast<uintmax_t>(value);
}

} // namespace logency::unit_test::utils::literals

#endif // LOGENCY_TEST_UTILS_LITERALS_HPP_
