#include "logency/manager.hpp"
#include "logency/sink_module/console_module.hpp"

#include <iostream>
#include <sstream>
#include <string>

struct message
{
    // Define proper type in your structure for manager
    using value_type = char;
    using string_type = std::string;
    using string_view_type = std::string_view;

    // Define additional proper type in your structure for sink_module
    using traits_type = std::char_traits<char>;

    // Write down your content
    std::string content;
    int i_need_it;
    float i_need_it_too;
};

class formatter
{
public:
    // Define what content you wish to print
    auto operator()(std::string_view logger, const message &message) const
        -> std::string
    {
        std::stringstream stream;
        stream << logger << ": " << message.content << " {" << message.i_need_it
               << ", " << message.i_need_it_too << "}\n";

        return stream.str();
    }
};

int main()
{
    using sink_module =
        logency::sink_module::console_module<message, formatter>;

    logency::manager<message> manager;

    auto logger{manager.new_logger("this is logger")};
    auto sink{manager.new_sink("this is sink",
                               std::make_unique<sink_module>(
                                   &std::cout, std::make_unique<formatter>()))};

    logger->add_sink(sink);

    logger->log("my content", 0, 1.0F);
    logger->log("another content", -1, 0.0F);

    return 0;
}
