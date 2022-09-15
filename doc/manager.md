# Manager

## Overview

Manager is the registry of the logging system. It record the resources it need for logging system.

It use RAII approach. That means it will instantiate when it is constructed, and deallocate when it is destructed.

```c++
logency::manager<my_message> manager;
logency::manager<my_another_message> manager2;
```

Based on this approach, you can cover the manager instance inside your class structure, make singleton of it or do whatever you like.

You can have as many message as possible, but most of the time one instance is already enough.

*As long as the manager is present, the logging system will be functional.*

## Logger resource

Logger is the entry point of the message.

All of the function listed here are meant to be thread safe.

### Allocate logger

```c++
auto manager::new_logger(string_view_type name) -> std::shared_ptr<logger_type>;
```

Create the new logger with specific name.
If such name is already registered, it will throw `logency::runtime_error()`.

```c++
void manager::delete_logger(string_view_type name);
```

Delete the logger with specific name.
If no such logger is presented in the manager, it will throw `logency::runtime_error()`.

```c++
auto manager::find_logger(string_view_type name) -> std::shared_ptr<logger_type>;
```

Find the logger with specific name.
Return its instance if the logger is registered inside the manager, otherwise return `nullptr`.

## Sink resource

Sink is the output point of the message.

Sink will be attached to `sink_module`, where the actual message parsing takes place.

Each sink should have its own `sink_module`.

All of the function listed here are meant to be thread safe.

### Allocate sink

```c++
template <typename SinkModule, typename... Args>
auto manager::new_sink(string_view_type name, Args... args) -> std::shared_ptr<sink_type>;

auto new_sink(string_view_type name, std::unique_ptr<sink_module_type> module) -> std::shared_ptr<sink_type>;
```

Create the new sink with specific name and module.
If such name is already registered, it will throw `logency::runtime_error()`.

```c++
void manager::delete_sink(string_view_type name);
```

Delete the sink with specific name.
If no such sink is presented in the manager, it will throw `logency::runtime_error()`.

```c++
auto manager::find_sink(string_view_type name) -> std::shared_ptr<sink_type>;
```

Find the sink with specific name.
Return its instance if the sink is registered inside the manager, otherwise return `nullptr`.

## Name conflict

Each logger inside the same manager should have its unique name.

```c++
auto logger = manager::new_logger("one");

auto prohibited = manager::new_logger("one"); // warning: prohibited
```

Each sink inside the same manager should have its unique name.

```c++
auto sink = manager::new_sink("one", ...);

auto prohibited1 = manager::new_sink("one", ...); // warning: prohibited
```

However, logger name and sink name can be the same since it use different database to record the resource its allocate.

```c++
std::shared_ptr<logger_type> allow1 = manager::new_logger("one");
std::shared_ptr<sink_type> allow2 = manager::new_sink("one", ...);
```

## Error handler

Manager can assign a error handler to prevent exception during logging.

The error handler will be triggered when something throws in logger during logging or when the thread throw.

Default manager does not have any error handler inside. It will **rethrow the exception if it is triggered in the logger** or **get ignored if it is triggered in the thread**.

It use `std::function<void(const std::exception &)>` to handle the error.

```c++
void manager::set_error_handler(
    [](const std::exception &) {/* your actual handler */});
```

Once it is set, it will copy the handler into the logger it holds. And every time the new logger get instantiated, it will copy into that logger as well.

## Wait until idle

Sometimes, user wish to change the state of the logging system. Some of it might not be thread safe.
Hence, this functionality try to solve the task.

All of the function listed here are meant to be thread safe.

```c++
void manager::wait_until_idle();
```

Wait until the thread pool inside the manager is idle.

When the function finished, it means that the **threads inside the manager** are sleeping right now. No other thread inside the manager are operating.

When the user `log()` the message, its thread pool will start processing the message (not idle).
Once there is no other message inside the manager, it will go back to idle state.

It is useful to change the non-thread-safe state of some functionalities of the system.
