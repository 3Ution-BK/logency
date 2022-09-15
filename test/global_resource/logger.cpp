#include "logger.hpp"

#include "dispatcher.hpp"

#include <memory>

namespace logency::unit_test::global_resource::logger
{

auto normal() -> std::shared_ptr<logency::logger<utils::message<char>>>
{
    using message_type = utils::message<char>;
    using logger_type = logency::logger<message_type>;

    return std::make_shared<logger_type>("logger", dispatcher::normal());
}

} // namespace logency::unit_test::global_resource::logger
