#include "logency/manager.hpp"
#include "logency/message/log_level.hpp"
#include "logency/message/stream_message.hpp"
#include "logency/sink_module/console_module.hpp"

#include <exception>
#include <iostream>
#include <string>

using example_message = logency::message::stream_message<char>;
using example_formatter = logency::message::stream_message_formatter<char>;
using example_manager = logency::manager<example_message>;
using sink_module =
    logency::sink_module::console_module<example_message, example_formatter>;

void level_flush();
void logger_flush();

int main()
{
    try
    {
        level_flush();
        logger_flush();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error occur: " << e.what();
        return 1;
    }

    return 0;
}

void level_flush()
{
    example_manager manager{};

    auto flush_logger{manager.new_logger("level flush")};

    auto sink{manager.new_sink(
        "picky sink", std::make_unique<sink_module>(
                          &std::cout, std::make_unique<example_formatter>()))};

    flush_logger->add_sink(sink);

    sink->set_filter(
        [&](std::string_view /* unused */, const example_message &message)
        { return message.level >= logency::log_level::info; });

    flush_logger->log(logency::log_level::debug, "will not flush");
    flush_logger->log(logency::log_level::info, "will flush");
}

void logger_flush()
{
    example_manager manager{};

    auto not_flush_logger{manager.new_logger("will not flush")};
    auto flush_logger{manager.new_logger("will flush")};

    auto sink{manager.new_sink(
        "picky sink", std::make_unique<sink_module>(
                          &std::cout, std::make_unique<example_formatter>()))};

    flush_logger->add_sink(sink);
    not_flush_logger->add_sink(sink);

    sink->set_filter(
        [&](std::string_view logger, const example_message & /* unused */)
        { return logger.compare("will flush"); });

    not_flush_logger->log(logency::log_level::info, "will not flush");
    flush_logger->log(logency::log_level::info, "will flush");
}
