#ifndef LOGENCY_INCLUDE_LOGENCY_DETAIL_INCLUDE_LINUX_HPP_
#define LOGENCY_INCLUDE_LOGENCY_DETAIL_INCLUDE_LINUX_HPP_

#if !defined(_WIN32)

    #include <cstdio>
    #include <unistd.h>

    #include <iostream>

namespace logency::detail::os
{

template <typename CharT>
int get_std_fd(std::basic_ostream<CharT> *stream);

template <typename CharT>
bool is_terminal(std::basic_ostream<CharT> *stream);

template <>
inline int get_std_fd(std::basic_ostream<char> *stream)
{
    if (stream->rdbuf() == std::cout.rdbuf())
    {
        return fileno(stdout);
    }
    else if (stream->rdbuf() == std::cerr.rdbuf())
    {
        return fileno(stderr);
    }

    return -1;
}

template <>
inline int get_std_fd(std::basic_ostream<wchar_t> *stream)
{
    if (stream->rdbuf() == std::cout.rdbuf())
    {
        return fileno(stdout);
    }
    else if (stream->rdbuf() == std::cerr.rdbuf())
    {
        return fileno(stderr);
    }

    return -1;
}

template <typename CharT>
inline bool is_terminal(std::basic_ostream<CharT> *stream)
{
    return isatty(get_std_fd(stream)) != 0;
}

} // namespace logency::detail::os

#endif

#endif // LOGENCY_INCLUDE_LOGENCY_DETAIL_INCLUDE_LINUX_HPP_
