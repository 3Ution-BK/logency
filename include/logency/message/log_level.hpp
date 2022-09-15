#ifndef LOGENCY_INCLUDE_LOGENCY_MESSAGE_LOG_LEVEL_HPP_
#define LOGENCY_INCLUDE_LOGENCY_MESSAGE_LOG_LEVEL_HPP_

#include <cassert>
#include <cstddef>

#include <array>
#include <string_view>

namespace logency
{

/**
 * \brief Represent the log level. From the least critical to the most one.
 */
enum class log_level : int
{
    trace = 0,   //!< Code tracing that should not be visible to the user.
    debug = 1,   //!< Code debugging that should not be visible to the user.
    info = 2,    //!< Tell user some info that it should be wary of.
    warning = 3, //!< Warn user some trouble that it sometimes can be ignored.
    error = 4,   //!< Warn user some trouble that it should not be ignored.
    critical = 5 //!< Warn user that something really bad happened.
};

constexpr const std::array<std::string_view, 6> log_string{
    {"trace", "debug", "info", "warning", "error", "critical"}};
constexpr const std::array<std::wstring_view, 6> log_wstring{
    {L"trace", L"debug", L"info", L"warning", L"error", L"critical"}};

template <typename CharT>
auto get_log_string(log_level level) -> std::basic_string_view<CharT>;

template <>
inline auto get_log_string<char>(log_level level) -> std::string_view
{
    /**
     * Note:
     * About clang-tidy (cppcoreguidelines-pro-bounds-constant-array-index)
     *
     * This check ensure the bound safety when access the array. It is good to
     * enable this feature. However, we are certain that the check is not
     * necessary in release mode in this case.
     *
     * * Input value is an enum class, the possible way to get invalid value is
     * to use type conversion(casting). Such possibility is not likely to happen
     * a lot if the user is careful enough.
     *
     * * This function will be called a lot of time (if the logger need to parse
     * the string value, which is likely to happen). It will speed up the
     * performance if the boundary check is not active.
     *
     * Overall, we will enable boundary check in debug mode by using
     * **assert()**. User should be wary of it.
     */

    const auto where{static_cast<std::size_t>(level)};

    assert(where < log_string.size());

    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
    return log_string[where];
}

template <>
inline auto get_log_string<wchar_t>(log_level level) -> std::wstring_view
{
    /**
     * Note:
     * About clang-tidy (cppcoreguidelines-pro-bounds-constant-array-index)
     *
     * This check ensure the bound safety when access the array. It is good to
     * enable this feature. However, we are certain that the check is not
     * necessary in release mode in this case.
     *
     * * Input value is an enum class, the possible way to get invalid value is
     * to use type conversion(casting). Such possibility is not likely to happen
     * a lot if the user is careful enough.
     *
     * * This function will be called a lot of time (if the logger need to parse
     * the string value, which is likely to happen). It will speed up the
     * performance if the boundary check is not active.
     *
     * Overall, we will enable boundary check in debug mode by using
     * **assert()**. User should be wary of it.
     */

    const auto where{static_cast<std::size_t>(level)};

    assert(where < log_wstring.size());

    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
    return log_wstring[where];
}

} // namespace logency

#endif // LOGENCY_INCLUDE_LOGENCY_MESSAGE_LOG_LEVEL_HPP_
