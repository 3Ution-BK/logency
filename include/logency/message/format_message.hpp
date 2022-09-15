#ifndef LOGENCY_INCLUDE_LOGENCY_MESSAGE_FORMAT_MESSAGE_HPP_
#define LOGENCY_INCLUDE_LOGENCY_MESSAGE_FORMAT_MESSAGE_HPP_

#include "log_level.hpp"
#include "message_formatter.hpp"
#include "time.hpp"

#include "logency/sink_module/color_output.hpp"

#include <chrono>
#include <format>
#include <vector>

namespace logency::message
{

/**
 * \brief This struct represent the basic message with the necessary basic data.
 */
struct format_message
{
    using value_type = char;
    using traits_type = std::char_traits<value_type>;
    using string_type = std::basic_string<value_type, traits_type>;
    using string_view_type = std::basic_string_view<value_type, traits_type>;
    using clock_type = std::chrono::system_clock;

    explicit format_message(log_level level, string_view_type content);
    explicit format_message(log_level level, string_type &&content);

    template <typename... Args>
    explicit format_message(log_level level, string_view_type fmt,
                            Args &&...args);

    string_type content;
    clock_type::time_point time;
    log_level level;
};

struct format_stringifier
{
    using message_type = format_message;
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

inline format_message::format_message(log_level level, string_view_type content)
    : content{content}, time{clock_type::now()}, level{level}
{
}

inline format_message::format_message(log_level level, string_type &&content)
    : content{std::move(content)}, time{clock_type::now()}, level{level}
{
}

template <typename... Args>
format_message::format_message(log_level level, string_view_type fmt,
                               Args &&...args)
    : content{std::format(fmt, std::forward<Args>(args)...)},
      time{clock_type::now()}, level{level}
{
}

inline auto format_stringifier::format(string_view_type logger,
                                       const message_type &message)
    -> string_type
{
    /**
     * `std::format("{}{}{}", ...);` needs to call std::format 4 times.
     *
     * This function will be called a lot of time (if the logger need to parse
     * the string value, which is likely to happen). It will speed up the
     * performance if we do it once.
     */
    const int milliseconds{get_ms(message.time)};

    return std::format(
        "[{0:%F %H:%M:%S}.{1:03}] [{2:>8}] [{3}] {4}\n",
        std::chrono::time_point_cast<std::chrono::seconds>(message.time),
        milliseconds, get_log_string<char>(message.level), logger,
        message.content);
}

inline auto format_stringifier::format_first(string_view_type /*logger*/,
                                             const message_type &message)
    -> string_type
{
    const int milliseconds{get_ms(message.time)};

    return std::format(
        "[{0:%F %H:%M:%S}.{1:03}] ",
        std::chrono::time_point_cast<std::chrono::seconds>(message.time),
        milliseconds);
}

inline auto format_stringifier::format_second(string_view_type /*logger*/,
                                              const message_type &message)
    -> string_type
{
    return std::format("[{:>8}]", get_log_string<char>(message.level));
}

inline auto format_stringifier::format_third(string_view_type logger,
                                             const message_type &message)
    -> string_type
{
    return std::format(" [{}] {}\n", logger, message.content);
}

using format_message_formatter =
    message_formatter_base<format_message, format_stringifier>;

using format_color_message_formatter =
    color_message_formatter_base<format_message, format_stringifier>;

} // namespace logency::message

#endif // LOGENCY_INCLUDE_LOGENCY_MESSAGE_FORMAT_MESSAGE_HPP_
