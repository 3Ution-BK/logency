#ifndef LOGENCY_INCLUDE_LOGENCY_DETAIL_FILE_BASIC_FILE_HPP_
#define LOGENCY_INCLUDE_LOGENCY_DETAIL_FILE_BASIC_FILE_HPP_

#include "logency/core/exception.hpp"
#include "logency/detail/file/file_helper.hpp"

#include <cassert>

#include <filesystem>
#include <fstream>
#include <string>
#include <type_traits>

namespace logency
{

enum class file_open_mode : std::ios_base::openmode
{
    append = std::ios_base::out | std::ios_base::app,
    truncate = std::ios_base::out | std::ios_base::trunc
};

namespace detail::file
{
/**
 * \brief This class represent the file_object.
 *
 * It open the file in the constructor, and close the file while it destruct.
 *
 * \par Close the file.
 * It will not delete the file when it destruct. Instead, it will close the
 * file.
 *
 * \par Throw
 * Because it is related to the filesystem in your devices, expect to throw
 * filesystem error when the operation failed to operate.
 * When it throws, we suggested to close the file.
 *
 * \tparam Chat Character type for the file content.
 */
template <typename CharT, typename Traits = std::char_traits<CharT>>
class basic_file
{
public:
    using value_type = CharT;
    using traits_type = Traits;
    using string_view_type =
        typename std::basic_string_view<value_type, traits_type>;
    using stream_type = typename std::basic_fstream<value_type, traits_type>;

    /**
     * \brief Initializes a new instance of the basic_file object class
     *
     * Create the file named \a filename and its \a mode. It will create the
     * file if \a filename is not existed. Otherwise, it will open it.
     *
     * \param filename Specified file name.
     * \param mode Specified mode.
     * \throw logency::runtime_error if the file failed to open.
     * \throw Anything that std::basic_fstream throws when it construct.
     */
    explicit basic_file(const char *filename, file_open_mode mode);

    /**
     * \overload
     *
     * \param filename Specified file name.
     * \param mode Specified mode.
     * \throw logency::runtime_error if the file failed to open.
     * \throw Anything that std::basic_fstream throws when it construct.
     */
    explicit basic_file(const std::string &filename, file_open_mode mode);

    /**
     * \overload
     *
     * \note
     * This constructor is enable only when \c std::filesystem::path::value_type
     * is not \c char. (e.g. Windows filesystem)
     *
     * \param filename Specified file name.
     * \param mode Specified mode.
     * \throw logency::runtime_error if the file failed to open.
     * \throw Anything that std::basic_fstream throws when it construct.
     */
    template <typename = typename std::enable_if<!std::is_same<
                  std::filesystem::path::value_type, char>::value>::type>
    explicit basic_file(const std::filesystem::path::value_type *filename,
                        file_open_mode mode);

    /**
     * \overload
     *
     * \note
     * This constructor is enable only when \c std::filesystem::path::value_type
     * is not \c char. (e.g. Windows filesystem)
     *
     * \note
     * std::fstream does not provide construction from std::basic_string except
     * std::string. Hence we provide it as an additional functionality.
     *
     * \param filename Specified file name.
     * \param mode Specified mode.
     * \throw logency::runtime_error if the file failed to open.
     * \throw Anything that std::basic_fstream throws when it construct.
     */
    template <typename = typename std::enable_if<!std::is_same<
                  std::filesystem::path::value_type, char>::value>::type>
    explicit basic_file(
        const std::basic_string<std::filesystem::path::value_type> &filename,
        file_open_mode mode);

    /**
     * \overload
     *
     * \param filename Specified file name.
     * \param mode Specified mode.
     * \throw logency::runtime_error if the file failed to open.
     * \throw Anything that std::basic_fstream throws when it construct.
     */
    template <typename FsPath = std::filesystem::path>
    explicit basic_file(const FsPath &filename, file_open_mode mode);

    /**
     * \brief Destroy the instance of the basic_file object class.
     *
     * It will close the file instead of delete.
     */
    ~basic_file();

    basic_file(const basic_file &other) = delete;
    auto operator=(const basic_file &other) -> basic_file & = delete;
    basic_file(basic_file &&other) noexcept = delete;
    auto operator=(basic_file &&other) noexcept -> basic_file & = delete;

    /**
     * \brief write the \a buffer to the file.
     *
     * \param buffer Specified buffer.
     * \throw logency::runtime_error when it failed to write.
     * \throw Anything that std::basic_fstream::write throws.
     */
    void write(string_view_type buffer);

    /**
     * \brief flush the file_object.
     *
     * \throw logency::runtime_error when it failed to flush.
     * \throw Anything that std::basic_fstream::flush throws.
     */
    void flush();

private:
    stream_type stream_{};
};

template <typename CharT, typename Traits>
basic_file<CharT, Traits>::basic_file(const char *filename, file_open_mode mode)
{
    create_necessary_directory(filename);

    stream_.open(filename, static_cast<std::ios_base::openmode>(mode));

    if (!stream_)
    {
        throw logency::system_error(
            std::error_code{errno, std::generic_category()},
            "Failed to open file"); // No period needed.
    }

    assert(stream_.is_open());
}

template <typename CharT, typename Traits>
basic_file<CharT, Traits>::basic_file(const std::string &filename,
                                      file_open_mode mode)
    : basic_file{filename.c_str(), mode}
{
}

template <typename CharT, typename Traits>
template <typename>
basic_file<CharT, Traits>::basic_file(
    const std::filesystem::path::value_type *filename, file_open_mode mode)
{
    create_necessary_directory(filename);

    stream_.open(filename, static_cast<std::ios_base::openmode>(mode));

    if (!stream_)
    {
        throw logency::system_error(
            std::error_code{errno, std::generic_category()},
            "Failed to open file"); // No period needed.
    }

    assert(stream_.is_open());
}

template <typename CharT, typename Traits>
template <typename>
basic_file<CharT, Traits>::basic_file(
    const std::basic_string<std::filesystem::path::value_type> &filename,
    file_open_mode mode)
    : basic_file{filename.c_str(), mode}
{
}

template <typename CharT, typename Traits>
template <typename FsPath>
basic_file<CharT, Traits>::basic_file(const FsPath &filename,
                                      file_open_mode mode)
    : basic_file{filename.c_str(), mode}
{
}

template <typename CharT, typename Traits>
basic_file<CharT, Traits>::~basic_file() = default;

template <typename CharT, typename Traits>
void basic_file<CharT, Traits>::flush()
{
    assert(stream_.is_open());

    stream_.flush();
    if (stream_.bad())
    {
        throw logency::system_error(
            std::error_code{errno, std::generic_category()},
            "Failed to flush the file"); // No period needed.
    }
}

template <typename CharT, typename Traits>
void basic_file<CharT, Traits>::write(string_view_type buffer)
{
    assert(stream_.is_open());

    stream_.write(buffer.data(), static_cast<std::streamsize>(buffer.size()));

    if (stream_.bad())
    {
        throw logency::system_error(
            std::error_code{errno, std::generic_category()},
            "Failed to write content"); // No period needed.
    }
}

} // namespace detail::file

} // namespace logency

#endif // LOGENCY_INCLUDE_LOGENCY_DETAIL_FILE_BASIC_FILE_HPP_
