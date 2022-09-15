# Custom message

One of the main purpose of the library is to create your own custom message for logging.

Although the library do provide some predefined structure in [`include/logency/message`](../include/logency/message/) that you might have interested, You can try to create your own message.

What content does it give? How the variable is arranged? What type does it provide? Defined your message accordingly.

After that, pass it to the template variable and you are good to go.

```c++
struct my_message
{
    using value_type = char;
    using traits_type = std::char_traits<char>;
    using string_type = std::string;
    using string_view_type = std::string_view;

    std::string message;
};

logency::manager<my_message> manager;
```

---

## Message Structure

### Content

Message determined what content you wish to log. You can add what you need, and discard what you don't.

If you don't know how to start, here are the list that typically appeared in the message:

* Logger name: Usually determine where the message takes place.
* Level: Usually indicate how important the message is.
* Time: Usually recorded when does the message appeared.
* Message: Usually tell the user what happened.

**Logger name is already recorded when the message is passing around.** The rest content can be determined by the user.

> e.g.
>
> ```c++
> std::string content;
> std::chrono::system_clock::time_point time;
> int level;
> ```

### Type alias

In addition, in order to let the library know what content you wish to use, you have to define proper type alias in the message structure.

Here are the necessary type alias for manager (logger, sink, etc.):

* `value_type`: Basic underlying type of the message. Determine what message structure when the message get formatted, outputted, etc.
* `string_type`: Basic buffer type of the message. Usually indicate a sequence of `value_type` object in the memory storage.
* `string_view_type`: Basic buffer view type of the message. It is used to refer to the location of a sequence of `value_type` object. It is not responsible to store value inside the memory.

**These are the essential alias that will be used in the manager. Define them properly.**

Some type alias are requested in `sink_module`. You need to defined them in order to use them properly.

Here are the necessary type alias for `sink_module`:

* `traits_type`: Basic traits type of the message. Usually indicate the traits of `value_type` when manipulating them.

> e.g.
>
> ```c++
> using value_type = char;
> using traits_type = std::char_traits<char>;
> using string_type = std::basic_string<value_type, traits_type>; // or std::string
> using string_view_type = std::basic_string_view<value_type, traits_type>; // or std::string_view
> ```

### Constructor

This library construct your message by using constructors.

It will pass the arguments in `logger::log()` to the constructor. Define your constructors accordingly.

> e.g.
>
> ```c++
> // my_logger->log(0, "this is my message");
> explicit constructor(int level_arg, string_view_type content_arg)
>     : content{content_arg}, time{std::chrono::system_clock::now()}, level{level_arg}
> {
> }
> ```
>
> ```c++
> // my_logger->log(0, "using", "std::", "stringstream");
> template<typename... Args>
> explicit std_stringstream_constructor(int level_arg, Args &&...args)
>     : time{std::chrono::system_clock::now()}, level{level_arg}
> {
>     stringstream_type stream;
>
>     (stream << ... << std::forward<Args>(args));
>
>     content = stream.str();
> }
> ```
>
>```c++
> // my_logger->log(0, "using", "std", "format");
> template<typename... Args>
> explicit std_format_constructor(int level_arg, string_view_type fmt, Args &&...args)
>     : content{std::format(fmt, std::forward<Args>(args)...)}, time{std::chrono::system_clock::now()}, level{level_arg}
> {
> }
> ```

### Create your message

When everything is finished, your structure should look like this:

```c++
struct my_message
{
    using value_type = char;
    using traits_type = std::char_traits<char>;
    using string_type = std::basic_string<value_type, traits_type>;
    using string_view_type = std::basic_string_view<value_type, traits_type>;

    explicit my_message(int level_arg, string_view content_arg)
        : content{content_arg}, time{std::chrono::system_clock::now()}, level{level_arg}
    {
    }

    std::string content;
    std::chrono::system_clock::time_point time;
    int level;
};
```

---

## Formatter

Formatter format your message into meaningful result. It takes logger name and the message object as input, produce requested output as result.

**Each `sink_module` should have one and only one formatter.**

Normally, the formatter code should look like this:

```c++
class formatter
{
public:
    auto operator()(string_view_type logger, const message &message) const -> output_type;
};
```

* It use `operator()` to format the message.
* Input should be `string_view_type, const message &` and **cannot** be reordered.
* Output type should be recognisable for sink module. Each sink module have its own requirement, so define them accordingly.

  > e.g.
  >
  > * `console_module`: `typename message_type::string_type`;
  > * `color_console_module`: `std::vector<color_message<value_type, traits_type>>`;

Library predefine structure do provide default formatter for you to use. Or you can do it in your own taste.

> e.g.
>
> ```c++
> class formatter
> {
> public:
>     auto operator()(string_view_type logger, const my_message &message) const -> std::string
>     {
>         stringstream_type stream;
>
>         stream << logger << ": " <<message.content;
>
>         return stream.str();
>     }
> };
> ```
