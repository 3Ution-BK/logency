#ifndef LOGENCY_INCLUDE_LOGENCY_SINK_MODULE_BASE_MODULE_INTERFACE_HPP_
#define LOGENCY_INCLUDE_LOGENCY_SINK_MODULE_BASE_MODULE_INTERFACE_HPP_

#include <string>
#include <vector>

namespace logency::sink_module
{

/**
 * \brief This class represent the base of the sink.
 *
 * User can derived this class to implement their own sink.
 *
 * \tparam LevelType Comparable level type.
 * \tparam MessageType MessageType type.
 */
template <typename MessageType>
class module_interface
{
public:
    using message_type = MessageType;

    using string_view_type = typename message_type::string_view_type;

    virtual ~module_interface() = default;

    /**
     * \brief flush the sink module.
     */
    virtual void flush() = 0;

    /**
     * \brief log the specified \a mesaage into the module with \a logger name
     * and its \a level.
     *
     * \param name Specified logger name.
     * \param level Specified level.
     * \param message Specified message.
     */
    virtual void log_message(string_view_type logger,
                             const message_type &message) = 0;
};

} // namespace logency::sink_module

#endif // LOGENCY_INCLUDE_LOGENCY_SINK_MODULE_BASE_MODULE_INTERFACE_HPP_
