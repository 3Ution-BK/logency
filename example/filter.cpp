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

void logger_example();
void level_flush();

int main()
{
    try
    {
        logger_example();
        level_flush();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error occur: " << e.what();
        return 1;
    }

    return 0;
}

void logger_example()
{
    example_manager manager{};

    auto logger{manager.new_logger("logger - filter")};
    auto sink{manager.new_sink(
        "sink", std::make_unique<sink_module>(
                    &std::cout, std::make_unique<example_formatter>()))};

    logger->add_sink(sink);

    logger->set_filter(
        [](std::string_view /*unused*/, const example_message &message)
        { return message.level >= logency::log_level::warning; });

    logger->log(logency::log_level::trace, "will not appear in sink");
    logger->log(logency::log_level::debug, "will not appear in sink");
    logger->log(logency::log_level::info, "will not appear in sink");
    logger->log(logency::log_level::warning, "will appear in sink");
    logger->log(logency::log_level::error, "will appear in sink");
    logger->log(logency::log_level::critical, "will appear in sink");
}

void level_flush()
{
    example_manager manager{};

    auto filter_in{manager.new_logger("sink - filter in")};
    auto filter_out{manager.new_logger("sink - filter out")};

    auto sink{manager.new_sink(
        "picky sink", std::make_unique<sink_module>(
                          &std::cout, std::make_unique<example_formatter>()))};

    filter_in->add_sink(sink);
    filter_out->add_sink(sink);

    sink->set_filter(
        [&](std::string_view logger, const example_message & /*unused*/)
        { return logger.compare(filter_in->name()) == 0; });

    filter_in->log(logency::log_level::info, "will appear in sink");
    filter_out->log(logency::log_level::info, "will not appear in sink");
}
