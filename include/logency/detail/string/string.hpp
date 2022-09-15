#ifndef LOGENCY_INCLUDE_LOGENCY_DETAIL_STRING_STRING_HPP_
#define LOGENCY_INCLUDE_LOGENCY_DETAIL_STRING_STRING_HPP_

#include <algorithm>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

/**
 * \brief This namspace represent basic string manipulation.
 */
namespace logency::detail::string
{

/**
 * \brief Concatenate multiple strings together.
 *
 * This function will allocate enough buffer first to parse all the content into
 * it.
 *
 * \note
 * This function is specialized for std::string.
 *
 * \param arguments Specified strings.
 * \return Requested result.
 * \throw Whatever std::basic_string constructor, std::copy_n throw.
 *
 * \sa concat
 */
auto concat_list(std::initializer_list<std::string_view> arguments)
    -> std::string;

/**
 * \brief Concatenate multiple strings together.
 *
 * This function will call concat_list to perform string concatenate.
 *
 * \note
 * This function is specialized for std::string.
 *
 * \tparam Args Any string type.
 * \param arg Specified strings.
 * \param args Specified strings.
 * \return Requested result.
 * \throw Whatever concat_list, std::string_view constructor throw.
 *
 * \sa concat_list
 */
template <typename... Args>
auto concat(std::string_view arg, Args &&...args) -> std::string;

inline auto concat_list(std::initializer_list<std::string_view> arguments)
    -> std::string
{
    size_t count{0};
    for (const auto &argument : arguments)
    {
        count += argument.size();
    }

    std::string result(count, 0);

    for (auto [from, to] = std::tuple{arguments.begin(), result.begin()};
         from != arguments.end(); ++from)
    {
        std::copy_n(from->begin(), from->size(), to);

        to += static_cast<int>(from->size());
    }
    return result;
}

template <typename... Args>
inline auto concat(std::string_view arg, Args &&...args) -> std::string
{
    return concat_list({arg, std::forward<Args>(args)...});
}

} // namespace logency::detail::string

#endif // LOGENCY_INCLUDE_LOGENCY_DETAIL_STRING_STRING_HPP_
