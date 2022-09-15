#ifndef LOGENCY_INCLUDE_LOGENCY_DETAIL_FILE_FILE_HELPER_HPP_
#define LOGENCY_INCLUDE_LOGENCY_DETAIL_FILE_FILE_HELPER_HPP_

#include <filesystem>
#include <iostream>
#include <map>
#include <regex>
#include <tuple>
#include <utility>

namespace logency::detail::file
{
void create_necessary_directory(const std::filesystem::path &path);

auto extract_file_extension(const std::filesystem::path &filename)
    -> std::tuple<std::filesystem::path, std::filesystem::path>;

inline void create_necessary_directory(const std::filesystem::path &path)
{
    if (!path.has_parent_path())
    {
        return;
    }
    auto parent{path.parent_path()};

    if (!std::filesystem::exists(parent))
    {
        std::ignore = std::filesystem::create_directories(parent);
    }
}

inline auto extract_file_extension(const std::filesystem::path &filename)
    -> std::tuple<std::filesystem::path, std::filesystem::path>
{
    auto front{std::filesystem::path{filename}.replace_extension("")};
    auto extension{filename.has_extension() ? filename.extension()
                                            : std::filesystem::path{}};

    return std::make_tuple(front, extension);
}

} // namespace logency::detail::file

#endif // LOGENCY_INCLUDE_LOGENCY_DETAIL_FILE_FILE_HELPER_HPP_
