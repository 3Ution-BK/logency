#include "logency/core/exception.hpp"
#include "logency/manager.hpp"

#include "include_doctest.hpp"
#include "utils/mock_sink_module.hpp"
#include "utils/test_message.hpp"

#include <memory>

namespace logency::unit_test
{

TEST_SUITE("logency::manager")
{
    using message_type = utils::message<char>;
    using value_type = message_type::value_type;
    using string_type = message_type::string_type;
    using string_view_type = message_type::string_view_type;

    using manager_type = logency::manager<message_type>;
    using logger_type = manager_type::logger_type;
    using sink_type = manager_type::sink_type;
    using sink_module_type = utils::mock_sink_module<message_type>;

    SCENARIO("manager::manager()")
    {
        GIVEN("not instantiated object")
        {
            std::unique_ptr<manager_type> manager{nullptr};

            WHEN("instantiate")
            {
                CHECK_NOTHROW({ manager = std::make_unique<manager_type>(); });

                THEN("object is instantiated") { CHECK(manager); }
            }
        }
    }

    SCENARIO("manager::manager()")
    {
        GIVEN("not instantiated object")
        {
            std::unique_ptr<manager_type> manager{nullptr};

            WHEN("instantiate")
            {
                CHECK_NOTHROW({ manager = std::make_unique<manager_type>(); });

                THEN("object is instantiated") { CHECK(manager); }
            }
        }
    }

    SCENARIO("manager::manager(size_t)")
    {
        GIVEN("not instantiated object")
        {
            std::unique_ptr<manager_type> manager{nullptr};

            WHEN("instantiate with requested thread number")
            {
                CHECK_NOTHROW({ manager = std::make_unique<manager_type>(2); });

                THEN("object is instantiated") { CHECK(manager); }
            }
        }
    }

    SCENARIO("manager::~manager")
    {
        GIVEN("instantiated object")
        {
            auto manager{std::make_unique<manager_type>()};

            WHEN("destruct")
            {
                CHECK_NOTHROW({ manager.reset(); });

                THEN("object is destroyed") { CHECK(!manager); }
            }
        }
    }

    SCENARIO("auto manager::new_logger(string_view_type) "
             "-> std::shared_ptr<logger_type>")
    {
        GIVEN("instantiated empty manager")
        {
            manager_type manager;

            WHEN("instantiate new logger")
            {
                std::shared_ptr<logger_type> logger{};
                CHECK_NOTHROW({ logger = manager.new_logger("new logger"); });

                THEN("logger is instantiated")
                {
                    CHECK(logger);

                    AND_THEN("logger in registered in manager")
                    {
                        auto find{manager.find_logger("new logger")};

                        CHECK_EQ(find.get(), logger.get());
                    }
                }
            }
        }

        GIVEN("instantiated not empty manager")
        {
            manager_type manager;
            std::ignore = manager.new_logger("exist logger");
            std::ignore = manager.new_sink<sink_module_type>("exist sink");

            WHEN("instantiate new logger with not exist logger name")
            {
                std::shared_ptr<logger_type> logger{};
                CHECK_NOTHROW({ logger = manager.new_logger("new logger"); });

                THEN("logger is instantiated")
                {
                    CHECK(logger);

                    AND_THEN("logger in registered in manager")
                    {
                        auto find{manager.find_logger("new logger")};

                        CHECK_EQ(find.get(), logger.get());
                    }
                }
            }

            WHEN("instantiate new logger with exist logger name")
            {
                auto act{[&]()
                         { std::ignore = manager.new_logger("exist logger"); }};

                THEN("throw logency::runtime_error")
                {
                    CHECK_THROWS_WITH_AS(
                        act(), "Already assigned a logger with the same name.",
                        logency::runtime_error);
                }
            }

            WHEN("instantiate new logger with exist sink name")
            {
                std::shared_ptr<logger_type> logger{};
                CHECK_NOTHROW({ logger = manager.new_logger("exist sink"); });

                THEN("logger is instantiated")
                {
                    CHECK(logger);

                    AND_THEN("logger in registered in manager")
                    {
                        auto find{manager.find_logger("exist sink")};

                        CHECK_EQ(find.get(), logger.get());
                    }
                }
            }
        }
    }

    SCENARIO("auto manager::find_logger(string_view_type) "
             "-> std::shared_ptr<logger_type>")
    {
        GIVEN("instantiated manager")
        {
            manager_type manager;
            auto logger{manager.new_logger("exist logger")};

            WHEN("find exist logger")
            {
                std::shared_ptr<logger_type> result;
                CHECK_NOTHROW(
                    { result = manager.find_logger("exist logger"); });

                THEN("logger is found")
                {
                    CHECK_NE(result.get(), nullptr);

                    AND_THEN("The found result point to the requested logger")
                    {
                        CHECK_EQ(result.get(), logger.get());
                    }
                }
            }

            WHEN("find not exist logger")
            {
                std::shared_ptr<logger_type> result;
                CHECK_NOTHROW(
                    { result = manager.find_logger("not exist logger"); });

                THEN("logger is not found") { CHECK_EQ(result.get(), nullptr); }
            }
        }
    }

    SCENARIO("void manager::delete_logger(string_view_type)")
    {
        GIVEN("instantiated manager")
        {
            manager_type manager;
            auto logger{manager.new_logger("exist logger")};

            WHEN("delete exist logger")
            {
                CHECK_NOTHROW({ manager.delete_logger("exist logger"); });

                THEN("logger is unregister from manager")
                {
                    CHECK_EQ(manager.find_logger("exist logger").get(),
                             nullptr);

                    AND_THEN("log the message in deleted logger will throw "
                             "logency::runtime_error")
                    {
                        CHECK_THROWS_WITH_AS(
                            { logger->log(); },
                            "This logger is marked as destroyed. "
                            "It is illegal to log this logger anymore.",
                            logency::runtime_error);
                    }
                }
            }

            WHEN("delete not exist logger")
            {
                auto act{[&]() { manager.delete_logger("not exist logger"); }};

                THEN("throw logency::runtime_error")
                {
                    CHECK_THROWS_WITH_AS(act(), "No such name in the manager.",
                                         logency::runtime_error);
                }
            }
        }
    }

    SCENARIO("template <typename SinkModule, typename... Args> "
             "auto manager::new_sink(string_view_type, Args...) "
             "-> std::shared_ptr<sink_type>;")
    {
        GIVEN("instantiated empty manager")
        {
            manager_type manager;

            WHEN("instantiate new sink")
            {
                std::shared_ptr<sink_type> sink{};
                CHECK_NOTHROW(
                    { sink = manager.new_sink<sink_module_type>("new sink"); });

                THEN("logger is instantiated")
                {
                    CHECK(sink);

                    AND_THEN("sink in registered in manager")
                    {
                        auto find{manager.find_sink("new sink")};

                        CHECK_EQ(find.get(), sink.get());
                    }
                }
            }

            WHEN("instantiate new sink with required argument")
            {
                std::shared_ptr<sink_type> sink{};
                CHECK_NOTHROW({
                    sink =
                        manager.new_sink<sink_module_type>("new sink", nullptr);
                });

                THEN("logger is instantiated")
                {
                    CHECK(sink);

                    AND_THEN("sink in registered in manager")
                    {
                        auto find{manager.find_sink("new sink")};

                        CHECK_EQ(find.get(), sink.get());
                    }
                }
            }
        }

        GIVEN("instantiated not empty manager")
        {
            manager_type manager;
            std::ignore = manager.new_logger("exist logger");
            std::ignore = manager.new_sink<sink_module_type>("exist sink");

            WHEN("instantiate new sink with not exist sink name")
            {
                std::shared_ptr<sink_type> sink{};
                CHECK_NOTHROW(
                    { sink = manager.new_sink<sink_module_type>("new sink"); });

                THEN("sink is instantiated")
                {
                    CHECK(sink);

                    AND_THEN("sink in registered in manager")
                    {
                        auto find{manager.find_sink("new sink")};

                        CHECK_EQ(find.get(), sink.get());
                    }
                }
            }

            WHEN("instantiate new sink with not exist sink name and required "
                 "argument")
            {
                std::shared_ptr<sink_type> sink{};
                CHECK_NOTHROW({
                    sink =
                        manager.new_sink<sink_module_type>("new sink", nullptr);
                });

                THEN("sink is instantiated")
                {
                    CHECK(sink);

                    AND_THEN("sink in registered in manager")
                    {
                        auto find{manager.find_sink("new sink")};

                        CHECK_EQ(find.get(), sink.get());
                    }
                }
            }

            WHEN("instantiate new sink with exist sink name")
            {
                auto act{[&]() {
                    std::ignore =
                        manager.new_sink<sink_module_type>("exist sink");
                }};

                THEN("throw logency::runtime_error")
                {
                    CHECK_THROWS_WITH_AS(
                        act(), "Already assigned a sink with the same name.",
                        logency::runtime_error);
                }
            }

            WHEN("instantiate new sink with exist sink name and required "
                 "argument")
            {
                auto act{[&]() {
                    std::ignore = manager.new_sink<sink_module_type>(
                        "exist sink", nullptr);
                }};

                THEN("throw logency::runtime_error")
                {
                    CHECK_THROWS_WITH_AS(
                        act(), "Already assigned a sink with the same name.",
                        logency::runtime_error);
                }
            }

            WHEN("instantiate new sink with exist logger name")
            {
                std::shared_ptr<sink_type> sink{};
                CHECK_NOTHROW({
                    sink = manager.new_sink<sink_module_type>("exist logger");
                });

                THEN("sink is instantiated")
                {
                    CHECK(sink);

                    AND_THEN("sink in registered in manager")
                    {
                        auto find{manager.find_sink("exist logger")};

                        CHECK_EQ(find.get(), sink.get());
                    }
                }
            }

            WHEN("instantiate new sink with exist logger name and required "
                 "argument")
            {
                std::shared_ptr<sink_type> sink{};
                CHECK_NOTHROW({
                    sink = manager.new_sink<sink_module_type>("exist logger",
                                                              nullptr);
                });

                THEN("sink is instantiated")
                {
                    CHECK(sink);

                    AND_THEN("sink in registered in manager")
                    {
                        auto find{manager.find_sink("exist logger")};

                        CHECK_EQ(find.get(), sink.get());
                    }
                }
            }
        }
    }

    SCENARIO("auto manager::new_sink(string_view_type, "
             "std::unique_ptr<sink_module_type>) "
             "-> std::shared_ptr<sink_type>;")
    {
        GIVEN("instantiated empty manager")
        {
            manager_type manager;

            WHEN("instantiate new sink")
            {
                std::shared_ptr<sink_type> sink{};
                CHECK_NOTHROW({
                    sink = manager.new_sink(
                        "new sink", std::make_unique<sink_module_type>());
                });

                THEN("logger is instantiated")
                {
                    CHECK(sink);

                    AND_THEN("sink in registered in manager")
                    {
                        auto find{manager.find_sink("new sink")};

                        CHECK_EQ(find.get(), sink.get());
                    }
                }
            }
        }

        GIVEN("instantiated not empty manager")
        {
            manager_type manager;
            std::ignore = manager.new_logger("exist logger");
            std::ignore = manager.new_sink<sink_module_type>("exist sink");

            WHEN("instantiate new sink with not exist sink name")
            {
                std::shared_ptr<sink_type> sink{};
                CHECK_NOTHROW({
                    sink = manager.new_sink(
                        "new sink", std::make_unique<sink_module_type>());
                });

                THEN("sink is instantiated")
                {
                    CHECK(sink);

                    AND_THEN("sink in registered in manager")
                    {
                        auto find{manager.find_sink("new sink")};

                        CHECK_EQ(find.get(), sink.get());
                    }
                }
            }

            WHEN("instantiate new sink with exist sink name")
            {
                auto act{[&]()
                         {
                             std::ignore = manager.new_sink(
                                 "exist sink",
                                 std::make_unique<sink_module_type>());
                         }};

                THEN("throw logency::runtime_error")
                {
                    CHECK_THROWS_WITH_AS(
                        act(), "Already assigned a sink with the same name.",
                        logency::runtime_error);
                }
            }

            WHEN("instantiate new sink with exist logger name")
            {
                std::shared_ptr<sink_type> sink{};
                CHECK_NOTHROW({
                    sink = manager.new_sink(
                        "exist logger", std::make_unique<sink_module_type>());
                });

                THEN("sink is instantiated")
                {
                    CHECK(sink);

                    AND_THEN("sink in registered in manager")
                    {
                        auto find{manager.find_sink("exist logger")};

                        CHECK_EQ(find.get(), sink.get());
                    }
                }
            }
        }
    }

    SCENARIO("auto manager::find_sink(string_view_type) "
             "-> std::shared_ptr<logger_type>")
    {
        GIVEN("instantiated manager")
        {
            manager_type manager;
            auto logger{manager.new_logger("exist logger")};

            WHEN("find exist logger")
            {
                std::shared_ptr<logger_type> result;
                CHECK_NOTHROW(
                    { result = manager.find_logger("exist logger"); });

                THEN("logger is found")
                {
                    CHECK_NE(result.get(), nullptr);

                    AND_THEN("The found result point to the requested logger")
                    {
                        CHECK_EQ(result.get(), logger.get());
                    }
                }
            }

            WHEN("find not exist logger")
            {
                std::shared_ptr<logger_type> result;
                CHECK_NOTHROW(
                    { result = manager.find_logger("not exist logger"); });

                THEN("logger is not found") { CHECK_EQ(result.get(), nullptr); }
            }
        }
    }

    SCENARIO("void manager::delete_sink(string_view_type)")
    {
        GIVEN("instantiated manager")
        {
            manager_type manager;
            auto logger{manager.new_sink<sink_module_type>("exist sink")};

            WHEN("delete exist sink")
            {
                CHECK_NOTHROW({ manager.delete_sink("exist sink"); });

                THEN("sink is unregister from manager")
                {
                    CHECK_EQ(manager.find_sink("exist sink").get(), nullptr);
                }
            }

            WHEN("delete not exist sink")
            {
                auto act{[&]() { manager.delete_sink("not exist sink"); }};

                THEN("throw logency::runtime_error")
                {
                    CHECK_THROWS_WITH_AS(act(), "No such name in the manager.",
                                         logency::runtime_error);
                }
            }
        }
    }

    SCENARIO("void manager::set_error_handler(error_handler_type)")
    {
        GIVEN("instantiated manager")
        {
            auto manager{std::make_unique<manager_type>()};
            auto exist_logger{manager->new_logger("exist logger")};

            int handler_trigger_counter{0};

            WHEN("set handler")
            {
                CHECK_NOTHROW({
                    manager->set_error_handler([&](const std::exception &)
                                               { ++handler_trigger_counter; });
                });

                THEN("the error handler in exist logger will be updated")
                {
                    // Throw the message to trigger counter
                    string_type message{"will throw"};
                    CHECK_NOTHROW(
                        { exist_logger->log(message, message.size() + 1U); });

                    // Thread pool will stop operation when manager deallocate.
                    manager.reset();

                    CHECK_EQ(handler_trigger_counter, 1);
                }

                THEN("the error handler in new logger is the same as manager")
                {
                    auto new_logger{manager->new_logger("new logger")};

                    // Throw the message to trigger counter
                    string_type message{"will throw"};
                    CHECK_NOTHROW(
                        { new_logger->log(message, message.size() + 1U); });

                    // Thread pool will stop operation when manager deallocate.
                    manager.reset();

                    CHECK_EQ(handler_trigger_counter, 1);
                }
            }
        }
    }
}

} // namespace logency::unit_test
