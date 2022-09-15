#include "logency/manager.hpp"
#include "logency/message/log_level.hpp"
#include "logency/message/stream_message.hpp"
#include "logency/sink_module/basic_file_module.hpp"

#include <exception>
#include <iostream>
#include <string>

using example_message = logency::message::stream_message<char>;
using example_formatter = logency::message::stream_message_formatter<char>;
using example_manager = logency::manager<example_message>;

void logger_example();

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

void logger_example()
{
    using sink_module =
        logency::sink_module::basic_file_module<example_message,
                                                example_formatter>;

    example_manager manager{};

    auto logger{manager.new_logger("logger")};
    auto sink{manager.new_sink(
        "sink",
        std::make_unique<sink_module>("log/basic_file_sink.txt",
                                      logency::file_open_mode::truncate,
                                      std::make_unique<example_formatter>()))};

    logger->add_sink(sink);

    logger->log(logency::log_level::trace, "message");
    logger->log(logency::log_level::debug, "message");
    logger->log(logency::log_level::info, "message");
    logger->log(logency::log_level::warning, "message");
    logger->log(logency::log_level::error, "message");
    logger->log(logency::log_level::critical, "message");
}
