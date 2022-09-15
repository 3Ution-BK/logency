# logency

Pipeline driven template logging header-only library written in c++17.

---

> This library is primary made for practice purpose by the author.
> The author does not plan to active develop this library.
>
> In addition, this library is not meant for performance, and has been surpassed by multiple battle-tested library.
>
> If you wish to get a proper c++ logging library, consider using another library.
>
> We listed some library below for you to choose:
>
> * [spdlog](https://github.com/gabime/spdlog)
> * [plog](https://github.com/SergiusTheBest/plog)
> * [Nanolog](https://github.com/PlatformLab/NanoLog)
> * [Boost.log](https://github.com/boostorg/log)
> * And many others...

mumbles...

mumbles...

mumbles...

> Ohh...
>
> Well, you're still here. I guess you really want to know what it is.
>
> Ok then. Let me be professional. Now where should we start...

---

## Overview

logency is the logging library aim for extensity. It use template to let user choose what content user wish to log.

Below code [example/custom_message.cpp](example/custom_message.cpp) shows how extensible it can be:

```c++
#include "logency/manager.hpp"
#include "logency/sink_module/console_module.hpp"

#include <iostream>
#include <sstream>
#include <string>

struct message
{
    // Define proper type in your structure for manager
    using value_type = char;
    using string_type = std::string;
    using string_view_type = std::string_view;

    // Define additional type in your structure for sink_module
    using traits_type = std::char_traits<char>;

    // Write down your content
    std::string content;
    int i_need_it;
    float i_need_it_too;
};

class formatter
{
public:
    // Define what content you wish to print
    auto operator()(std::string_view logger, const message &message) const
        -> std::string
    {
        std::stringstream stream;
        stream << logger << ": " << message.content << " {" << message.i_need_it
               << ", " << message.i_need_it_too << "}\n";

        return stream.str();
    }
};

int main()
{
    using sink_module =
        logency::sink_module::console_module<message, formatter>;

    logency::manager<message> manager;

    auto logger{manager.new_logger("this is logger")};
    auto sink{manager.new_sink(
        "this is sink",
        std::make_unique<sink_module>(
            &std::cout, std::make_unique<formatter>()))};

    logger->add_sink(sink);

    logger->log("my content", 0, 1.0F);
    logger->log("another content", -1, 0.0F);

    return 0;
}
```

The following source code will produce the content in the console:

```bash
this is logger: my content {0, 1}
this is logger: another content {-1, 0}
```

## Feature

* Header only.
* C++17
* Asynchronous logging (multithread logger).
* User defined message & format.
* No unnecessary marco.
* RAII type manager.
* Multiple threads in thread pool while keeping the message ordering.
* Log filtering in logger and sink.

Additionally:

* Default message for user to choose
  * `std::stringstream`
  * `{fmt}` (require additional repository)

## How to install

### Header only

Copy [include folder](include/logency) and you are good to go.

### CMake

```CMake
add_subdirectory(location/of/the/library)

target_link_libraries(target_name PRIVATE logency)
target_include_directories(target_name PRIVATE ${logency_INCLUDE_DIR})
```

## Documentation

See [Overview](doc/readme.md) for more info.

## License

The whole repository is covered under MIT License.
