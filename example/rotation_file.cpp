#include "logency/manager.hpp"
#include "logency/message/stream_message.hpp"
#include "logency/sink_module/rotation_file_module.hpp"

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
        logency::sink_module::rotation_file_module<example_message,
                                                   example_formatter>;

    example_manager manager{};

    constexpr const auto one_kb{1 * 1024 * 1024};
    constexpr const auto archive_size{2};
    constexpr const logency::sink_module::rotation_file::rotate_info
        rotate_info{one_kb, archive_size};
    constexpr const auto mode{
        logency::sink_module::rotation_file::construct_mode::create_new_file};

    auto logger{manager.new_logger("logger")};
    auto sink{manager.new_sink(
        "sink", std::make_unique<sink_module>(
                    "log/rotation_file_sink.txt", rotate_info, mode,
                    std::make_unique<example_formatter>()))};

    logger->add_sink(sink);

    logger->log(logency::log_level::trace, "message");
    logger->log(logency::log_level::debug, "message");
    logger->log(logency::log_level::info, "message");
    logger->log(logency::log_level::warning, "message");
    logger->log(logency::log_level::error, "message");
    logger->log(logency::log_level::critical, "message");
}
