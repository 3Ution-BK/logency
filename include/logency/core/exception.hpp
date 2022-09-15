#ifndef LOGENCY_INCLUDE_LOGENCY_CORE_EXCEPTION_HPP_
#define LOGENCY_INCLUDE_LOGENCY_CORE_EXCEPTION_HPP_

#include <cstring>

#include <stdexcept>
#include <string>
#include <system_error>

namespace logency
{

class runtime_error : public std::runtime_error
{
public:
    explicit runtime_error(const char *what);
    explicit runtime_error(const std::string &what);
};

class system_error : public runtime_error
{
public:
    explicit system_error(std::error_code code);
    explicit system_error(std::error_code code, const char *what);
    explicit system_error(std::error_code code, const std::string &what);

    [[nodiscard]] auto code() const noexcept -> const std::error_code &;

private:
    static auto make_what(std::error_code code, const std::string &string)
        -> std::string;

    std::error_code code_;
};

inline runtime_error::runtime_error(const char *what) : std::runtime_error{what}
{
}

inline runtime_error::runtime_error(const std::string &what)
    : std::runtime_error{what}
{
}

inline system_error::system_error(std::error_code code) : system_error{code, ""}
{
}

inline system_error::system_error(std::error_code code, const char *what)
    : system_error{code, std::string{what}}
{
}

inline system_error::system_error(std::error_code code, const std::string &what)
    : runtime_error{make_what(code, what)}, code_{code}
{
}

inline auto system_error::code() const noexcept -> const std::error_code &
{
    return code_;
}

inline auto system_error::make_what(std::error_code code,
                                    const std::string &string) -> std::string
{
    if (!string.empty())
    {
        auto message{code.message()};
        constexpr const char *seperator{": "};

        std::string buffer;
        buffer.reserve(string.size() + message.size() + std::strlen(seperator));
        buffer.append(string).append(seperator).append(message);

        return buffer;
    }

    return code.message();
}

} // namespace logency

#endif // LOGENCY_INCLUDE_LOGENCY_CORE_EXCEPTION_HPP_
