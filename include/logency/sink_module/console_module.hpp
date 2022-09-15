#ifndef LOGENCY_INCLUDE_LOGENCY_SINK_MODULE_CONSOLE_MODULE_HPP_
#define LOGENCY_INCLUDE_LOGENCY_SINK_MODULE_CONSOLE_MODULE_HPP_

#include "color_output.hpp"
#include "logency/detail/thread/console_mutex.hpp"
#include "module_interface.hpp"

#include <cassert>

#include <functional>
#include <ostream>

namespace logency::sink_module
{

template <typename MessageType, typename Formatter,
          typename ConsoleMutex = detail::thread::console_mutex>
class console_module : public module_interface<MessageType>
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
        std::is_convertible<typename decltype(std::function{
                                std::declval<formatter_type>()})::result_type,
                            string_type>::value,
        "Formatter output cannot transfer input message to \"string_type\".");

    explicit console_module(ostream_type *stream,
                            std::unique_ptr<formatter_type> formatter);
    ~console_module() override;

    console_module(const console_module &other) = delete;
    console_module(console_module &&other) noexcept = delete;
    auto operator=(const console_module &other) -> console_module & = delete;
    auto operator=(console_module &&other) noexcept
        -> console_module & = delete;

    /**
     * \copydoc module_interface::flush
     */
    void flush() override;

    /**
     * \copydoc module_interface::log_message
     */
    void log_message(string_view_type logger,
                     const message_type &message) override;

    auto ostream() noexcept -> ostream_type &;
    auto ostream() const noexcept -> const ostream_type &;

protected:
    void log_to_stream(string_view_type value);

private:
    template <typename Mutex>
    using lock_type = std::scoped_lock<Mutex>;

    ostream_type *ostream_;
    std::unique_ptr<formatter_type> formatter_;

    mutex_type &mutex_;
    bool is_color_parse_enable_{false};
};

template <typename MessageType, typename Formatter, typename ConsoleMutex>
console_module<MessageType, Formatter, ConsoleMutex>::console_module(
    ostream_type *stream, std::unique_ptr<formatter_type> formatter)
    : ostream_{stream},
      formatter_{std::move(formatter)}, mutex_{ConsoleMutex::mutex()}
{
}

template <typename MessageType, typename Formatter, typename ConsoleMutex>
inline console_module<MessageType, Formatter, ConsoleMutex>::~console_module() =
    default;

template <typename MessageType, typename Formatter, typename ConsoleMutex>
void console_module<MessageType, Formatter, ConsoleMutex>::flush()
{
    assert(ostream_);
    assert(ostream_->good());

    {
        lock_type<mutex_type> lock{mutex_};
        ostream_->flush();
    }
}

template <typename MessageType, typename Formatter, typename ConsoleMutex>
void console_module<MessageType, Formatter, ConsoleMutex>::log_message(
    string_view_type logger, const message_type &message)
{
    const auto formatted_message{(*formatter_)(logger, message)};

    log_to_stream(formatted_message);
}

template <typename MessageType, typename Formatter, typename ConsoleMutex>
void console_module<MessageType, Formatter, ConsoleMutex>::log_to_stream(
    string_view_type value)
{
    assert(ostream_);
    assert(ostream_->good());

    {
        lock_type<mutex_type> lock{mutex_};
        ostream_->write(value.data(),
                        static_cast<std::streamsize>(value.size()));
    }
}

template <typename MessageType, typename Formatter, typename ConsoleMutex>
auto console_module<MessageType, Formatter, ConsoleMutex>::ostream() noexcept
    -> ostream_type &
{
    return *ostream_;
}

template <typename MessageType, typename Formatter, typename ConsoleMutex>
auto console_module<MessageType, Formatter, ConsoleMutex>::ostream()
    const noexcept -> const ostream_type &
{
    return *ostream_;
}

} // namespace logency::sink_module

#endif // LOGENCY_INCLUDE_LOGENCY_SINK_MODULE_CONSOLE_MODULE_HPP_
