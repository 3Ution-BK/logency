#include "logency/manager.hpp"
#include "logency/sink_module/console_module.hpp"

#include <exception>
#include <iostream>
#include <string>

struct empty
{
    using value_type = char;
    using traits_type = std::char_traits<char>;
    using string_type = std::string;
    using string_view_type = std::string_view;

    explicit empty(const std::string & /*unused*/) {}
};

struct throw_in_logging
{
    using value_type = char;
    using traits_type = std::char_traits<char>;
    using string_type = std::string;
    using string_view_type = std::string_view;

    explicit throw_in_logging(const std::string & /*unused*/)
    {
        throw std::runtime_error("throw in logging");
    }
};

template <typename T>
class throw_in_formatting
{
public:
    auto operator()(std::string_view /*unused*/, const T & /*unused*/) const
        -> std::string
    {
        throw std::runtime_error("throw in formatting");
    }
};

void logger_error_handler_example();
void manager_error_handler_example();

int main()
{
    try
    {
        logger_error_handler_example();
        manager_error_handler_example();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error occur: " << e.what();
        return 1;
    }

    return 0;
}

void logger_error_handler_example()
{
    using example_message = throw_in_logging;
    using example_formatter = throw_in_formatting<example_message>;
    using example_manager = logency::manager<example_message>;
    using sink_module = logency::sink_module::console_module<example_message,
                                                             example_formatter>;

    example_manager manager{};

    auto logger{manager.new_logger("logger - example handler")};
    auto sink{manager.new_sink(
        "sink", std::make_unique<sink_module>(
                    &std::cout, std::make_unique<example_formatter>()))};

    logger->add_sink(sink);

    logger->set_error_handler(
        [](const std::exception &exception)
        { std::cout << "Logger error: " << exception.what() << std::endl; });

    logger->log("will not appear");
}

void manager_error_handler_example()
{
    using example_message = empty;
    using example_formatter = throw_in_formatting<example_message>;
    using example_manager = logency::manager<example_message>;
    using sink_module = logency::sink_module::console_module<example_message,
                                                             example_formatter>;

    example_manager manager{};

    auto logger{manager.new_logger("sink - example handler")};
    auto sink{manager.new_sink(
        "sink", std::make_unique<sink_module>(
                    &std::cout, std::make_unique<example_formatter>()))};

    logger->add_sink(sink);

    manager.set_error_handler(
        [](const std::exception &exception)
        { std::cout << "Manager error: " << exception.what() << std::endl; });

    logger->log("will not appear");
}
