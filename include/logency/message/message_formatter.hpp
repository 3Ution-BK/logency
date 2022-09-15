#ifndef LOGENCY_INCLUDE_LOGENCY_MESSAGE_MESSAGE_FORMATTER_HPP_
#define LOGENCY_INCLUDE_LOGENCY_MESSAGE_MESSAGE_FORMATTER_HPP_

#include "log_level.hpp"

#include "logency/sink_module/color_output.hpp"

#include <vector>

namespace logency::message
{

template <typename MessageType, typename StringifierType>
class message_formatter_base
{
public:
    using message_type = MessageType;
    using string_type = typename message_type::string_type;
    using string_view_type = typename message_type::string_view_type;
    using output_type = string_type;

    using stringifier_type = StringifierType;

    auto operator()(string_view_type logger, const message_type &message) const
        -> output_type;

private:
    stringifier_type stringifier{};
};

template <typename MessageType, typename StringifierType>
class color_message_formatter_base
{
public:
    using message_type = MessageType;
    using value_type = typename message_type::value_type;
    using traits_type = typename message_type::traits_type;
    using string_type = typename message_type::string_type;
    using string_view_type = typename message_type::string_view_type;
    using message_attribute_type =
        logency::sink_module::color_message<value_type, traits_type>;
    using output_type = std::vector<message_attribute_type>;

    using stringifier_type = StringifierType;

    auto operator()(string_view_type logger, const message_type &message) const
        -> output_type;

private:
    using color_attribute_type =
        typename message_attribute_type::color_attribute_type;
    using color_type = logency::sink_module::console_color;

    [[nodiscard]] auto get_color(log_level level) const noexcept
        -> color_attribute_type;

    stringifier_type stringifier{};
};

template <typename MessageType, typename StringifierType>
inline auto message_formatter_base<MessageType, StringifierType>::operator()(
    string_view_type logger, const message_type &message) const -> output_type
{
    return stringifier.format(logger, message);
}

template <typename MessageType, typename StringifierType>
inline auto
color_message_formatter_base<MessageType, StringifierType>::operator()(
    string_view_type logger, const message_type &message) const -> output_type
{
    return output_type{
        message_attribute_type{stringifier.format_first(logger, message),
                               color_attribute_type{}},
        message_attribute_type{stringifier.format_second(logger, message),
                               get_color(message.level)},
        message_attribute_type{stringifier.format_third(logger, message),
                               color_attribute_type{}},
    };
}

template <typename MessageType, typename StringifierType>
inline auto
color_message_formatter_base<MessageType, StringifierType>::get_color(
    log_level level) const noexcept -> color_attribute_type
{
    switch (level)
    {
    case log_level::trace:
        return color_attribute_type{color_type::white, color_type::original};
    case log_level::debug:
        return color_attribute_type{color_type::cyan, color_type::original};
    case log_level::info:
        return color_attribute_type{color_type::green, color_type::original};
    case log_level::warning:
        return color_attribute_type{color_type::yellow, color_type::original};
    case log_level::error:
        return color_attribute_type{color_type::red, color_type::original};
    case log_level::critical:
        return color_attribute_type{color_type::intense_white, color_type::red};
    default:
#if !defined(NDEBUG)
        assert(false);
#else
    #if defined(__GNUC__) || defined(__GNUG__)
        __builtin_unreachable();
    #elif defined(_MSC_VER)
        __assume(false);
    #endif
#endif
    }
}

} // namespace logency::message

#endif // LOGENCY_INCLUDE_LOGENCY_MESSAGE_MESSAGE_FORMATTER_HPP_
