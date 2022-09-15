#ifndef LOGENCY_INCLUDE_LOGENCY_SINK_MODULE_BASIC_FILE_MODULE_HPP_
#define LOGENCY_INCLUDE_LOGENCY_SINK_MODULE_BASIC_FILE_MODULE_HPP_

#include "logency/detail/file/basic_file.hpp"
#include "module_interface.hpp"

#include <cassert>

#include <chrono>
#include <functional>
#include <string>
#include <type_traits>

namespace logency::sink_module
{

/**
 * \brief This class represent the single file sink.
 *
 * This class is using fstream as the sink. It will open= the file on
 * construction, and close the file on destruction.
 *
 * \tparam MessageType MessageType type.
 * \tparam Formatter Formatter type.
 */
template <typename MessageType, typename Formatter>
class basic_file_module : public module_interface<MessageType>
{
    using base_type = module_interface<MessageType>;

public:
    using message_type = typename base_type::message_type;
    using value_type = typename message_type::value_type;
    using string_type = typename message_type::string_type;
    using string_view_type = typename message_type::string_view_type;
    using formatter_type = Formatter;

    using file_type = logency::detail::file::basic_file<value_type>;
    using file_open_mode = logency::file_open_mode;

    static_assert(
        std::is_convertible<typename decltype(std::function{
                                std::declval<formatter_type>()})::result_type,
                            string_type>::value,
        "Formatter output cannot transfer input message to "
        "\"std::basic_string<value_type>\".");

    template <typename CharT>
    explicit basic_file_module(const CharT *name, file_open_mode mode,
                               std::unique_ptr<formatter_type> formatter);

    template <typename CharT>
    explicit basic_file_module(const std::basic_string<CharT> &name,
                               file_open_mode mode,
                               std::unique_ptr<formatter_type> formatter);

    ~basic_file_module() override;

    basic_file_module(const basic_file_module &other) = delete;
    basic_file_module(basic_file_module &&other) noexcept = delete;
    basic_file_module &operator=(const basic_file_module &other) = delete;
    basic_file_module &operator=(basic_file_module &&other) noexcept = delete;

    /**
     * \copydoc module_interface::flush
     */
    void flush() override;

    /**
     * \copydoc module_interface::log_message
     */
    void log_message(string_view_type logger,
                     const message_type &message) override;

protected:
    template <typename T>
    void log_to_stream(const T &value);

private:
    file_type file_;                            //!< represent the file.
    std::unique_ptr<formatter_type> formatter_; //!< message formatter.
};

template <typename MessageType, typename Formatter>
template <typename CharT>
basic_file_module<MessageType, Formatter>::basic_file_module(
    const CharT *name, file_open_mode mode,
    std::unique_ptr<formatter_type> formatter)
    : file_{name, mode}, formatter_{std::move(formatter)}
{
}

template <typename MessageType, typename Formatter>
template <typename CharT>
basic_file_module<MessageType, Formatter>::basic_file_module(
    const std::basic_string<CharT> &name, file_open_mode mode,
    std::unique_ptr<formatter_type> formatter)
    : file_{name, mode}, formatter_{std::move(formatter)}
{
}

template <typename MessageType, typename Formatter>
basic_file_module<MessageType, Formatter>::~basic_file_module() = default;

template <typename MessageType, typename Formatter>
void basic_file_module<MessageType, Formatter>::flush()
{
    file_.flush();
}

template <typename MessageType, typename Formatter>
void basic_file_module<MessageType, Formatter>::log_message(
    string_view_type logger, const message_type &message)
{
    auto formatted_message{(*formatter_)(logger, message)};

    log_to_stream(formatted_message);
}

template <typename MessageType, typename Formatter>
template <typename T>
void basic_file_module<MessageType, Formatter>::log_to_stream(const T &value)
{
    file_.write(value);
}

} // namespace logency::sink_module

#endif // LOGENCY_INCLUDE_LOGENCY_SINK_MODULE_BASIC_FILE_MODULE_HPP_
