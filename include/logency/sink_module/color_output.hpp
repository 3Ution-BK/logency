#ifndef LOGENCY_INCLUDE_LOGENCY_SINK_MODULE_COLOR_OUTPUT_HPP_
#define LOGENCY_INCLUDE_LOGENCY_SINK_MODULE_COLOR_OUTPUT_HPP_

#include <string>
#include <type_traits>
#include <vector>

namespace logency::sink_module
{

/**
 * \brief console color
 *
 * This enum represent the available console color.
 *
 * \note Some terminal might not implement the requested color. In this case, we
 * will warn the user with the star sign (*) and the message. It is user's
 * responsibility to set the console color properly.
 *
 * (* background): Some terminal may not implement the target background color.
 */
enum class console_color : unsigned int
{
    black = 0x00,               //!< black
    blue = 0x01,                //!< blue
    green = 0x02,               //!< green
    cyan = green | blue,        //!< cyan
    red = 0x04,                 //!< red
    magenta = red | blue,       //!< magenta
    yellow = red | green,       //!< yellow
    white = red | green | blue, //!< white

    intense = 0x08, //!< Combine it with other value to produce intense effect.

    intense_black = black | intense,     //!< intense black (* background)
    intense_blue = blue | intense,       //!< intense blue (* background)
    intense_green = green | intense,     //!< intense green (* background)
    intense_cyan = cyan | intense,       //!< intense cyan (* background)
    intense_red = red | intense,         //!< intense red (* background)
    intense_magenta = magenta | intense, //!< intense magenta (* background)
    intense_yellow = yellow | intense,   //!< intense yellow (* background)
    intense_white = white | intense,     //!< intense white (* background)

    original = 0x10 //!< Original console color. (Set the value to original)
};

auto operator~(console_color rhs) -> console_color;
auto operator|(console_color lhs, console_color rhs) -> console_color;
auto operator&(console_color lhs, console_color rhs) -> console_color;
auto operator^(console_color lhs, console_color rhs) -> console_color;
auto operator|=(console_color &lhs, console_color rhs) -> console_color &;
auto operator&=(console_color &lhs, console_color rhs) -> console_color &;
auto operator^=(console_color &lhs, console_color rhs) -> console_color &;

struct color_attribute
{
    console_color foreground{console_color::original};
    console_color background{console_color::original};
};

template <typename CharT, typename Traits = std::char_traits<CharT>>
struct color_message
{
    using value_type = CharT;
    using traits_type = Traits;
    using string_type = std::basic_string<value_type, traits_type>;
    using color_attribute_type = color_attribute;

    string_type message;
    color_attribute_type color;
};

inline auto operator~(console_color rhs) -> console_color
{
    return static_cast<console_color>(
        ~static_cast<std::underlying_type<console_color>::type>(rhs));
}

inline auto operator|(console_color lhs, console_color rhs) -> console_color
{
    return static_cast<console_color>(
        static_cast<std::underlying_type<console_color>::type>(lhs) |
        static_cast<std::underlying_type<console_color>::type>(rhs));
}

inline auto operator&(console_color lhs, console_color rhs) -> console_color
{
    return static_cast<console_color>(
        static_cast<std::underlying_type<console_color>::type>(lhs) &
        static_cast<std::underlying_type<console_color>::type>(rhs));
}

inline auto operator^(console_color lhs, console_color rhs) -> console_color
{
    return static_cast<console_color>(
        static_cast<std::underlying_type<console_color>::type>(lhs) ^
        static_cast<std::underlying_type<console_color>::type>(rhs));
}

inline auto operator|=(console_color &lhs, console_color rhs) -> console_color &
{
    lhs = lhs | rhs;
    return lhs;
}

inline auto operator&=(console_color &lhs, console_color rhs) -> console_color &
{
    lhs = lhs & rhs;
    return lhs;
}

inline auto operator^=(console_color &lhs, console_color rhs) -> console_color &
{
    lhs = lhs ^ rhs;
    return lhs;
}

} // namespace logency::sink_module

#endif // LOGENCY_INCLUDE_LOGENCY_SINK_MODULE_COLOR_OUTPUT_HPP_
