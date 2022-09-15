#include "logency/manager.hpp"
#include "logency/message/stream_message.hpp"
#include "logency/sink_module/color_console_module.hpp"
#include "logency/sink_module/win32_color_console_module.hpp"

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

using example_message = logency::message::stream_message<char>;
using example_formatter =
    logency::message::stream_color_message_formatter<char>;
using example_manager = logency::manager<example_message>;

void logger_example();

void color_sink(example_manager &manager);
void win32_sink(example_manager &manager);

int main()
{
    try
    {
        logger_example();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error occur: " << e.what();
        return 1;
    }

    return 0;
}

void color_sink(example_manager &manager)
{
    using sink_module =
        logency::sink_module::color_console_module<example_message,
                                                   example_formatter>;

    auto logger{manager.new_logger("color console")};
    auto sink{manager.new_sink(
        "color sink", std::make_unique<sink_module>(
                          &std::cout, std::make_unique<example_formatter>()))};

    logger->add_sink(sink);

    logger->log(logency::log_level::trace, "message");
    logger->log(logency::log_level::debug, "message");
    logger->log(logency::log_level::info, "message");
    logger->log(logency::log_level::warning, "message");
    logger->log(logency::log_level::error, "message");
    logger->log(logency::log_level::critical, "message");
}

void logger_example()
{
    example_manager manager{};

    color_sink(manager);
    win32_sink(manager);
}

void win32_sink(example_manager &manager)
{
#if defined(_WIN32)
    using sink_module =
        logency::sink_module::win32_color_console_module<example_message,
                                                         example_formatter>;

    auto logger{manager.new_logger("win32 console")};

    auto sink{manager.new_sink(
        "win32 sink", std::make_unique<sink_module>(
                          &std::cout, std::make_unique<example_formatter>()))};

    logger->add_sink(sink);

    logger->log(logency::log_level::trace, "message");
    logger->log(logency::log_level::debug, "message");
    logger->log(logency::log_level::info, "message");
    logger->log(logency::log_level::warning, "message");
    logger->log(logency::log_level::error, "message");
    logger->log(logency::log_level::critical, "message");
#endif
}
