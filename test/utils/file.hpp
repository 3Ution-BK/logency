#ifndef LOGENCY_TEST_TEST_HELP_MODULES_FILE_HPP_
#define LOGENCY_TEST_TEST_HELP_MODULES_FILE_HPP_

#include "logency/core/exception.hpp"

#include "include_doctest.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace logency::unit_test::utils::file
{

template <typename CharT, typename FileCharT = CharT>
auto get_content(const FileCharT *name) -> std::basic_string<CharT>;

template <typename CharT, typename FileCharT = CharT>
auto get_content(const std::basic_string<FileCharT> &name)
    -> std::basic_string<CharT>;

template <typename CharT, typename FileCharT = CharT>
void set_content(const FileCharT *name,
                 const std::basic_string<CharT> &content);

template <typename CharT, typename FileCharT = CharT>
void set_content(const std::basic_string<FileCharT> &name,
                 const std::basic_string<CharT> &content);

void touch(const std::filesystem::path &name);

template <typename CharT, typename FileCharT>
auto get_content(const FileCharT *name) -> std::basic_string<CharT>
{
    std::basic_fstream<CharT> file{name,
                                   std::ios_base::in | std::ios_base::app};
    if (!file)
    {
        return std::basic_string<CharT>{};
    }

    std::basic_stringstream<CharT> buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

template <typename CharT, typename FileCharT>
auto get_content(const std::basic_string<FileCharT> &name)
    -> std::basic_string<CharT>
{
    return get_content<CharT, FileCharT>(name.c_str());
}

template <typename CharT, typename FileCharT>
void set_content(const FileCharT *name, const std::basic_string<CharT> &content)
{
    std::filesystem::path where{name};

    if (where.has_parent_path() &&
        !std::filesystem::exists(where.parent_path()))
    {
        std::ignore = std::filesystem::create_directories(where.parent_path());
    }

    std::basic_fstream<CharT> file{name,
                                   std::ios_base::out | std::ios_base::trunc};

    if (!file)
    {
        std::string what{logency::system_error(
                             std::error_code{errno, std::generic_category()},
                             "failed to open file")
                             .what()};
        FAIL(what);
        return;
    }

    file << content;

    if (!file)
    {
        std::string what{logency::system_error(
                             std::error_code{errno, std::generic_category()},
                             "failed to write file")
                             .what()};

        FAIL(what);
        return;
    }
}

template <typename CharT, typename FileCharT>
void set_content(const std::basic_string<FileCharT> &name,
                 const std::basic_string<CharT> &content)
{
    return set_content<CharT, FileCharT>(name.c_str(), content);
}

inline void touch(const std::filesystem::path &name)
{
    std::ignore = std::ofstream{name};
}

} // namespace logency::unit_test::utils::file

#endif // LOGENCY_TEST_TEST_HELP_MODULES_FILE_HPP_
