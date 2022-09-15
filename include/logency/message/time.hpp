#ifndef LOGENCY_INCLUDE_LOGENCY_MESSAGE_TIME_HPP_
#define LOGENCY_INCLUDE_LOGENCY_MESSAGE_TIME_HPP_

#include "logency/core/exception.hpp"

#include <chrono>

namespace logency::message
{

int get_ms(const std::chrono::system_clock::time_point &time_point);
auto get_tm(const std::chrono::system_clock::time_point &time_point) -> std::tm;

struct time_data
{
    explicit time_data(const std::chrono::system_clock::time_point &time_point);

    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
    int millisecond;
};

inline time_data::time_data(
    const std::chrono::system_clock::time_point &time_point)
    : millisecond{get_ms(time_point)}
{
    constexpr const int year_base{1900};
    constexpr const int month_base{1};

    std::tm tm_value{logency::message::get_tm(time_point)};

    year = tm_value.tm_year + year_base;
    month = tm_value.tm_mon + month_base;
    day = tm_value.tm_mday;
    hour = tm_value.tm_hour;
    minute = tm_value.tm_min;
    second = tm_value.tm_sec;
}

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

inline int get_ms(const std::chrono::system_clock::time_point &time_point)
{
    constexpr const int ms_unit{1000};

    return static_cast<int>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            time_point.time_since_epoch())
            .count() %
        ms_unit);
}

} // namespace logency::message

#endif // LOGENCY_INCLUDE_LOGENCY_MESSAGE_TIME_HPP_
