#ifndef LOGENCY_INCLUDE_LOGENCY_SINK_MODULE_COLOR_CONSOLE_MODULE_BASE_HPP_
#define LOGENCY_INCLUDE_LOGENCY_SINK_MODULE_COLOR_CONSOLE_MODULE_BASE_HPP_

#include "color_output.hpp"
#include "logency/detail/thread/console_mutex.hpp"
#include "module_interface.hpp"

#include <cassert>

#include <functional>
#include <mutex>
#include <ostream>

namespace logency::sink_module
{

/**
 * \brief Determine if the color should be parsed when the sink should log
 * the output.
 *
 * This flag is useful to detect if the stream is attached to the valid
 * console.
 */
enum class color_mode
{
    on,        //!< Always parse the color output.
    automatic, //!< Automatic detect if the color should be parse or not.
    off        //!< Do not parse the color output.
};

template <typename MessageType, typename Formatter,
          typename ConsoleMutex = detail::thread::console_mutex>
class color_console_module_base : public module_interface<MessageType>
{
    using base_type = module_interface<MessageType>;

public:
    using message_type = typename base_type::message_type;
    using value_type = typename message_type::value_type;
    using traits_type = typename message_type::traits_type;
    using string_type = typename message_type::string_type;
    using string_view_type = typename message_type::string_view_type;

    using ostream_type = std::basic_ostream<value_type, traits_type>;
    using formatter_type = Formatter;

    using color_attribute_type = color_attribute;
    using mutex_type = typename ConsoleMutex::mutex_type;

    static_assert(
        std::is_same<
            typename decltype(std::function{
                std::declval<formatter_type>()})::result_type,
            std::vector<color_message<value_type, traits_type>>>::value,
        "Formatter output is not "
        "\"std::vector<color_message<value_type, traits_type>>.\"");

    explicit color_console_module_base(
        ostream_type *stream, std::unique_ptr<formatter_type> formatter);
    ~color_console_module_base() override;

    color_console_module_base(const color_console_module_base &other) = delete;
    color_console_module_base(color_console_module_base &&other) noexcept =
        delete;
    auto operator=(const color_console_module_base &other)
        -> color_console_module_base & = delete;
    auto operator=(color_console_module_base &&other) noexcept
        -> color_console_module_base & = delete;

    /**
     * \copydoc module_interface::flush
     */
    void flush() override;

    /**
     * \copydoc module_interface::log_message
     */
    void log_message(std::string_view logger,
                     const message_type &message) override;

    auto ostream() noexcept -> ostream_type &;
    auto ostream() const noexcept -> const ostream_type &;

    [[nodiscard]] auto get_color_mode() const noexcept -> color_mode;
    [[nodiscard]] bool is_parsing_color() const noexcept;
    void set_color_mode(color_mode mode);

protected:
    using string_view_type = std::basic_string_view<value_type, traits_type>;

    virtual void before_log() = 0;
    virtual void after_log() = 0;
    virtual void set_color_attribute(color_attribute_type attribute) = 0;
    virtual void set_color_mode_implement(color_mode mode) = 0;

    [[nodiscard]] bool is_color_parse_enable() const noexcept;
    void set_color_parse(bool enable) noexcept;

    void log_to_stream(string_view_type value);

private:
    template <typename MutexType>
    using lock_type = std::scoped_lock<MutexType>;

    ostream_type *ostream_;
    std::unique_ptr<formatter_type> formatter_;

    mutex_type &mutex_;
    color_mode color_mode_{color_mode::off};
    bool is_color_parse_enable_{false};
};

template <typename MessageType, typename Formatter, typename ConsoleMutex>
color_console_module_base<MessageType, Formatter, ConsoleMutex>::
    color_console_module_base(ostream_type *stream,
                              std::unique_ptr<formatter_type> formatter)
    : ostream_{stream},
      formatter_{std::move(formatter)}, mutex_{ConsoleMutex::mutex()}
{
}

template <typename MessageType, typename Formatter, typename ConsoleMutex>
inline color_console_module_base<MessageType, Formatter,
                                 ConsoleMutex>::~color_console_module_base() =
    default;

template <typename MessageType, typename Formatter, typename ConsoleMutex>
void color_console_module_base<MessageType, Formatter, ConsoleMutex>::flush()
{
    assert(ostream_);
    assert(ostream_->good());

    ostream_->flush();
}

template <typename MessageType, typename Formatter, typename ConsoleMutex>
bool color_console_module_base<MessageType, Formatter,
                               ConsoleMutex>::is_color_parse_enable()
    const noexcept
{
    return is_color_parse_enable_;
}

template <typename MessageType, typename Formatter, typename ConsoleMutex>
auto color_console_module_base<MessageType, Formatter,
                               ConsoleMutex>::get_color_mode() const noexcept
    -> color_mode
{
    return color_mode_;
}

template <typename MessageType, typename Formatter, typename ConsoleMutex>
void color_console_module_base<MessageType, Formatter, ConsoleMutex>::
    log_message(std::string_view logger, const message_type &message)
{
    lock_type<mutex_type> lock{mutex_};

    const auto formatted_messages{(*formatter_)(logger, message)};

    if (is_color_parse_enable_)
    {
        before_log();
    }

    for (const auto &formatted_message : formatted_messages)
    {
        if (is_color_parse_enable_)
        {
            set_color_attribute(formatted_message.color);
        }

        log_to_stream(formatted_message.message);
    }

    if (is_color_parse_enable_)
    {
        after_log();
    }
}

template <typename MessageType, typename Formatter, typename ConsoleMutex>
void color_console_module_base<
    MessageType, Formatter, ConsoleMutex>::log_to_stream(string_view_type value)
{
    assert(ostream_);
    assert(ostream_->good());

    ostream_->write(value.data(), static_cast<std::streamsize>(value.size()));
}

template <typename MessageType, typename Formatter, typename ConsoleMutex>
auto color_console_module_base<MessageType, Formatter,
                               ConsoleMutex>::ostream() noexcept
    -> ostream_type &
{
    return *ostream_;
}

template <typename MessageType, typename Formatter, typename ConsoleMutex>
auto color_console_module_base<MessageType, Formatter, ConsoleMutex>::ostream()
    const noexcept -> const ostream_type &
{
    return *ostream_;
}

template <typename MessageType, typename Formatter, typename ConsoleMutex>
void color_console_module_base<
    MessageType, Formatter, ConsoleMutex>::set_color_parse(bool enable) noexcept
{
    is_color_parse_enable_ = enable;
}

template <typename MessageType, typename Formatter, typename ConsoleMutex>
void color_console_module_base<MessageType, Formatter,
                               ConsoleMutex>::set_color_mode(color_mode mode)
{
    color_mode_ = mode;
    set_color_mode_implement(mode);
}

} // namespace logency::sink_module

#endif // LOGENCY_INCLUDE_LOGENCY_SINK_MODULE_COLOR_CONSOLE_MODULE_BASE_HPP_
