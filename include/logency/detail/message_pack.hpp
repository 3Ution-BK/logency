#ifndef LOGENCY_INCLUDE_LOGENCY_CORE_MESSAGE_PACK_HPP_
#define LOGENCY_INCLUDE_LOGENCY_CORE_MESSAGE_PACK_HPP_

#include <memory>
#include <string>
#include <utility>

#include <chrono>

namespace logency
{

/**
 * \brief this class represent the message_pack_base
 *
 * It is the basic unit for transfer message between each working unit like
 * queue, logger, and sink.
 *
 * \tparam MessageType User message type
 */
template <typename MessageType>
struct message_pack_base
{
    using message_type = MessageType;
    using string_type = typename message_type::string_type;

    explicit message_pack_base(std::shared_ptr<string_type> &&logger,
                               message_type &&message);

    std::shared_ptr<string_type> logger_name;
    message_type message;
};

template <typename MessageType>
inline message_pack_base<MessageType>::message_pack_base(
    std::shared_ptr<string_type> &&logger, message_type &&message)
    : logger_name{std::move(logger)}, message{std::move(message)}
{
}

template <typename MessageType>
using message_pack = std::shared_ptr<message_pack_base<MessageType>>;

template <typename MessageType, typename... Args>
auto make_message_pack(
    std::shared_ptr<typename MessageType::string_type> logger, Args &&...args)
    -> message_pack<MessageType>
{
    return std::make_shared<message_pack_base<MessageType>>(
        std::move(logger), std::forward<Args>(args)...);
}

} // namespace logency

#endif // LOGENCY_INCLUDE_LOGENCY_CORE_MESSAGE_PACK_HPP_
