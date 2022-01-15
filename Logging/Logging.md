# Logging
A simple logging library using a tree structure to organize the loggers.

___
## Table of contents
* [Dependencies](#dependencies)
* [Compilation](#compilation)
* [Usage](#usage)
    * [Log::getLogger()](#loggetlogger)
    * [Log::Logging::getChild()](#loglogginggetchild) 
    * [Log::Logging::setFormat()](#logloggingsetformat)
    * [Log::Logging::setFormatTime()](#logloggingsetformattime)
    * [Log::Logging::setLevel()](#logloggingsetlevel)
    * [Log::Logging::pullProperties()](#logloggingpullproperties)
    * [Log::Logging::pushProperties()](#logloggingpushproperties)
    * [Log::Logging::log()](#loglogginglog)
* [Planned implementations](#plannedimplementations)

___
## Dependencies
The following external library is needed to compile the Logging library.
- [fmt](https://fmt.dev/latest/index.html)

Install it with your package manager of choice and it should work.

___
## Compilation
Add `Logging.cpp` and `Logging.hpp` to your project like and treat it like yourown source files. 

Due to the use of the external librarie `fmt`, a flag should be added to the final linking process. The command should look like that:
```bash
g++ -o target $(pkg-config fmt --libs --cflags) Logging.cpp source.cpp
```
`$(pkg-config fmt --libs --cflags)` can obviously be replaced by its output `-lfmt`.
___
## Usage
Before you start using it, keep in mind the library is defined inside the namespace `Log` and that it is **strongly discouraged** adding `using namespace Log;` to your source file. I hade my reasons to define it inside the namespace.

* [Log::getLogger()](#loggetlogger)
* [Log::Logging::getChild()](#loglogginggetchild) 
* [Log::Logging::setFormat()](#logloggingsetformat)
* [Log::Logging::setFormatTime()](#logloggingsetformattime)
* [Log::Logging::setLevel()](#logloggingsetlevel)
* [Log::Logging::pullProperties()](#logloggingpullproperties)
* [Log::Logging::pushProperties()](#logloggingpushproperties)
* [Log::Logging::log()](#loglogginglog)

___
### Log::getLogger()
> `std::shared_ptr<Log::Logging> Log::getLogger()`\
> `std::shared_ptr<Log::Logging> Log::getLogger(std::string name)`

This function should be used as the main utility of the library. It returns a `shared_ptr<Log::Logging>`. The argument `name` contains the full path of the logger you want to access.
```c++
std::shared_ptr<Log::Logging> logger = Log::getLogger("/mainLogger/childLogger");
```
__Note:__ The `std::shared<Log::Logging>` will be replaced by the keyword `auto` in the following snippets for the readabillity sake.

Assuming that's the library's first use in your code, this snippet will create the loggers `/mainLogger` and `/mainLogger/childLogger` and return the pointer of the latter logger. The newly created loggers are inheriting the properties of their parents.

To access the logger's member, you only need to use the pointer operators.
```c++
auto logger = Log::getLogger("/mainLogger/childLogger");
logger->warning("This is a warning");
(*logger).warning("This is another warning");
```

As you probably have noticed, the loggers paths are accessed by a Unix-stile structure. At the beginning, a root logger is created. You can access it in multiple way.
```c++
// Using full path
auto rootPtr1 = Log::getLogger("/");
// The function returns the root logger by default
auto rootPtr3 = Log::getLogger();
```
__Note:__ This function returns smart pointers. That means they are destroying themself when they are out of scope. You should not deallocate them by yourself.

___
### Log::Logging::getChild()
> `std::shared_ptr<Log::Logging> Log::Logging::getLogger(std::string name)`

This is a member of the `Logging` class.

It is used in a similar way as [Log::getLogger()](#Log::getLogger). The argument `name` now contains the local path to the wanted logger.
```c++
auto mainLogger = Log::getLogger("/mainLogger");
auto childLogger = mainLogger->getChild("childLogger");
```
As for `Log::getLogger()`, the child is created if it does not exist yet.

The argument name also can contain `.` and `..`, they are interpreted the same way as in the Unix filesystem. When you want to access the parent of the root logger, `out_of_range` is thrown.
```c++
auto mainLogger = Log::getLogger("/mainLogger");
std::shared_ptr<Log::Logging> childLogger;
// The following three expressions are leading to the same result
childLogger = mainLogger->getChild("./childLogger");
childLogger = mainLogger->getChild("childLogger");
// Great example of "Just because it is possible doesn't mean you should do it"
childLogger = mainLogger->getChild("./.././././mainLogger/./././././childLogger");
// Get the parent of mainLogger, the root logger
auto root = mainLogger->getChild("..");
// This line throws an out_of_range exception
auto error = root->getChild("..");
```

___
### Log::Logging::setFormat()
> `void Log::Logging::setFormat(std::string format)`

This is a member of the `Logging` class.

Change the format which is embedding the log message.

The default format is 
```c++
"[{time}] [{level}] [{name}]: {msg}\n"
```

| Keyword   | Description                     |
|-----------|---------------------------------|
| `{msg}`   | The actual log message          |
| `{level}` | Log level                       |
| `{time}`  | Time when the log has been send |
| `{name}`  | Full name of current logger     |

__Notes:__ 
* The time format is changed with help of [Log::Logging::setFormatTime()](#Log::Logging::setFormatTime()).
* The format is only changed to the specific object. To apply it to the object's children, use [Log::Logging::pushProperties()](#Log::Logging::pushProperties()).

___
### Log::Logging::setFormatTime()
> `void Log::Logging::setFormatTime(std::string format)`

This is a member of the `Logging` class.

The default format is 
```c++
"{:%d-%m-%Y %H:%M:%S}"
```
The leading `:` is needed by the `fmt` library to parse the string correctly. 

Further informations about how to format the time string can be found here [std::put_time](https://en.cppreference.com/w/cpp/io/manip/put_time).

### Log::Logging::setLevel()
> `void Log::Logging::setevel(int level)`

The argument `level` contains the minimum level required for the log message to be displayed. It can either be a plain int or one of the constants listed below.

The default level is `Log::WARNING`.

| Constant         | Level |
|------------------|:-----:|
| `Log::NOTSET`    | 0     |
| `Log::DEBUG`     | 1     |
| `Log::INFO`      | 2     |
| `Log::WARNING`   | 3     |
| `Log::ERROR`     | 4     |
| `Log::CRITICAL`  | 5     |
| `Log::MAX_LEVEL` | 6     |

__Notes:__ 
* An `out_of_range` exception is thrown if the argument is not listed in the list above.
* `Log::MAX_LEVEL` should only be used if __no__ logs should be displayed. We all like to make programs crash without knowing what actually happened.

___
### Log::Logging::pullProperties()
> `void Log::Logging::pullProperties()`

This is a member of the `Logging` class.

Pull parent's properties

```c++
auto mainLogger = Log::getLogger("/mainLogger");
auto childLogger = Log::getLogger("/mainLogger/childLogger");

mainLogger->setLevel(Log::INFO);
// This expression should not display a log message
childLogger->log(Log::INFO, "This is an information");

childLgger->pushProperties();
// Now, this expression should display a log message
childLogger->log(Log::INFO, "This is an information");
```

___
### Log::Logging::pushProperties()
> `void Log::Logging::pushProperties()`

This is a member of the `Logging` class.

Push own properties to every child recursively.
```c++
auto root = Log::getLogger();
auto logger = Log::getLogger("/mainLogger/childLogger");

root->setLevel(Log::INFO);
// This expression should not display a log message
logger->log(Log::INFO, "This is an information");

root->pushProperties();
// Now, this expression should display a log message
logger->log(Log::INFO, "This is an information");
```

___
### Log::Logging::log()
> `void Log::Logging::log(int level, std::string msg)`\
> `void Log::Logging::debug(std::string msg)`\
> `void Log::Logging::info(std::string msg)`\
> `void Log::Logging::warning(std::string msg)`\
> `void Log::Logging::error(std::string msg)`\
> `void Log::Logging::critical(std::string msg)`

This is a member of the `Logging` class.

What was the library supposed to do again? Oh right, logging stuff.

This function displays the actual log message `msg` with a priority of `level`.
The argument `level` can contain any number listed [here](#Log::Logging::setLevel()) except of `Log::MAX_LEVEL`.
`out_of_range` exception is thrown when level is out of range.

The `Logging` class also contains some additional log functions.
```c++
auto logger = Log::getLogger();
logger->log(Log::WARNING, "A warning");
logger->warning("Another warning");
```

___
## Planned Implementations
- Log functions can take any types
- Saving logs into a file 