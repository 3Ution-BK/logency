#ifndef LOGENCY_INCLUDE_LOGENCY_DETAIL_INCLUDE_WINDOWS_HPP_
#define LOGENCY_INCLUDE_LOGENCY_DETAIL_INCLUDE_WINDOWS_HPP_

#if defined(_WIN32)
    #pragma warning(push)
    /**
     * [MSVC C4668] 'symbol' is not defined as a preprocessor macro.
     */
    #pragma warning(disable : 4668)
    /**
     * [MSVC C5039] 'function': pointer or reference to potentially throwing
     * function passed to extern C function under -EHc.
     */
    #pragma warning(disable : 5039)

    #include <Windows.h>

    #pragma warning(pop)
#endif

#include <iostream>

namespace logency::detail::os
{

template <typename CharT>
auto get_handle(std::basic_ostream<CharT> *stream) -> HANDLE;

template <>
inline auto get_handle(std::basic_ostream<char> *stream) -> HANDLE
{
    // NOLINTNEXTLINE(*-no-int-to-ptr, *-pro-type-cstyle-cast)
    HANDLE handle{INVALID_HANDLE_VALUE};

    if (stream->rdbuf() == std::cout.rdbuf())
    {
        handle = GetStdHandle(STD_OUTPUT_HANDLE);
    }
    else if (stream->rdbuf() == std::cerr.rdbuf())
    {
        handle = GetStdHandle(STD_ERROR_HANDLE);
    }

    return handle;
}

template <>
inline auto get_handle(std::basic_ostream<wchar_t> *stream) -> HANDLE
{
    // NOLINTNEXTLINE(*-no-int-to-ptr, *-pro-type-cstyle-cast)
    HANDLE handle{INVALID_HANDLE_VALUE};

    if (stream->rdbuf() == std::wcout.rdbuf())
    {
        handle = GetStdHandle(STD_OUTPUT_HANDLE);
    }
    else if (stream->rdbuf() == std::wcerr.rdbuf())
    {
        handle = GetStdHandle(STD_ERROR_HANDLE);
    }

    return handle;
}

} // namespace logency::detail::os

#endif // LOGENCY_INCLUDE_LOGENCY_DETAIL_INCLUDE_WINDOWS_HPP_
