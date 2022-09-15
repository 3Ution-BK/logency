#ifndef LOGENCY_INCLUDE_LOGENCY_SINK_MODULE_COLOR_CONSOLE_MODULE_HPP_
#define LOGENCY_INCLUDE_LOGENCY_SINK_MODULE_COLOR_CONSOLE_MODULE_HPP_

#include "ansi_color_console_module.hpp"

namespace logency::sink_module
{

template <typename MessageType, typename Formatter,
          typename ConsoleMutex = detail::thread::console_mutex>
using color_console_module =
    ansi_color_console_module<MessageType, Formatter, ConsoleMutex>;

} // namespace logency::sink_module

#endif // LOGENCY_INCLUDE_LOGENCY_SINK_MODULE_COLOR_CONSOLE_MODULE_HPP_
