#include "logency/dispatcher.hpp"

#include "logency/detail/message_pack.hpp"
#include "logency/sink.hpp"

#include "global_resource/thread_pool.hpp"
#include "include_doctest.hpp"
#include "utils/mock_sink_module.hpp"

#include "utils/test_message.hpp"

#include <iostream>
#include <memory>
#include <utility>

namespace logency::unit_test
{

namespace
{

auto new_logger(
    std::weak_ptr<logency::dispatcher<utils::message<char>>> &&dispatcher,
    std::shared_ptr<logency::sink<utils::message<char>>> sink)
    -> std::shared_ptr<logency::logger<utils::message<char>>>;

auto ordinary_dispatcher()
    -> std::shared_ptr<logency::dispatcher<utils::message<char>>>;

auto queue_only_dispatcher(std::size_t &size)
    -> std::shared_ptr<logency::dispatcher<utils::message<char>>>;

auto new_logger(
    std::weak_ptr<logency::dispatcher<utils::message<char>>> &&dispatcher,
    std::shared_ptr<logency::sink<utils::message<char>>> sink)
    -> std::shared_ptr<logency::logger<utils::message<char>>>
{
    using message_type = utils::message<char>;
    using logger_type = logency::logger<message_type>;

    auto logger{std::make_shared<logger_type>(std::string{"logger"},
                                              std::move(dispatcher))};

    logger->add_sink(std::move(sink));

    return logger;
}

auto ordinary_dispatcher()
    -> std::shared_ptr<logency::dispatcher<utils::message<char>>>
{
    using message_type = utils::message<char>;
    using dispatcher_type = logency::dispatcher<message_type>;

    return std::make_shared<dispatcher_type>(
        global_resource::thread_pool::normal());
}

auto queue_only_dispatcher(std::size_t &size)
    -> std::shared_ptr<logency::dispatcher<utils::message<char>>>
{
    using message_type = utils::message<char>;
    using dispatcher_type = logency::dispatcher<message_type>;

    auto dispatcher{std::make_shared<dispatcher_type>(
        std::shared_ptr<detail::thread::thread_pool>{})};

    for (int iter{0}; iter < static_cast<int>(size); ++iter)
    {
        try
        {
            using logger_type = std::shared_ptr<dispatcher_type::logger_type>;
            using message_pack_type =
                std::shared_ptr<logency::message_pack_base<message_type>>;

            dispatcher->enqueue(logger_type{nullptr},
                                message_pack_type{nullptr});
        }
        catch (const logency::runtime_error &e)
        {
            /* Ignore it.
             * Message will enqueue first, then it will check for thread pool.
             *
             * It only throw the first time it log. But we use try block in
             * every operation anyways.
             */
        }
    }

    return dispatcher;
}

} // namespace

TEST_SUITE("logency::dispatcher")
{
    using message_type = utils::message<char>;
    using string_type = message_type::string_type;
    using string_view_type = message_type::string_view_type;

    using dispatcher_type = logency::dispatcher<message_type>;
    using logger_type = dispatcher_type::logger_type;
    using message_pack_type =
        std::shared_ptr<logency::message_pack_base<message_type>>;
    using sink_type = logency::sink<message_type>;
    using sink_module = utils::mock_sink_module<message_type>;

    SCENARIO("dispatcher::dispatcher(std::weak_ptr<thread_pool_type>)")
    {
        GIVEN("not instantiated object")
        {
            std::shared_ptr<dispatcher_type> dispatcher{nullptr};

            WHEN("instantiate it")
            {
                CHECK_NOTHROW({
                    dispatcher = std::make_shared<dispatcher_type>(
                        global_resource::thread_pool::normal());
                });

                THEN("object is instantiated") { CHECK(dispatcher); }
            }
        }
    }

    SCENARIO("dispatcher::~dispatcher")
    {
        GIVEN("instantiated object")
        {
            auto dispatcher{ordinary_dispatcher()};

            WHEN("destruct")
            {
                CHECK_NOTHROW({ dispatcher.reset(); });

                THEN("object is destroyed") { CHECK(!dispatcher); }
            }
        }
    }

    SCENARIO("void dispatcher::reserve(size_type)")
    {
        GIVEN("instantiated object")
        {
            std::size_t capacity{4U};
            auto dispatcher{queue_only_dispatcher(capacity)};

            WHEN("reserve the dispatcher with larger than its current capacity")
            {
                auto new_capacity{capacity + 1};

                CHECK_NOTHROW({ dispatcher->reserve(new_capacity); });

                THEN("the capacity became larger than or equal to its original "
                     "reserved value")
                {
                    CHECK_GE(dispatcher->queue_capacity(), new_capacity);
                }
            }

            WHEN("reserve the dispatcher with no larger than its current "
                 "capacity")
            {
                auto new_capacity{capacity};

                CHECK_NOTHROW({ dispatcher->reserve(new_capacity); });

                THEN("the capacity remain unchanged")
                {
                    CHECK_EQ(dispatcher->queue_capacity(), new_capacity);
                }
            }
        }
    }

    SCENARIO("void dispatcher::shrink_to_fit()")
    {
        GIVEN("a dispatcher with items and enough capacity")
        {
            auto dispatcher{ordinary_dispatcher()};
            const auto capacity{dispatcher->queue_capacity()};

            WHEN("call function")
            {
                CHECK_NOTHROW({ dispatcher->shrink_to_fit(); });

                THEN("the capacity remain unchanged")
                {
                    CHECK_EQ(dispatcher->queue_capacity(), capacity);
                }
            }
        }

        GIVEN("a dispatcher with items and some extra capacity")
        {
            std::size_t capacity{4U};
            auto dispatcher{queue_only_dispatcher(capacity)};

            WHEN("call function")
            {
                CHECK_NOTHROW({ dispatcher->shrink_to_fit(); });

                THEN("the new capacity become no larger than its original one")
                {
                    CHECK_LE(dispatcher->queue_capacity(), capacity);
                }
            }
        }
    }

    SCENARIO("void dispatcher::enqueue("
             "std::shared_ptr<logger_type> &&, message_pack_type &&)")
    {
        GIVEN("instantiated object")
        {
            auto dispatcher{ordinary_dispatcher()};
            auto sink{std::make_shared<sink_type>(
                "sink", std::make_unique<sink_module>(),
                global_resource::thread_pool::normal())};
            auto logger{new_logger(dispatcher, sink)};

            WHEN("queue message")
            {
                CHECK_NOTHROW({
                    dispatcher->enqueue(
                        std::shared_ptr<logger_type>{logger},
                        make_message_pack<utils::message<char>>(
                            std::make_shared<string_type>(logger->name()),
                            message_type{}));
                });

                THEN("message is successfully queued in")
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
            auto dispatcher{std::make_shared<dispatcher_type>(
                std::shared_ptr<pool_type>{})};

            WHEN("log message")
            {
                auto logger{std::make_shared<logger_type>(
                    std::string{"not used"}, dispatcher)};

                auto act{[&]()
                         {
                             dispatcher->enqueue(
                                 std::shared_ptr<logger_type>{logger},
                                 message_pack_type{});
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

    SCENARIO("auto dispatcher::queue_capacity() -> size_type")
    {
        GIVEN("instantiated object")
        {
            std::size_t capacity{4U};
            auto dispatcher{queue_only_dispatcher(capacity)};

            WHEN("call function")
            {
                std::size_t actual{0U};

                CHECK_NOTHROW({ actual = dispatcher->queue_capacity(); });

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

    SCENARIO("auto dispatcher::queue_size() -> size_type")
    {
        GIVEN("instantiated object")
        {
            std::size_t size{4U};
            auto dispatcher{queue_only_dispatcher(size)};

            WHEN("call function")
            {
                std::size_t actual{0U};

                CHECK_NOTHROW({ actual = dispatcher->queue_size(); });

                THEN("return the requested size") { CHECK_EQ(actual, size); }
            }
        }
    }

    SCENARIO("bool dispatcher::is_queue_empty()")
    { // Cannot check properly because it is rely on other thread(thread pool).
        GIVEN("instantiated empty object")
        {
            auto dispatcher{ordinary_dispatcher()};

            WHEN("call function")
            {
                bool actual{};

                CHECK_NOTHROW({ actual = dispatcher->is_queue_empty(); });

                THEN("return true") { CHECK(actual); }
            }
        }

        GIVEN("instantiated non-empty object")
        {
            std::size_t size{4U};
            auto dispatcher{queue_only_dispatcher(size)};

            WHEN("call function")
            {
                bool actual{};

                CHECK_NOTHROW({ actual = dispatcher->is_queue_empty(); });

                THEN("return false") { CHECK(!actual); }
            }
        }
    }
}

} // namespace logency::unit_test
