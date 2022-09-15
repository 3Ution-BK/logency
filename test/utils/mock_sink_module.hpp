#ifndef LOGENCY_TEST_UTILS_MOCK_SINK_MODULE_HPP_
#define LOGENCY_TEST_UTILS_MOCK_SINK_MODULE_HPP_

#include "logency/sink_module/module_interface.hpp"
#include <cstddef>

namespace logency::unit_test::utils
{

template <typename MessageType>
class mock_sink_module
    : public logency::sink_module::module_interface<MessageType>
{
public:
    using message_type = MessageType;
    using string_view_type = typename message_type::string_view_type;

    explicit mock_sink_module() noexcept = default;
    template <typename T>
    explicit mock_sink_module(T /* not used */) noexcept
        : mock_sink_module<MessageType>{}
    {
    }
    mock_sink_module(const mock_sink_module &other) = default;
    mock_sink_module(mock_sink_module &&other) noexcept = default;
    auto operator=(const mock_sink_module &other)
        -> mock_sink_module & = default;
    auto operator=(mock_sink_module &&other) noexcept
        -> mock_sink_module & = default;
    ~mock_sink_module() = default;

    void flush() override { ++flush_counter_; }
    void log_message(string_view_type /*logger*/,
                     const message_type & /*message*/) override
    {
        ++log_counter_;
    }

    [[nodiscard]] int flush_counter() const noexcept { return flush_counter_; }
    [[nodiscard]] int log_counter() const noexcept { return log_counter_; }

private:
    int flush_counter_{0};
    int log_counter_{0};
};

} // namespace logency::unit_test::utils

#endif // LOGENCY_TEST_UTILS_MOCK_SINK_MODULE_HPP_
