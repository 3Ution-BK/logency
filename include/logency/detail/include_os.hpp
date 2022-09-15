#ifndef LOGENCY_INCLUDE_LOGENCY_DETAIL_INCLUDE_OS_HPP_
#define LOGENCY_INCLUDE_LOGENCY_DETAIL_INCLUDE_OS_HPP_

#include "include_linux.hpp"
#include "include_windows.hpp"

#include <chrono>

namespace logency::detail::os
{

auto get_tm(const std::chrono::system_clock::time_point &time_point) -> std::tm;

inline auto get_tm(const std::chrono::system_clock::time_point &time_point)
    -> std::tm
{
    auto time_value{std::chrono::system_clock::to_time_t(time_point)};
    std::tm tm_value{};

#if defined(__GNUC__) || defined(__GNUG__)
    if (auto *result{localtime_r(&time_value, &tm_value)}; result == NULL)
    {
        throw logency::system_error(
            std::error_code{errno, std::generic_category()},
            "Failed to parse std::tm");
    }
#elif defined(_MSC_VER)
    if (auto result{localtime_s(&tm_value, &time_value)}; result != 0)
    {
        throw logency::system_error(
            std::error_code{result, std::generic_category()},
            "Failed to parse std::tm");
    }
#else
    tm_value = *localtime(&time_value);
    if (tm_value == NULL)
    {
        throw logency::system_error(
            std::error_code{errno, std::generic_category()},
            "Failed to parse std::tm");
    }
#endif

    return tm_value;
}

} // namespace logency::detail::os

#endif // LOGENCY_INCLUDE_LOGENCY_DETAIL_INCLUDE_OS_HPP_
