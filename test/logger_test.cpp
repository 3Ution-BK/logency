#include "logency/core/exception.hpp"
#include "logency/logger.hpp"

#include "global_resource/dispatcher.hpp"
#include "global_resource/thread_pool.hpp"
#include "include_doctest.hpp"
#include "utils/mock_sink_module.hpp"
#include "utils/string.hpp"
#include "utils/test_message.hpp"

#include <cstddef>
#include <exception>
#include <iostream>
#include <memory>
#include <string_view>

namespace logency::unit_test
{

TEST_SUITE("logency::logger")
{
    using message_type = utils::message<char>;
    using value_type = message_type::value_type;
    using string_type = message_type::string_type;
    using string_view_type = message_type::string_view_type;

    using logger_type = logency::logger<message_type>;

    SCENARIO("logger::logger(string_type &&, std::weak_ptr<dispatcher_type>)")
    {
        GIVEN("not instantiated object")
        {
            std::shared_ptr<logger_type> logger{nullptr};

            WHEN("instantiate")
            {
                CHECK_NOTHROW({
                    logger = std::make_shared<logger_type>(
                        "logger", global_resource::dispatcher::normal());
                });

                THEN("object is instantiated") { CHECK(logger); }
            }
        }
    }

    SCENARIO("logger::~logger")
    {
        GIVEN("instantiated object")
        {
            auto logger{std::make_shared<logger_type>(
                "logger", global_resource::dispatcher::normal())};

            WHEN("destruct")
            {
                CHECK_NOTHROW({ logger.reset(); });

                THEN("object is destroyed") { CHECK(!logger); }
            }
        }
    }

    SCENARIO("template <typename... Args>"
             "void logger::log(Args &&...)")
    {
        GIVEN("instantiated object")
        {
            using sink_module = utils::mock_sink_module<message_type>;
            using sink_type = logency::sink<message_type>;

            auto logger{std::make_shared<logger_type>(
                "logger", global_resource::dispatcher::normal())};

            auto sink{std::make_shared<sink_type>(
                "sink", std::make_unique<sink_module>(),
                global_resource::thread_pool::normal())};

            logger->add_sink(sink);

            WHEN("log message")
            {
                CHECK_NOTHROW({ logger->log("message"); });

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
            auto logger{std::make_shared<logger_type>(
                "destructed thread pool",
                global_resource::dispatcher::invalid_thread_pool())};

            WHEN("log message")
            {
                auto act{[&]() { logger->log("message"); }};

                THEN("throw logency::runtime_error")
                {
                    CHECK_THROWS_WITH_AS(
                        act(), "Thread pool does not exist any longer.",
                        logency::runtime_error);
                }
            }
        }
    }

    SCENARIO("auto logger::name() const noexcept -> string_type")
    {
        GIVEN("instantiated object")
        {
            const value_type *expect{"name"};

            auto logger{std::make_shared<logger_type>(
                string_type{expect}, global_resource::dispatcher::normal())};

            WHEN("access name")
            {
                const auto actual{logger->name()};

                THEN("return same sink_module location from constructor")
                {
                    CHECK(utils::string::string_equal(actual, expect));
                }
            }
        }
    }

    SCENARIO("void logger::add_sink(sink_pointer_type)")
    {
        GIVEN("instantiated object")
        {
            auto logger{std::make_shared<logger_type>(
                "logger", global_resource::dispatcher::normal())};

            WHEN("add sink")
            {
                using sink_type = logency::sink<message_type>;
                using sink_module = utils::mock_sink_module<message_type>;

                auto sink{std::make_shared<sink_type>(
                    "sink", std::make_unique<sink_module>(),
                    global_resource::thread_pool::normal())};

                CHECK_NOTHROW({ logger->add_sink(sink); });

                THEN("the sink is connected to the logger")
                {
                    CHECK_EQ(logger->find_sink("sink").get(), sink.get());
                }
            }
        }
    }

    SCENARIO("void logger::find_sink(const string_type &name)")
    {
        GIVEN("instantiated object with target sink connected")
        {
            using sink_type = logency::sink<message_type>;
            using sink_module = utils::mock_sink_module<message_type>;

            auto logger{std::make_shared<logger_type>(
                "logger", global_resource::dispatcher::normal())};

            auto sink{std::make_shared<sink_type>(
                "sink", std::make_unique<sink_module>(),
                global_resource::thread_pool::normal())};

            logger->add_sink(sink);

            WHEN("find sink")
            {
                std::shared_ptr<sink_type> result;

                CHECK_NOTHROW({ result = logger->find_sink("sink"); });

                THEN("the sink is founded and it points to the target sink")
                {
                    CHECK_EQ(result.get(), sink.get());
                }
            }
        }

        GIVEN("instantiated object with target sink not connected")
        {
            using sink_type = logency::sink<message_type>;

            auto logger{std::make_shared<logger_type>(
                "logger", global_resource::dispatcher::normal())};

            WHEN("find sink")
            {
                std::shared_ptr<sink_type> result;

                CHECK_NOTHROW({ result = logger->find_sink("sink"); });

                THEN("the sink is not founded")
                {
                    CHECK_EQ(result.get(), nullptr);
                }
            }
        }
    }

    SCENARIO("void logger::delete_sink(string_view_type)")
    {
        GIVEN("instantiated object with target sink connected")
        {
            using sink_type = logency::sink<message_type>;
            using sink_module = utils::mock_sink_module<message_type>;

            auto logger{std::make_shared<logger_type>(
                "logger", global_resource::dispatcher::normal())};

            auto sink{std::make_shared<sink_type>(
                "sink", std::make_unique<sink_module>(),
                global_resource::thread_pool::normal())};

            logger->add_sink(sink);

            WHEN("delete sink")
            {
                CHECK_NOTHROW({ logger->delete_sink("sink"); });

                THEN("the sink is deleted")
                {
                    CHECK_EQ(logger->find_sink("sink").get(), nullptr);
                }
            }
        }

        GIVEN("instantiated object with target sink not connected")
        {
            auto logger{std::make_shared<logger_type>(
                "logger", global_resource::dispatcher::normal())};

            WHEN("delete sink")
            {
                auto act{[&]() { logger->delete_sink("sink"); }};

                THEN("throw logency::runtime_error")
                {
                    CHECK_THROWS_WITH_AS(
                        act(), "Cannot found the sink requested in the logger.",
                        logency::runtime_error);
                }
            }
        }
    }

    SCENARIO("void logger::delete_sink(sink_type)")
    {
        GIVEN("instantiated object with target sink connected")
        {
            using sink_type = logency::sink<message_type>;
            using sink_module = utils::mock_sink_module<message_type>;

            auto logger{std::make_shared<logger_type>(
                "logger", global_resource::dispatcher::normal())};

            auto sink{std::make_shared<sink_type>(
                "sink", std::make_unique<sink_module>(),
                global_resource::thread_pool::normal())};

            logger->add_sink(sink);

            WHEN("delete sink")
            {
                CHECK_NOTHROW({ logger->delete_sink(sink); });

                THEN("the sink is deleted")
                {
                    CHECK_EQ(logger->find_sink("sink").get(), nullptr);
                }
            }
        }

        GIVEN("instantiated object with target sink not connected")
        {
            using sink_type = logency::sink<message_type>;
            using sink_module = utils::mock_sink_module<message_type>;

            auto logger{std::make_shared<logger_type>(
                "logger", global_resource::dispatcher::normal())};

            auto sink{std::make_shared<sink_type>(
                "sink", std::make_unique<sink_module>(),
                global_resource::thread_pool::normal())};

            WHEN("delete sink")
            {
                auto act{[&]() { logger->delete_sink(sink); }};

                THEN("throw logency::runtime_error")
                {
                    CHECK_THROWS_WITH_AS(
                        act(), "Cannot found the sink requested in the logger.",
                        logency::runtime_error);
                }
            }
        }
    }

    SCENARIO("void logger::set_filter(filter_type)")
    {
        GIVEN("instantiated object")
        {
            using sink_type = logency::sink<message_type>;
            using sink_module = utils::mock_sink_module<message_type>;

            auto logger{std::make_shared<logger_type>(
                "logger", global_resource::dispatcher::normal())};

            auto sink{std::make_shared<sink_type>(
                "sink", std::make_unique<sink_module>(),
                global_resource::thread_pool::normal())};

            logger->add_sink(sink);

            WHEN("set filter")
            {
                CHECK_NOTHROW({
                    logger->set_filter(
                        [](string_view_type, const message_type &message)
                        { return message.content == "qualify"; });
                });

                THEN("filter a qualified message will pass")
                {
                    CHECK_NOTHROW({ logger->log("qualify"); });

                    global_resource::thread_pool::normal()
                        ->wait_until_queue_empty();

                    CHECK_EQ(
                        dynamic_cast<const sink_module &>(sink->sink_module())
                            .log_counter(),
                        1);
                }

                THEN("filter a disqualified message will filtered out")
                {
                    CHECK_NOTHROW({ logger->log("disqualify"); });

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

    SCENARIO("void logger::set_error_handler(error_handler_type)")
    {
        GIVEN("instantiated object")
        {
            auto logger{std::make_shared<logger_type>(
                "logger", global_resource::dispatcher::normal())};

            int handler_trigger_counter{0};

            WHEN("no handler is set")
            {
                CHECK_NOTHROW({
                    logger->set_error_handler(
                        logger_type::error_handler_type{});
                });

                THEN("log the no throw message will not trigger handler")
                {
                    CHECK_NOTHROW({ logger->log("will not throw"); });

                    global_resource::thread_pool::normal()
                        ->wait_until_queue_empty();
                }

                THEN("log the throw message will throw exception outside of "
                     "the log function")
                {
                    string_type message{"will throw"};
                    auto act{[&]()
                             { logger->log(message, message.size() + 1U); }};

                    CHECK_THROWS(act()); // Can throw any exceptions.
                }
            }

            WHEN("set handler")
            {
                CHECK_NOTHROW({
                    logger->set_error_handler([&](const std::exception &)
                                              { ++handler_trigger_counter; });
                });

                THEN("log the no throw message will not trigger handler")
                {
                    CHECK_NOTHROW({ logger->log("will not throw"); });

                    global_resource::thread_pool::normal()
                        ->wait_until_queue_empty();

                    CHECK_EQ(handler_trigger_counter, 0);
                }

                THEN("log the throw message will trigger handler without "
                     "throw exception outside of the log function")
                {
                    string_type message{"will throw"};
                    CHECK_NOTHROW(
                        { logger->log(message, message.size() + 1U); });

                    global_resource::thread_pool::normal()
                        ->wait_until_queue_empty();

                    CHECK_EQ(handler_trigger_counter, 1);
                }
            }
        }
    }
}

} // namespace logency::unit_test
