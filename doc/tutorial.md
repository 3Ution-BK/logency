# Tutorial

This is the working example using predefine structure in the library:

```c++
#include "logency/manager.hpp"
#include "logency/message/stream_message.hpp"
#include "logency/sink_module/console_module.hpp"

#include <iostream>

using message = logency::message::message;
using formatter = logency::message::message_formatter;

using manager = logency::manager<message>;
using sink_module = logency::sink_module::console_module<message, formatter>;

int main()
{
    manager my_manager{};

    auto my_logger{my_manager.new_logger("my_logger")};
    auto my_sink{my_manager.new_sink(
        "my_sink",
        std::make_unique<sink_module>(
            &std::cout, std::make_unique<formatter>()))};

    my_logger->add_sink(sink);

    logger->log(logency::log_level::trace, "Hello world!");
    logger->log(logency::log_level::debug, "Hello world!");
    logger->log(logency::log_level::info, "Hello world!");
    logger->log(logency::log_level::warning, "Hello world!");
    logger->log(logency::log_level::error, "Hello world!");
    logger->log(logency::log_level::critical, "Hello world!");

    return 0;
}
```

---

logency is a header library. Include the header file and you are good to go. (Remember to use c++17 compatible compiler)

logency only include source code that is actually need, no additional unused code is imported.

> e.g.
>
> * include "logency/manager.hpp" to include logging manager.
> * include "logency/message/message.hpp" to include your message.
> * include "logency/sink_module/console_module.hpp" to include your sink_module.
> * Unused code will not include into your repository.

---

## Essential component

It require many components to work properly. But user normally need to know 5 of them.

* Manager

  The registry of the logging system. It record the resources it need for logging system.

  When it instantiate, it will create necessary resources without any loggers and sinks.

  When it destructed, It release the logger, sinks, and all of the resources it allocate.

* Logger

  The entry of the logging message. Each message will be instantiated inside logger. Then it will pass it into the manager itself for further uses.

* Sink

  The output of the logging system. Each sink has it `sink_modules` to parse the message into useful output.

* Message

  The actual content of your message. It will be hanging around the system from the logger to the sink.

  User can choose whatever message it should carry.

* Formatter

  The parser of your message. It will generate the useful output from the message.

  User can choose whatever output it should be parsed.

---

## How to start

### 1. Select your message

Message determined what content you wish to log. You can add what you need, and discard what you don't.

If you don't know how to start, the library do provide some predefined structure in [`include/logency/message`](../include/logency/message/) that you might have interested.

If you wish to build your own message, consider reading the article [Custom Message/Message Structure](custom_message.md#message-structure).

### 2. Construct your manager

```c++
#include "logency/manager.hpp"

logency::manager<message> manager;
```

Thats it. Simple. Right?

Manager will record all of the message manipulate process. Including message dispatching, threading, outputting content and so forth.
Normally, user will not care too much about it.

### 3. Construct your logger

```c++
auto logger = manager.new_logger("my fancy logger name");
```

Logger is responsible for message entry. You put your message inside the logger, and it will produce formatted output.

Logger name should be storable in `string_type` and each logger should have its unique name.

see [Logger](logger.md) for more example.

### 4. Construct your sink

Sink is responsible for output your message.

Things get a little trickly here. We will discuss the step below:

#### 1. Sink module

Each sink is connect to its own sink module. Its main focus is to output and flush the message to its target.

Sink modules are diverse. It can be console, file, etc.

To create a sink_module, you need to know what module you should use.

```c++
#include "logency/sink_module/console_module.hpp"

auto console_module = std::make_unique<console_module>(&std::cout, std::make_unique<formatter>());
```

Please look over [`include/logency/sink_module`](../include/logency/sink_module/) to see what arguments it takes to create the module.

Some sink_module might need its additional type alias. See [Custom Message](custom_message.md) for more information.

#### 2. Formatter

Formatter is responsible for formatting your message. You need to output the message into something readable, that's where the formatter takes place.

The library do provide some formatter for predefined structure in [`include/logency/message`](../include/logency/message/) that you might have interested.

If you wish to build the formatter of your own, consider this article [Custom Message/Formatter](custom_message.md#formatter) for more information.

#### 3. Create your sink and connect to logger

You can create your sink by the manager.

```c++
auto sink{my_manager.new_sink("my wonderful sink name", std::move(sink_module))};
```

Sink name should be storable in `string_type` and each sink should have its unique name. You can overlap the sink name and logger name, however.
See [Manager/Name conflict](manager.md#name-conflict) for more info.

Connect your sink with logger and we are good to go.

```c++
logger->add_sink(sink);
```

### 5. Log the message

Now, everything is set. You can log the message.

```c++
logger->log("my awesome message.");
```

The `log()` function depend on the constructor of your message. See [Logger/Overview](logger.md#overview) for more info.
