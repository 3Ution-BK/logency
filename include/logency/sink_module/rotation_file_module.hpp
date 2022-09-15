#ifndef LOGENCY_INCLUDE_LOGENCY_SINK_MODULE_ROTATION_FILE_MODULE_HPP_
#define LOGENCY_INCLUDE_LOGENCY_SINK_MODULE_ROTATION_FILE_MODULE_HPP_

#include "logency/core/exception.hpp"
#include "logency/detail/file/basic_file.hpp"
#include "logency/detail/file/file_helper.hpp"
#include "module_interface.hpp"

#include <cassert>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <optional>
#include <string>
#include <system_error>
#include <type_traits>

namespace logency::sink_module
{

namespace rotation_file
{

enum class construct_mode
{
    append_previous,
    create_new_file
};

struct rotate_info
{
    using file_size_type = std::uintmax_t;

    file_size_type file_size{0U};
    int file_count{0};
};

} // namespace rotation_file

template <typename MessageType, typename Formatter>
class rotation_file_module : public module_interface<MessageType>
{
    using base_type = module_interface<MessageType>;

public:
    using message_type = typename base_type::message_type;
    using value_type = typename message_type::value_type;
    using traits_type = typename message_type::traits_type;
    using string_type = typename message_type::string_type;
    using string_view_type = typename message_type::string_view_type;

    using formatter_type = Formatter;

    using file_type =
        logency::detail::file::basic_file<value_type, traits_type>;

    using construct_mode = rotation_file::construct_mode;
    using file_size_type = rotation_file::rotate_info::file_size_type;
    using rotate_info = rotation_file::rotate_info;

    static_assert(
        std::is_convertible<typename decltype(std::function{
                                std::declval<formatter_type>()})::result_type,
                            string_type>::value,
        "Formatter output cannot transfer input message to \"string_type\".");

    template <typename CharT>
    explicit rotation_file_module(const CharT *name, rotate_info rotate_info,
                                  construct_mode mode,
                                  std::unique_ptr<formatter_type> formatter);

    template <typename CharT>
    explicit rotation_file_module(const std::basic_string<CharT> &name,
                                  rotate_info rotate_info, construct_mode mode,
                                  std::unique_ptr<formatter_type> formatter);

    ~rotation_file_module() override;

    rotation_file_module(const rotation_file_module &other) = delete;
    rotation_file_module(rotation_file_module &&other) noexcept = delete;
    auto operator=(const rotation_file_module &other)
        -> rotation_file_module & = delete;
    auto operator=(rotation_file_module &&other) noexcept
        -> rotation_file_module & = delete;

    /**
     * \copydoc module_interface::flush
     */
    void flush() override;

    /**
     * \copydoc module_interface::log_message
     */
    void log_message(string_view_type logger,
                     const message_type &message) override;

private:
    using path_type = std::filesystem::path;
    using file_value_type = path_type::value_type;
    using filename_type = std::basic_string<file_value_type>;

    struct file_info
    {
        path_type name;
        path_type parent;
        path_type front;
        path_type extension;
    };

    static auto parse_filename(const file_info &file, int index) -> path_type;

    static void get_file_info(const path_type &name, file_info &info);
    static auto get_file_size(const path_type &name) -> std::optional<size_t>;

    void close_file();
    void rotate_file();
    void rotate();
    bool should_rotate(file_size_type offset);
    void open_file();

    std::unique_ptr<file_type> file_;
    file_info file_info_;
    const rotate_info rotate_info_;
    file_size_type current_size_{};

    std::unique_ptr<formatter_type> formatter_;
};

template <typename MessageType, typename Formatter>
template <typename CharT>
rotation_file_module<MessageType, Formatter>::rotation_file_module(
    const CharT *name, rotate_info rotate_info, construct_mode mode,
    std::unique_ptr<formatter_type> formatter)
    : rotation_file_module{std::basic_string<CharT>{name}, rotate_info, mode,
                           std::move(formatter)}
{
}

template <typename MessageType, typename Formatter>
template <typename CharT>
rotation_file_module<MessageType, Formatter>::rotation_file_module(
    const std::basic_string<CharT> &name, rotate_info rotate_info,
    construct_mode mode, std::unique_ptr<formatter_type> formatter)
    : rotate_info_{rotate_info}, formatter_{std::move(formatter)}
{
    if (rotate_info_.file_size <= 0)
    {
        throw logency::runtime_error("file size should be smaller than zero.");
    }

    if (rotate_info_.file_count <= 0)
    {
        throw logency::runtime_error("file count should be positive integer.");
    }

    get_file_info(name, file_info_);

    if (!std::filesystem::exists(file_info_.name))
    {
        open_file();
    }
    else
    {
        open_file();
        if (mode == construct_mode::create_new_file || should_rotate(0))
        {
            rotate();
        }
    }
}

template <typename MessageType, typename Formatter>
rotation_file_module<MessageType, Formatter>::~rotation_file_module() = default;

template <typename MessageType, typename Formatter>
void rotation_file_module<MessageType, Formatter>::close_file()
{
    file_.reset();
}

template <typename MessageType, typename Formatter>
void rotation_file_module<MessageType, Formatter>::rotate_file()
{
    assert(std::filesystem::exists(file_info_.name));

    /**
     * Do file replace for `rotate_info_.file_count - 1` times.
     *
     * e.g. rotate_info_.file_count = 3
     * a-1.txt -> a-2.txt
     * a.txt -> a-1.txt
     * a-2.txt is deleted by replacing file from a-1.txt
     */
    for (int i{rotate_info_.file_count - 1}; i > 0; --i)
    {
        auto source{parse_filename(file_info_, i - 1)};

        if (!std::filesystem::exists(source))
        {
            continue;
        }

        auto target{parse_filename(file_info_, i)};

        std::error_code code;
        std::filesystem::rename(source, target, code);
        if (code)
        {
            throw logency::system_error(code, "Failed to rotate file");
        }
    }
}

template <typename MessageType, typename Formatter>
void rotation_file_module<MessageType, Formatter>::flush()
{
    file_->flush();
}

template <typename MessageType, typename Formatter>
void rotation_file_module<MessageType, Formatter>::get_file_info(
    const path_type &name, file_info &info)
{
    auto extracted{detail::file::extract_file_extension(name)};
    info.name = name;
    info.front = std::get<0>(extracted).concat("-");
    info.extension = std::get<1>(extracted);
}

template <typename MessageType, typename Formatter>
auto rotation_file_module<MessageType, Formatter>::get_file_size(
    const path_type &name) -> std::optional<size_t>
{
    std::error_code code;

    auto size{std::filesystem::file_size(name, code)};

    if (code)
    {
        return std::nullopt;
    }

    return static_cast<size_t>(size);
}

template <typename MessageType, typename Formatter>
void rotation_file_module<MessageType, Formatter>::log_message(
    string_view_type logger, const message_type &message)
{
    auto formatted_message{(*formatter_)(logger, message)};

    const auto size{static_cast<file_size_type>(formatted_message.size())};

    if (should_rotate(size))
    {
        rotate();
    }

    file_->write(formatted_message);
    current_size_ += size;
}

template <typename MessageType, typename Formatter>
void rotation_file_module<MessageType, Formatter>::open_file()
{
    constexpr auto mode{file_open_mode::append};

    file_ = std::make_unique<file_type>(file_info_.name, mode);

    current_size_ = get_file_size(file_info_.name).value_or(0);
}

template <typename MessageType, typename Formatter>
auto rotation_file_module<MessageType, Formatter>::parse_filename(
    const file_info &file, int index) -> path_type
{
    if (index > 0)
    {
        return path_type{file.front}
            .concat(std::to_string(index))
            .concat(file.extension.native());
    }

    return file.name;
}

template <typename MessageType, typename Formatter>
void rotation_file_module<MessageType, Formatter>::rotate()
{
    close_file();
    {
        assert(std::filesystem::exists(file_info_.name));

        rotate_file();
    }
    open_file();
}

template <typename MessageType, typename Formatter>
bool rotation_file_module<MessageType, Formatter>::should_rotate(
    file_size_type offset)
{
    return current_size_ + offset >= rotate_info_.file_size;
}

} // namespace logency::sink_module

#endif // LOGENCY_INCLUDE_LOGENCY_SINK_MODULE_ROTATION_FILE_MODULE_HPP_
