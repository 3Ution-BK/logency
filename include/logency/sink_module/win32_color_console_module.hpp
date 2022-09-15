#ifndef LOGENCY_INCLUDE_LOGENCY_SINK_MODULE_WIN32_COLOR_CONSOLE_MODULE_HPP_
#define LOGENCY_INCLUDE_LOGENCY_SINK_MODULE_WIN32_COLOR_CONSOLE_MODULE_HPP_

#include "color_console_module_base.hpp"
#include "logency/detail/include_windows.hpp"

#include <iostream>

namespace logency::sink_module
{

template <typename MessageType, typename Formatter,
          typename ConsoleMutex = detail::thread::console_mutex>
class win32_color_console_module
    : public color_console_module_base<MessageType, Formatter, ConsoleMutex>
{
    using base_type =
        color_console_module_base<MessageType, Formatter, ConsoleMutex>;

public:
    using message_type = typename base_type::message_type;
    using value_type = typename base_type::value_type;
    using traits_type = typename base_type::traits_type;

    using formatter_type = typename base_type::formatter_type;
    using ostream_type = typename base_type::ostream_type;

    static_assert(std::is_same<ostream_type, std::ostream>::value ||
                      std::is_same<ostream_type, std::wostream>::value,
                  "ostream_type is not supported. "
                  "support type: std::ostream, std::wostream");

    explicit win32_color_console_module(
        ostream_type *stream, std::unique_ptr<formatter_type> formatter,
        color_mode mode = color_mode::automatic);

    ~win32_color_console_module() override;

    win32_color_console_module(const win32_color_console_module &other) =
        delete;
    win32_color_console_module(win32_color_console_module &&other) noexcept =
        delete;
    auto operator=(const win32_color_console_module &other)
        -> win32_color_console_module & = delete;
    auto operator=(win32_color_console_module &&other) noexcept
        -> win32_color_console_module & = delete;

protected:
    using color_attribute_type = typename base_type::color_attribute_type;

    void before_log() override;
    void after_log() override;
    void set_color_attribute(color_attribute_type attribute) override;
    void set_color_mode_implement(color_mode mode) override;

private:
    using handle_type = HANDLE;
    using machine_attribute_type = WORD;

    struct color_map
    {
        machine_attribute_type red;
        machine_attribute_type blue;
        machine_attribute_type green;
        machine_attribute_type intensity;
    };

    static constexpr const color_map foreground_map{
        FOREGROUND_RED, FOREGROUND_BLUE, FOREGROUND_GREEN,
        FOREGROUND_INTENSITY};
    static constexpr const color_map background_map{
        BACKGROUND_RED, BACKGROUND_BLUE, BACKGROUND_GREEN,
        BACKGROUND_INTENSITY};

    static bool detect_handle_can_parse_color(handle_type handle);
    static auto map_color(console_color color, color_map map)
        -> machine_attribute_type;

    [[nodiscard]] bool is_handle_valid() const noexcept;

    void set_machine_attribute(machine_attribute_type attribute);

    auto convert_to_machine_attribute(color_attribute_type color)
        -> machine_attribute_type;

    handle_type handle_;
    machine_attribute_type original_machine_attribute_{0x0};
    machine_attribute_type current_machine_attribute_{0x0};

    bool can_parse_color_;
};

template <typename MessageType, typename Formatter, typename ConsoleMutex>
win32_color_console_module<MessageType, Formatter, ConsoleMutex>::
    win32_color_console_module(ostream_type *stream,
                               std::unique_ptr<formatter_type> formatter,
                               color_mode mode)
    : base_type{stream, std::move(formatter)}, handle_{detail::os::get_handle(
                                                   stream)},
      can_parse_color_{detect_handle_can_parse_color(handle_)}
{
    base_type::set_color_mode(mode);
}

template <typename MessageType, typename Formatter, typename ConsoleMutex>
win32_color_console_module<MessageType, Formatter,
                           ConsoleMutex>::~win32_color_console_module() =
    default;

template <typename MessageType, typename Formatter, typename ConsoleMutex>
void win32_color_console_module<MessageType, Formatter,
                                ConsoleMutex>::after_log()
{
    if (original_machine_attribute_ != current_machine_attribute_)
    {
        set_machine_attribute(original_machine_attribute_);
    }
}

template <typename MessageType, typename Formatter, typename ConsoleMutex>
void win32_color_console_module<MessageType, Formatter,
                                ConsoleMutex>::before_log()
{
    original_machine_attribute_ = 0x0;

    if (is_handle_valid())
    {
        if (CONSOLE_SCREEN_BUFFER_INFO info;
            GetConsoleScreenBufferInfo(handle_, &info))
        {
            original_machine_attribute_ = info.wAttributes;
        }
    }

    current_machine_attribute_ = original_machine_attribute_;
}

template <typename MessageType, typename Formatter, typename ConsoleMutex>
auto win32_color_console_module<MessageType, Formatter, ConsoleMutex>::
    convert_to_machine_attribute(color_attribute_type color)
        -> machine_attribute_type
{
    constexpr const machine_attribute_type foreground_mask{
        // NOLINTNEXTLINE(*-signed-bitwise)
        FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED |
        FOREGROUND_INTENSITY};
    constexpr const machine_attribute_type background_mask{
        // NOLINTNEXTLINE(*-signed-bitwise)
        BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED |
        BACKGROUND_INTENSITY};

    machine_attribute_type result{0x0};

    if (color.foreground != console_color::original)
    {
        // NOLINTNEXTLINE(*-signed-bitwise)
        result |= map_color(color.foreground, foreground_map);
    }
    else
    {
        // NOLINTNEXTLINE(*-signed-bitwise)
        result |= (original_machine_attribute_ & foreground_mask);
    }

    if (color.background != console_color::original)
    {
        // NOLINTNEXTLINE(*-signed-bitwise)
        result |= map_color(color.background, background_map);
    }
    else
    {
        // NOLINTNEXTLINE(*-signed-bitwise)
        result |= (original_machine_attribute_ & background_mask);
    }

    return result;
}

template <typename MessageType, typename Formatter, typename ConsoleMutex>
bool win32_color_console_module<MessageType, Formatter, ConsoleMutex>::
    detect_handle_can_parse_color(handle_type handle)
{
    /**
     * GetConsoleMode will try to get the console mode from the handle. Useful
     * to detect if the handle is attached to the console.
     */
    if (DWORD mode_not_used{0}; GetConsoleMode(handle, &mode_not_used) != 0)
    {
        /**
         * Use `GetConsoleScreenBufferInfo` and `SetConsoleTextAttribute` to
         * check if the console failed to set color.
         *
         * The color should not change after calling this function since the
         * attribute is set by its original one.
         */
        if (CONSOLE_SCREEN_BUFFER_INFO info;
            GetConsoleScreenBufferInfo(handle, &info) != 0)
        {
            if (SetConsoleTextAttribute(handle, info.wAttributes) != 0)
            {
                return true;
            }
        }
    }

    return false;
}

template <typename MessageType, typename Formatter, typename ConsoleMutex>
bool win32_color_console_module<MessageType, Formatter,
                                ConsoleMutex>::is_handle_valid() const noexcept
{
    // NOLINTNEXTLINE(*-no-int-to-ptr, *-pro-type-cstyle-cast)
    return handle_ != nullptr && handle_ != INVALID_HANDLE_VALUE;
}

template <typename MessageType, typename Formatter, typename ConsoleMutex>
auto win32_color_console_module<MessageType, Formatter,
                                ConsoleMutex>::map_color(console_color color,
                                                         color_map map)
    -> machine_attribute_type
{
    machine_attribute_type result{0x0};

    if (static_cast<std::underlying_type<console_color>::type>(
            color & console_color::red) != 0x0)
    {
        result |= map.red;
    }
    if (static_cast<std::underlying_type<console_color>::type>(
            color & console_color::blue) != 0x0)
    {
        result |= map.blue;
    }
    if (static_cast<std::underlying_type<console_color>::type>(
            color & console_color::green) != 0x0)
    {
        result |= map.green;
    }
    if (static_cast<std::underlying_type<console_color>::type>(
            color & console_color::intense) != 0x0)
    {
        result |= map.intensity;
    }

    return result;
}

template <typename MessageType, typename Formatter, typename ConsoleMutex>
void win32_color_console_module<MessageType, Formatter, ConsoleMutex>::
    set_color_attribute(color_attribute_type attribute)
{
    const auto machine_attribute{convert_to_machine_attribute(attribute)};
    set_machine_attribute(machine_attribute);
}

template <typename MessageType, typename Formatter, typename ConsoleMutex>
void win32_color_console_module<MessageType, Formatter, ConsoleMutex>::
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
void win32_color_console_module<MessageType, Formatter, ConsoleMutex>::
    set_machine_attribute(machine_attribute_type attribute)
{
    if (is_handle_valid())
    {
        if (SetConsoleTextAttribute(handle_, attribute) != 0)
        {
            current_machine_attribute_ = attribute;
        }
    }
}

} // namespace logency::sink_module

#endif // LOGENCY_INCLUDE_LOGENCY_SINK_MODULE_WIN32_COLOR_CONSOLE_MODULE_HPP_
