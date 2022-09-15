#ifndef LOGENCY_INCLUDE_LOGENCY_MESSAGE_STREAM_MESSAGE_HPP_
#define LOGENCY_INCLUDE_LOGENCY_MESSAGE_STREAM_MESSAGE_HPP_

#include "log_level.hpp"
#include "message_formatter.hpp"
#include "time.hpp"

#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>

namespace logency::message
{

/**
 * \brief This struct represent the basic message with the necessary basic data.
 */
template <typename CharT>
struct stream_message
{
    using value_type = CharT;
    using traits_type = std::char_traits<value_type>;
    using string_type = std::basic_string<value_type, traits_type>;
    using string_view_type = std::basic_string_view<value_type, traits_type>;
    using clock_type = std::chrono::system_clock;

    template <typename... Args>
    explicit stream_message(log_level level, Args &&...args);

    string_type content;
    clock_type::time_point time;
    log_level level;
};

template <typename CharT>
struct stream_stringifier
{
    using message_type = stream_message<CharT>;
    using value_type = typename message_type::value_type;
    using traits_type = typename message_type::traits_type;
    using string_type = typename message_type::string_type;
    using string_view_type = typename message_type::string_view_type;
    using clock_type = typename message_type::clock_type;

    [[nodiscard]] static auto format(string_view_type logger,
                                     const message_type &message)
        -> string_type;
    [[nodiscard]] static auto format_first(string_view_type logger,
                                           const message_type &message)
        -> string_type;
    [[nodiscard]] static auto format_second(string_view_type logger,
                                            const message_type &message)
        -> string_type;
    [[nodiscard]] static auto format_third(string_view_type logger,
                                           const message_type &message)
        -> string_type;
};

template <typename CharT>
template <typename... Args>
stream_message<CharT>::stream_message(log_level level, Args &&...args)
    : time{clock_type::now()}, level{level}
{
    std::basic_stringstream<value_type> stream;

    (stream << ... << std::forward<Args>(args));

    content = stream.str();
}

template <typename CharT>
inline auto stream_stringifier<CharT>::format(string_view_type logger,
                                              const message_type &message)
    -> string_type
{
    const auto first{format_first(logger, message)};
    const auto second{format_second(logger, message)};
    const auto third{format_third(logger, message)};

    string_type output{first};
    output.reserve(first.size() + second.size() + third.size());
    output.append(second).append(third);

    return output;
}

template <>
inline auto stream_stringifier<char>::format_first(string_view_type /*logger*/,
                                                   const message_type &message)
    -> string_type
{
    using size_type = string_type::size_type;

    constexpr const size_type ms_buffer_size{3};

    const auto tm_value{logency::message::get_tm(message.time)};

    std::basic_stringstream<value_type> stream{};

    stream << "[" << std::put_time(&tm_value, "%F %T") << "."
           << std::setfill('0') << std::setw(ms_buffer_size)
           << get_ms(message.time) << "] ";

    return stream.str();
}

template <>
inline auto stream_stringifier<wchar_t>::format_first(
    string_view_type /*logger*/, const message_type &message) -> string_type
{
    using size_type = string_type::size_type;

    constexpr const size_type ms_buffer_size{3};

    const auto tm_value{logency::message::get_tm(message.time)};

    std::basic_stringstream<value_type> stream{};

    stream << L"[" << std::put_time(&tm_value, L"%F %T") << L"."
           << std::setfill(L'0') << std::setw(ms_buffer_size)
           << get_ms(message.time) << L"] ";

    return stream.str();
}

template <>
inline auto stream_stringifier<char>::format_second(string_view_type /*logger*/,
                                                    const message_type &message)
    -> string_type
{
    using size_type = std::string::size_type;

    constexpr const size_type level_buffer_size{8};

    std::basic_stringstream<value_type> stream{};

    stream << "[" << std::setfill(' ') << std::setw(level_buffer_size)
           << get_log_string<value_type>(message.level) << "]";

    return stream.str();
}

template <>
inline auto stream_stringifier<wchar_t>::format_second(
    string_view_type /*logger*/, const message_type &message) -> string_type
{
    using size_type = std::string::size_type;

    constexpr const size_type level_buffer_size{8};

    std::basic_stringstream<value_type> stream{};

    stream << L"[" << std::setfill(L' ') << std::setw(level_buffer_size)
           << get_log_string<value_type>(message.level) << L"]";

    return stream.str();
}

template <>
inline auto stream_stringifier<char>::format_third(string_view_type logger,
                                                   const message_type &message)
    -> string_type
{
    std::basic_stringstream<value_type> stream{};

    stream << " [" << logger << "] " << message.content << "\n";

    return stream.str();
}

template <>
inline auto stream_stringifier<wchar_t>::format_third(
    string_view_type logger, const message_type &message) -> string_type
{
    std::basic_stringstream<value_type> stream{};

    stream << L" [" << logger << L"] " << message.content << L"\n";

    return stream.str();
}

template <typename CharT>
using stream_message_formatter =
    message_formatter_base<stream_message<CharT>, stream_stringifier<CharT>>;

template <typename CharT>
using stream_color_message_formatter =
    color_message_formatter_base<stream_message<CharT>,
                                 stream_stringifier<CharT>>;

} // namespace logency::message

#endif // LOGENCY_INCLUDE_LOGENCY_MESSAGE_STREAM_MESSAGE_HPP_
