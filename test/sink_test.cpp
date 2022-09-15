#include "logency/sink.hpp"

#include "logency/core/exception.hpp"
#include "logency/detail/message_pack.hpp"
#include "logency/detail/thread/thread_pool.hpp"

#include "global_resource/thread_pool.hpp"
#include "include_doctest.hpp"
#include "utils/mock_sink_module.hpp"
#include "utils/string.hpp"
#include "utils/test_message.hpp"

#include <memory>

namespace logency::unit_test
{

namespace
{

auto ordinary_sink() -> std::shared_ptr<logency::sink<utils::message<char>>>;

auto queue_only_sink(std::size_t &size)
    -> std::shared_ptr<logency::sink<utils::message<char>>>;

auto ordinary_sink() -> std::shared_ptr<logency::sink<utils::message<char>>>
{
    using message_type = utils::message<char>;
    using sink_type = logency::sink<message_type>;
    using sink_module = utils::mock_sink_module<message_type>;

    auto sink{
        std::make_shared<sink_type>("not used", std::make_unique<sink_module>(),
                                    global_resource::thread_pool::normal())};

    return sink;
}

auto queue_only_sink(std::size_t &size)
    -> std::shared_ptr<logency::sink<utils::message<char>>>
{
    using message_type = utils::message<char>;
    using sink_type = logency::sink<message_type>;
    using sink_module = utils::mock_sink_module<message_type>;

    auto sink{std::make_shared<sink_type>(
        "not used", std::make_unique<sink_module>(),
        std::shared_ptr<detail::thread::thread_pool>{})};

    try
    {
        std::vector<message_pack<message_type>> tray(size, nullptr);

        sink->log(tray.begin(), tray.end());
    }
    catch (const logency::runtime_error &e)
    {
        /* Ignore it.
         * Message will enqueue first, then it will check for thread pool.
         */
    }

    return sink;
}

} // namespace

TEST_SUITE("logency::sink")
{
    // message type does not affect unit test result in this suite.
    using message_type = utils::message<char>;
    using value_type = message_type::value_type;
    using string_type = message_type::string_type;
    using string_view_type = message_type::string_view_type;

    using message_pack_type = message_pack<message_type>;
    using sink_type = logency::sink<message_type>;
    using sink_module = utils::mock_sink_module<message_type>;

    SCENARIO("sink::sink(string_view_type, std::unique_ptr<sink_module_type>,"
             "std::weak_ptr<thread_pool_type>)")
    {
        GIVEN("not instantiated object")
        {
            std::shared_ptr<sink_type> sink{nullptr};

            WHEN("instantiate it with sink_module")
            {
                CHECK_NOTHROW({
                    sink = std::make_shared<sink_type>(
                        "not used", std::make_unique<sink_module>(),
                        global_resource::thread_pool::normal());
                });

                THEN("object is instantiated")
                {
                    CHECK(sink);

                    AND_THEN(
                        "has a empty queue which does not reserve any capacity")
                    {
                        CHECK(sink->is_queue_empty());
                        CHECK_EQ(sink->queue_capacity(), 0);
                    }
                }
            }

            WHEN("instantiate it with no module")
            {
                auto act{[&]
                         {
                             sink = std::make_shared<sink_type>(
                                 "not used",
                                 std::unique_ptr<sink_module>{nullptr},
                                 global_resource::thread_pool::normal());
                         }};

                THEN("throw logency::runtime_error")
                {
                    CHECK_THROWS_WITH_AS(act(), "No sink_module assigned.",
                                         logency::runtime_error);
                }
            }
        }
    }

    SCENARIO("sink::sink(string_view_type, std::unique_ptr<sink_module_type>,"
             "typename queue_type::size_type, std::weak_ptr<thread_pool_type>)")
    {
        GIVEN("not instantiated object")
        {
            std::shared_ptr<sink_type> sink{nullptr};

            WHEN("instantiate it with sink_module and reserved queue size")
            {
                constexpr const std::size_t reserved_capacity{10U};

                CHECK_NOTHROW({
                    sink = std::make_shared<sink_type>(
                        "not used", std::make_unique<sink_module>(),
                        reserved_capacity,
                        global_resource::thread_pool::normal());
                });

                THEN("object is instantiated")
                {
                    CHECK(sink);

                    AND_THEN("has a empty queue and its reserved capacity")
                    {
                        CHECK(sink->is_queue_empty());
                        CHECK_EQ(sink->queue_capacity(), reserved_capacity);
                    }
                }
            }

            WHEN("instantiate it with no module")
            {
                auto act{[&]
                         {
                             sink = std::make_shared<sink_type>(
                                 "not used",
                                 std::unique_ptr<sink_module>{nullptr}, 0U,
                                 global_resource::thread_pool::normal());
                         }};

                THEN("throw logency::runtime_error")
                {
                    CHECK_THROWS_WITH_AS(act(), "No sink_module assigned.",
                                         logency::runtime_error);
                }
            }
        }
    }

    SCENARIO("sink::~sink()")
    {
        GIVEN("instantiated object")
        {
            auto sink{ordinary_sink()};

            WHEN("destruct")
            {
                CHECK_NOTHROW({ sink.reset(); });

                THEN("object is destroyed") { CHECK(!sink); }
            }
        }
    }

    SCENARIO("void sink::reserve(size_type)")
    {
        GIVEN("instantiated object")
        {
            std::size_t capacity{4U};
            auto sink{queue_only_sink(capacity)};

            WHEN("reserve the sink with larger than its current capacity")
            {
                auto new_capacity{capacity + 1};

                CHECK_NOTHROW({ sink->reserve(new_capacity); });

                THEN("the capacity became larger than or equal to its original "
                     "reserved value")
                {
                    CHECK_GE(sink->queue_capacity(), new_capacity);
                }
            }

            WHEN("reserve the sink with no larger than its current capacity")
            {
                auto new_capacity{capacity};

                CHECK_NOTHROW({ sink->reserve(new_capacity); });

                THEN("the capacity remain unchanged")
                {
                    CHECK_EQ(sink->queue_capacity(), new_capacity);
                }
            }
        }
    }

    SCENARIO("void sink::shrink_to_fit()")
    {
        GIVEN("a sink with items and enough capacity")
        {
            auto sink{ordinary_sink()};
            const auto capacity{sink->queue_capacity()};

            WHEN("call function")
            {
                CHECK_NOTHROW({ sink->shrink_to_fit(); });

                THEN("the capacity remain unchanged")
                {
                    CHECK_EQ(sink->queue_capacity(), capacity);
                }
            }
        }

        GIVEN("a sink with items and some extra capacity")
        {
            std::size_t capacity{4U};
            auto sink{queue_only_sink(capacity)};

            WHEN("call function")
            {
                CHECK_NOTHROW({ sink->shrink_to_fit(); });

                THEN("the new capacity become no larger than its original one")
                {
                    CHECK_LE(sink->queue_capacity(), capacity);
                }
            }
        }
    }

    SCENARIO("void sink::set_filter(filter_type)")
    {
        GIVEN("instantiated object")
        {
            auto sink{ordinary_sink()};

            WHEN("set filter")
            {
                CHECK_NOTHROW({
                    sink->set_filter(
                        [](string_view_type, const message_type &message)
                        { return message.content == "qualify"; });
                });

                THEN("filter a qualified message will pass")
                {
                    auto pack{make_message_pack<message_type>(
                        std::make_shared<string_type>("not used"),
                        message_type{"qualify"})};

                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    sink->log(&pack, &pack + 1);

                    global_resource::thread_pool::normal()
                        ->wait_until_queue_empty();

                    CHECK_EQ(
                        dynamic_cast<const sink_module &>(sink->sink_module())
                            .log_counter(),
                        1);
                }

                THEN("filter a disqualified message will filtered out")
                {
                    auto pack{make_message_pack<message_type>(
                        std::make_shared<string_type>("not used"),
                        message_type{"disqualify"})};

                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    sink->log(&pack, &pack + 1);

                    global_resource::thread_pool::normal()
                        ->wait_until_queue_empty();

                    CHECK_EQ(
                        dynamic_cast<const sink_module &>(sink->sink_module())
                            .log_counter(),
                        0);
                }
            }
        }
    }

    SCENARIO("void sink::set_flusher(flusher_type)")
    {
        GIVEN("instantiated object")
        {
            auto sink{ordinary_sink()};

            WHEN("set flusher")
            {
                CHECK_NOTHROW({
                    sink->set_flusher(
                        [](string_view_type, const message_type &message)
                        { return message.content == "qualify"; });
                });

                THEN("flush a qualified message will trigger flush")
                {
                    auto pack{make_message_pack<message_type>(
                        std::make_shared<string_type>("not used"),
                        message_type{"qualify"})};

                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    sink->log(&pack, &pack + 1);

                    global_resource::thread_pool::normal()
                        ->wait_until_queue_empty();

                    CHECK_EQ(
                        dynamic_cast<const sink_module &>(sink->sink_module())
                            .flush_counter(),
                        1);
                }

                THEN("flush a disqualified message will not trigger flush")
                {
                    auto pack{make_message_pack<message_type>(
                        std::make_shared<string_type>("not used"),
                        message_type{"disqualify"})};

                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    sink->log(&pack, &pack + 1);

                    global_resource::thread_pool::normal()
                        ->wait_until_queue_empty();

                    CHECK_EQ(
                        dynamic_cast<const sink_module &>(sink->sink_module())
                            .flush_counter(),
                        0);
                }
            }
        }
    }

    SCENARIO("template <typename Iterator> "
             "void sink::log(Iterator begin, Iterator end)")
    {
        GIVEN("instantiated object")
        {
            auto sink{ordinary_sink()};

            WHEN("log message")
            {
                auto pack{make_message_pack<message_type>(
                    std::make_shared<string_type>("not used"),
                    message_type{"not used"})};

                CHECK_NOTHROW({
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    sink->log(&pack, &pack + 1);
                });

                THEN("message is successfully logged in")
                {
                    global_resource::thread_pool::normal()
                        ->wait_until_queue_empty();

                    CHECK_EQ(
                        dynamic_cast<const sink_module &>(sink->sink_module())
                            .log_counter(),
                        1);
                }
            }
        }

        GIVEN("instantiated object with destructed pool")
        {
            using pool_type = logency::detail::thread::thread_pool;
            auto sink{std::make_shared<sink_type>(
                "not used", std::make_unique<sink_module>(),
                std::shared_ptr<pool_type>{})};

            WHEN("log message")
            {
                auto pack{make_message_pack<message_type>(
                    std::make_shared<string_type>("not used"),
                    message_type{"not used"})};

                auto act{[&]()
                         {
                             // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                             sink->log(&pack, &pack + 1);
                         }};

                THEN("throw logency::runtime_error")
                {
                    CHECK_THROWS_WITH_AS(
                        act(), "Thread pool does not exist any longer.",
                        logency::runtime_error);
                }
            }
        }
    }

    SCENARIO(
        "auto sink::sink_module() const noexcept -> const sink_module_type &")
    {
        GIVEN("instantiated object")
        {
            auto instance{std::make_unique<sink_module>()};
            const auto *expect{instance.get()};

            auto sink{std::make_shared<sink_type>(
                "not used", std::move(instance),
                global_resource::thread_pool::normal())};

            WHEN("access location")
            {
                const auto &actual{sink->sink_module()};

                THEN("return same sink_module location from constructor")
                {
                    CHECK_EQ(std::addressof(
                                 dynamic_cast<const sink_module &>(actual)),
                             expect);
                }
            }
        }
    }

    SCENARIO("auto sink::sink_module() noexcept -> sink_module_type &")
    {
        GIVEN("instantiated object")
        {
            auto instance{std::make_unique<sink_module>()};
            const auto *expect{instance.get()};

            auto sink{std::make_shared<sink_type>(
                "not used", std::move(instance),
                global_resource::thread_pool::normal())};

            WHEN("access location")
            {
                auto &actual{sink->sink_module()};

                THEN("return same sink_module location from constructor")
                {
                    CHECK_EQ(
                        std::addressof(dynamic_cast<sink_module &>(actual)),
                        expect);
                }
            }
        }
    }

    SCENARIO("auto sink::name() const noexcept -> string_type")
    {
        GIVEN("instantiated object")
        {
            const value_type *expect{"sink name"};
            auto sink{std::make_shared<sink_type>(
                string_type{expect}, std::make_unique<sink_module>(),
                global_resource::thread_pool::normal())};

            WHEN("access name")
            {
                const auto actual{sink->name()};

                THEN("return same sink_module location from constructor")
                {
                    CHECK(utils::string::string_equal(actual, expect));
                }
            }
        }
    }

    SCENARIO("auto sink::queue_capacity() -> size_type")
    {
        GIVEN("instantiated object")
        {
            std::size_t capacity{4U};
            auto sink{queue_only_sink(capacity)};

            WHEN("call function")
            {
                std::size_t actual{0U};

                CHECK_NOTHROW({ actual = sink->queue_capacity(); });

                THEN("return the requested capacity")
                {
                    /* Actual capacity might vary. But it will be no smaller
                     * than original requested value.
                     */
                    CHECK_GE(actual, capacity);
                }
            }
        }
    }

    SCENARIO("auto sink::queue_size() -> size_type")
    {
        GIVEN("instantiated object")
        {
            std::size_t size{4U};
            auto sink{queue_only_sink(size)};

            WHEN("call function")
            {
                std::size_t actual{0U};

                CHECK_NOTHROW({ actual = sink->queue_size(); });

                THEN("return the requested size") { CHECK_EQ(actual, size); }
            }
        }
    }

    SCENARIO("bool sink::is_queue_empty()")
    {
        GIVEN("instantiated empty object")
        {
            auto sink{ordinary_sink()};

            WHEN("call function")
            {
                bool actual{};

                CHECK_NOTHROW({ actual = sink->is_queue_empty(); });

                THEN("return true") { CHECK(actual); }
            }
        }

        GIVEN("instantiated non-empty object")
        {
            std::size_t size{4U};
            auto sink{queue_only_sink(size)};

            WHEN("call function")
            {
                bool actual{};

                CHECK_NOTHROW({ actual = sink->is_queue_empty(); });

                THEN("return false") { CHECK(!actual); }
            }
        }
    }
}

} // namespace logency::unit_test
