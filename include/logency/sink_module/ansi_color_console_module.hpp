#ifndef LOGENCY_INCLUDE_LOGENCY_SINK_MODULE_ANSI_COLOR_CONSOLE_MODULE_HPP_
#define LOGENCY_INCLUDE_LOGENCY_SINK_MODULE_ANSI_COLOR_CONSOLE_MODULE_HPP_

#include "color_console_module_base.hpp"

#if defined(_WIN32)
    #include "logency/detail/include_windows.hpp"
#else
    #include "logency/detail/include_linux.hpp"
#endif

#include <array>
#include <iostream>

namespace logency::sink_module
{

template <typename MessageType, typename Formatter,
          typename ConsoleMutex = detail::thread::console_mutex>
class ansi_color_console_module
    : public color_console_module_base<MessageType, Formatter, ConsoleMutex>
{
    using base_type =
        color_console_module_base<MessageType, Formatter, ConsoleMutex>;

public:
    using message_type = typename base_type::message_type;
    using value_type = typename base_type::value_type;
    using traits_type = typename base_type::traits_type;
    using ostream_type = typename base_type::ostream_type;

    using formatter_type = typename base_type::formatter_type;

    explicit ansi_color_console_module(
        ostream_type *stream, std::unique_ptr<formatter_type> formatter,
        color_mode mode = color_mode::automatic);
    ~ansi_color_console_module() override;

    ansi_color_console_module(const ansi_color_console_module &other) = delete;
    ansi_color_console_module(ansi_color_console_module &&other) noexcept =
        delete;
    auto operator=(const ansi_color_console_module &other)
        -> ansi_color_console_module & = delete;
    auto operator=(ansi_color_console_module &&other) noexcept
        -> ansi_color_console_module & = delete;

protected:
    using color_attribute_type = typename base_type::color_attribute_type;
    using machine_attribute_type = typename base_type::string_view_type;

    void before_log() override;
    void after_log() override;
    void set_color_attribute(color_attribute_type attribute) override;
    void set_color_mode_implement(color_mode mode) override;

private:
    struct code
    {
        static constexpr const machine_attribute_type reset{"\x1b[0m"};

        struct foreground
        {
            static constexpr const machine_attribute_type black{"\x1b[30m"};
            static constexpr const machine_attribute_type red{"\x1b[31m"};
            static constexpr const machine_attribute_type green{"\x1b[32m"};
            static constexpr const machine_attribute_type yellow{"\x1b[33m"};
            static constexpr const machine_attribute_type blue{"\x1b[34m"};
            static constexpr const machine_attribute_type magenta{"\x1b[35m"};
            static constexpr const machine_attribute_type cyan{"\x1b[36m"};
            static constexpr const machine_attribute_type white{"\x1b[37m"};

            static constexpr const machine_attribute_type intense_black{
                "\x1b[90m"};
            static constexpr const machine_attribute_type intense_red{
                "\x1b[91m"};
            static constexpr const machine_attribute_type intense_green{
                "\x1b[92m"};
            static constexpr const machine_attribute_type intense_yellow{
                "\x1b[93m"};
            static constexpr const machine_attribute_type intense_blue{
                "\x1b[94m"};
            static constexpr const machine_attribute_type intense_magenta{
                "\x1b[95m"};
            static constexpr const machine_attribute_type intense_cyan{
                "\x1b[96m"};
            static constexpr const machine_attribute_type intense_white{
                "\x1b[97m"};

            static constexpr const std::array<machine_attribute_type, 16> map{{
                black,
                blue,
                green,
                cyan,
                red,
                magenta,
                yellow,
                white,
                intense_black,
                intense_blue,
                intense_green,
                intense_cyan,
                intense_red,
                intense_magenta,
                intense_yellow,
                intense_white,
            }};
        };

        struct background
        {
            static constexpr const machine_attribute_type black{"\x1b[40m"};
            static constexpr const machine_attribute_type red{"\x1b[41m"};
            static constexpr const machine_attribute_type green{"\x1b[42m"};
            static constexpr const machine_attribute_type yellow{"\x1b[43m"};
            static constexpr const machine_attribute_type blue{"\x1b[44m"};
            static constexpr const machine_attribute_type magenta{"\x1b[45m"};
            static constexpr const machine_attribute_type cyan{"\x1b[46m"};
            static constexpr const machine_attribute_type white{"\x1b[47m"};

            static constexpr const machine_attribute_type intense_black{
                "\x1b[100m"};
            static constexpr const machine_attribute_type intense_red{
                "\x1b[101m"};
            static constexpr const machine_attribute_type intense_green{
                "\x1b[102m"};
            static constexpr const machine_attribute_type intense_yellow{
                "\x1b[103m"};
            static constexpr const machine_attribute_type intense_blue{
                "\x1b[104m"};
            static constexpr const machine_attribute_type intense_magenta{
                "\x1b[105m"};
            static constexpr const machine_attribute_type intense_cyan{
                "\x1b[106m"};
            static constexpr const machine_attribute_type intense_white{
                "\x1b[107m"};

            static constexpr const std::array<machine_attribute_type, 16> map{{
                black,
                blue,
                green,
                cyan,
                red,
                magenta,
                yellow,
                white,
                intense_black,
                intense_blue,
                intense_green,
                intense_cyan,
                intense_red,
                intense_magenta,
                intense_yellow,
                intense_white,
            }};
        };
    };

    void instantiate();
    void tidy();

    void set_machine_attribute(machine_attribute_type attribute);

    bool should_reset(color_attribute_type attribute);

#if defined(_WIN32)

    using console_mode_type = DWORD;
    using handle_type = HANDLE;

    bool set_virtual_terminal_processing(handle_type handle);

    template <typename T>
    auto replace_bit(T replace, T with, T mask) -> T;

    // NOLINTNEXTLINE(*-no-int-to-ptr, *-pro-type-cstyle-cast)
    handle_type handle_{INVALID_HANDLE_VALUE};
    console_mode_type original_mode_{0};

#endif

    color_attribute_type current_machine_attribute_{};

    bool can_parse_color_{false};
};

template <typename MessageType, typename Formatter, typename ConsoleMutex>
ansi_color_console_module<MessageType, Formatter, ConsoleMutex>::
    ansi_color_console_module(ostream_type *stream,
                              std::unique_ptr<formatter_type> formatter,
                              color_mode mode)
    : base_type{stream, std::move(formatter)}
{
    instantiate();

    base_type::set_color_mode(mode);
}

template <typename MessageType, typename Formatter, typename ConsoleMutex>
ansi_color_console_module<MessageType, Formatter,
                          ConsoleMutex>::~ansi_color_console_module()
{
    tidy();
}

template <typename MessageType, typename Formatter, typename ConsoleMutex>
void ansi_color_console_module<MessageType, Formatter,
                               ConsoleMutex>::after_log()
{
    set_color_attribute(color_attribute_type{});
}

template <typename MessageType, typename Formatter, typename ConsoleMutex>
void ansi_color_console_module<MessageType, Formatter,
                               ConsoleMutex>::before_log()
{
}

template <typename MessageType, typename Formatter, typename ConsoleMutex>
void ansi_color_console_module<MessageType, Formatter,
                               ConsoleMutex>::instantiate()
{
#if defined(_WIN32)
    handle_ = detail::os::get_handle(&base_type::ostream());

    if (set_virtual_terminal_processing(handle_))
    {
        can_parse_color_ = true;
    }
#else
    can_parse_color_ = detail::os::is_terminal(&base_type::ostream());
#endif
}

template <typename MessageType, typename Formatter, typename ConsoleMutex>
void ansi_color_console_module<MessageType, Formatter, ConsoleMutex>::
    set_color_attribute(color_attribute_type attribute)
{
    if (should_reset(attribute))
    {
        current_machine_attribute_ = color_attribute_type{};
        set_machine_attribute(code::reset);
    }

    if (current_machine_attribute_.foreground != attribute.foreground)
    {
        assert(attribute.foreground != console_color::original);

        current_machine_attribute_.foreground = attribute.foreground;
        set_machine_attribute(
            code::foreground::map[static_cast<size_t>(attribute.foreground)]);
    }

    if (current_machine_attribute_.background != attribute.background)
    {
        assert(attribute.background != console_color::original);

        current_machine_attribute_.background = attribute.background;
        set_machine_attribute(
            code::background::map[static_cast<size_t>(attribute.background)]);
    }
}

template <typename MessageType, typename Formatter, typename ConsoleMutex>
void ansi_color_console_module<MessageType, Formatter, ConsoleMutex>::
    set_color_mode_implement(color_mode mode)
{
    switch (mode)
    {
    case color_mode::automatic:
        base_type::set_color_parse(can_parse_color_);
        return;
    case color_mode::on:
        base_type::set_color_parse(true);
        return;
    default:
        base_type::set_color_parse(false);
        return;
    }
}

template <typename MessageType, typename Formatter, typename ConsoleMutex>
void ansi_color_console_module<MessageType, Formatter, ConsoleMutex>::
    set_machine_attribute(machine_attribute_type attribute)
{
    base_type::log_to_stream(attribute);
}

template <typename MessageType, typename Formatter, typename ConsoleMutex>
bool ansi_color_console_module<MessageType, Formatter, ConsoleMutex>::
    should_reset(color_attribute_type attribute)
{
    return (current_machine_attribute_.foreground != console_color::original &&
            attribute.foreground == console_color::original) ||
           (current_machine_attribute_.background != console_color::original &&
            attribute.background == console_color::original);
}

template <typename MessageType, typename Formatter, typename ConsoleMutex>
void ansi_color_console_module<MessageType, Formatter, ConsoleMutex>::tidy()
{
#if defined(_WIN32)
    if (!can_parse_color_)
    {
        return;
    }

    console_mode_type mode{0};
    if (GetConsoleMode(handle_, &mode) == 0)
    {
        return;
    }

    mode = replace_bit<console_mode_type>(mode, original_mode_,
                                          ENABLE_VIRTUAL_TERMINAL_PROCESSING);

    std::ignore = SetConsoleMode(handle_, mode);
#endif
}

#if defined(_WIN32)

template <typename MessageType, typename Formatter, typename ConsoleMutex>
template <typename T>
auto ansi_color_console_module<MessageType, Formatter,
                               ConsoleMutex>::replace_bit(T replace, T with,
                                                          T mask) -> T
{
    return (replace & ~mask) | (with & mask);
}

template <typename MessageType, typename Formatter, typename ConsoleMutex>
bool ansi_color_console_module<MessageType, Formatter, ConsoleMutex>::
    set_virtual_terminal_processing(handle_type handle)
{
    // NOLINTNEXTLINE(*-no-int-to-ptr, *-pro-type-cstyle-cast)
    if (handle == INVALID_HANDLE_VALUE || handle == nullptr)
    {
        return false;
    }

    if (GetConsoleMode(handle, &original_mode_) == 0)
    {
        return false;
    }

    // NOLINTNEXTLINE(*-signed-bitwise)
    auto mode{original_mode_ | ENABLE_VIRTUAL_TERMINAL_PROCESSING};
    return (SetConsoleMode(handle, mode) != 0);
}

#endif

} // namespace logency::sink_module

#endif // LOGENCY_INCLUDE_LOGENCY_SINK_MODULE_ANSI_COLOR_CONSOLE_MODULE_HPP_
