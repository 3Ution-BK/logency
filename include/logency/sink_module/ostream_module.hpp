#ifndef LOGENCY_INCLUDE_LOGENCY_SINK_MODULE_OSTREAM_MODULE_HPP_
#define LOGENCY_INCLUDE_LOGENCY_SINK_MODULE_OSTREAM_MODULE_HPP_

#include "logency/core/exception.hpp"
#include "module_interface.hpp"

#include <cassert>

#include <functional>
#include <ostream>
#include <type_traits>

namespace logency::sink_module
{

/**
 * \brief This class represent the ostream sink.
 *
 * Typical ostream sink. Assign a ostream and formatter to this sink and it is
 * ready to go.
 *
 * \tparam MessageType MessageType type.
 * \tparam Formatter Formatter type.
 */
template <typename MessageType, typename Formatter>
class ostream_module : public module_interface<MessageType>
{
    using base_type = module_interface<MessageType>;

public:
    using message_type = typename base_type::message_type;
    using value_type = typename message_type::value_type;
    using traits_type = typename message_type::traits_type;
    using string_type = typename message_type::string_type;
    using string_view_type = typename message_type::string_view_type;

    using formatter_type = Formatter;

    using ostream_type = std::basic_ostream<value_type, traits_type>;
    using formatter_type = Formatter;

    static_assert(
        std::is_convertible<typename decltype(std::function{
                                std::declval<formatter_type>()})::result_type,
                            string_type>::value,
        "Formatter output cannot transfer input message to \"string_type\".");

    /**
     * \brief Initializes a new instance of the ostream sink class with
     * specified \a ostream and \a formatter.
     *
     * \param stream Specified ostream. Any logged message will print out
     * through this stream.
     * \param formatter Specified formatter.
     */
    explicit ostream_module(ostream_type *stream,
                            std::unique_ptr<formatter_type> formatter);

    /**
     * \brief Destroy the instance of the ostream sink class.
     */
    ~ostream_module() override;

    ostream_module(const ostream_module &other) = delete;
    ostream_module(ostream_module &&other) noexcept = delete;
    auto operator=(const ostream_module &other) -> ostream_module & = delete;
    auto operator=(ostream_module &&other) noexcept
        -> ostream_module & = delete;

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
    void log_to_stream(string_view_type value);

private:
    static auto throw_if_nullptr(ostream_type *stream) -> ostream_type *;

    ostream_type *ostream_; //!< ostream pointer to prevent slicing.
     std::unique_ptr<formatter_type> formatter_;
};

template <typename MessageType, typename Formatter>
ostream_module<MessageType, Formatter>::ostream_module(
    ostream_type *stream, std::unique_ptr<formatter_type> formatter)
    : ostream_{throw_if_nullptr(stream)}, formatter_{std::move(formatter)}
{
}

template <typename MessageType, typename Formatter>
ostream_module<MessageType, Formatter>::~ostream_module() = default;

template <typename MessageType, typename Formatter>
void ostream_module<MessageType, Formatter>::flush()
{
    assert(ostream_);
    assert(ostream_->good());

    ostream_->flush();
}

template <typename MessageType, typename Formatter>
void ostream_module<MessageType, Formatter>::log_message(
    string_view_type logger, const message_type &message)
{
    auto formatted_message{(*formatter_)(logger, message)};

    log_to_stream(formatted_message);
}

template <typename MessageType, typename Formatter>
void ostream_module<MessageType, Formatter>::log_to_stream(
    string_view_type value)
{
    assert(ostream_);
    assert(ostream_->good());

    ostream_->write(value.data(), static_cast<std::streamsize>(value.size()));
}

template <typename MessageType, typename Formatter>
auto ostream_module<MessageType, Formatter>::throw_if_nullptr(
    ostream_type *stream) -> ostream_type *
{
    if (stream)
    {
        return stream;
    }

    throw logency::runtime_error("stream is nullptr.");
}

} // namespace logency::sink_module

#endif // LOGENCY_INCLUDE_LOGENCY_SINK_MODULE_OSTREAM_MODULE_HPP_
