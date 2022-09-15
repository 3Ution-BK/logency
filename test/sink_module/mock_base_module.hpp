
#include "logency/sink_module/module_interface.hpp"

#include "google_test.hpp"

namespace logency::unit_test::sink_module
{

template <typename MessageType>
class mock_base_module
    : public logency::sink_module::module_interface<MessageType>
{
public:
    using module_interface =
        logency::sink_module::module_interface<MessageType>;
    using message_type = typename module_interface::message_type;

    explicit mock_base_module();
    ~mock_base_module();

    mock_base_module(const mock_base_module &other);
    mock_base_module(mock_base_module &&other) noexcept;
    mock_base_module &operator=(const mock_base_module &other);
    mock_base_module &operator=(mock_base_module &&other) noexcept;

    MOCK_METHOD(void, flush, (), (override));
    MOCK_METHOD(void, log_message, (const message_type &message), (override));

    int get_foo() const noexcept;
    int set_foo(int foo) noexcept;

private:
    int foo_{0};
};

template <typename MessageType>
inline mock_base_module<MessageType>::mock_base_module() : module_interface()
{
}

template <typename MessageType>
inline mock_base_module<MessageType>::~mock_base_module() = default;

template <typename MessageType>
inline int mock_base_module<MessageType>::get_foo() const noexcept
{
    return foo_;
}

template <typename MessageType>
inline int mock_base_module<MessageType>::set_foo(int foo) noexcept
{
    foo_ = foo;
    return foo_;
}

} // namespace logency::unit_test::sink_module
