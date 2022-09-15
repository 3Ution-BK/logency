#ifndef LOGENCY_TEST_UTILS_TEST_MESSAGE_HPP_
#define LOGENCY_TEST_UTILS_TEST_MESSAGE_HPP_

#include <cstddef>
#include <string>
#include <string_view>

namespace logency::unit_test::utils
{

template <typename value_type>
auto not_used() -> std::basic_string<value_type>;

template <typename CharT>
struct message
{
    using value_type = CharT;
    using traits_type = std::char_traits<value_type>;
    using string_type = std::basic_string<value_type>;
    using string_view_type = std::basic_string_view<value_type>;

    explicit message() : content{} {}
    explicit message(string_view_type message) : content{message} {}
    explicit message(string_type message, std::size_t position)
        : content{message, position}
    {
    }

    auto operator()() const -> string_type { return content; }

    string_type content;
};

template <typename CharT>
class formatter
{
    static_assert(std::is_same<CharT, char>::value ||
                      std::is_same<CharT, wchar_t>::value,
                  "Character type not supported.");

public:
    using value_type = CharT;
    using string_type = std::basic_string<value_type>;
    using string_view_type = std::basic_string_view<value_type>;

    using message_type = message<value_type>;

    auto operator()(string_view_type /* logger */,
                    const message_type &message) const -> string_type
    {
        return message.content;
    }
};

template <typename value_type>
auto not_used() -> std::basic_string<value_type>
{
    return std::basic_string<value_type>{};
}

} // namespace logency::unit_test::utils

#endif // LOGENCY_TEST_UTILS_TEST_MESSAGE_HPP_
