#ifndef LOGENCY_INCLUDE_LOGENCY_SINK_MODULE_NULL_MODULE_HPP_
#define LOGENCY_INCLUDE_LOGENCY_SINK_MODULE_NULL_MODULE_HPP_

#include "module_interface.hpp"

namespace logency::sink_module
{

template <typename MessageType>
class null_module : public module_interface<MessageType>
{
    using base_type = module_interface<MessageType>;

public:
    using message_type = typename base_type::message_type;

    explicit null_module();

    void flush() override;
    void log_message(std::string_view logger,
                     const message_type &message) override;
};

template <typename MessageType>
inline null_module<MessageType>::null_module() = default;

template <typename MessageType>
inline void null_module<MessageType>::flush()
{
}

template <typename MessageType>
inline void null_module<MessageType>::log_message(
    [[maybe_unused]] std::string_view logger,
    [[maybe_unused]] const message_type &message)
{
}

} // namespace logency::sink_module

#endif // LOGENCY_INCLUDE_LOGENCY_SINK_MODULE_NULL_MODULE_HPP_
