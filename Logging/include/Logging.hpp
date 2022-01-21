/*
    Logger library written by NattCaat
    Copyright (C) 2022  NattCaat

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

    You can reach the me by the email address nattcaat@yahoo.com
*/

#ifndef LOGGING_H_
#define LOGGING_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <fmt/core.h>
#include <fmt/chrono.h>

namespace Log
{
    // Enumerate log levels
    enum LEVEL {
        NOTSET,
        DEBUG,
        INFO,
        WARNING,
        ERROR,
        CRITICAL,
        MAX_LEVEL
    };

    const std::string LEVELS[] = {
        "", "DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL"
    };

    // Ansi color codes 
    const std::string RESET = "\033[0m";
    const std::string WHITE = "\033[37m";
    const std::string GREEN = "\033[32m";
    const std::string BLUE = "\033[34m";
    const std::string YELLOW = "\033[33m";
    const std::string RED = "\033[31m";
    const std::string MAGENTA ="\033[35m";
    const std::string COLORS[] = {
        WHITE, GREEN, BLUE, YELLOW, RED, MAGENTA
    };


    class Logging : public std::enable_shared_from_this<Logging>
    {
        private:
            int _level;
            std::string _format;
            std::string _formatTime;
            std::string _name;
            std::string _fullName;
            Logging *_parent;
            std::unordered_map<std::string, std::shared_ptr<Logging>> _children;
        public:
            // Constructors with overloads
            Logging() 
                : Logging("") {};
            Logging(std::string fullName);

            void createChild(std::string name);
            void clean();
            void pullProperties();
            void pushProperties();

            // Setters
            void setLevel(int level);
            void setFormat(std::string format);
            void setFormatTime(std::string formatTime);
            void setParent(Logging *parent);

            // Getters
            std::string getName()
            { return _name; };
            std::string getFullName()
            { return _fullName; };
            int getLevel()
            { return _level; };
            std::string getFormat()
            { return _format; };
            std::string getFormatTime()
            { return _formatTime; };
            int getChildrenCnt()
            { return _children.size(); };
            std::shared_ptr<Logging> getChild(std::string name);
            std::shared_ptr<Logging> operator[](std::string name)
            { return getChild(name); };

            // Methods to actually log messages
            void log(int level, std::string msg);
            void debug(std::string msg) 
            { log(DEBUG, msg); };
            void info(std::string msg)
            { log(INFO, msg); };
            void warning(std::string msg)
            { log(WARNING, msg); };
            void error(std::string msg)
            { log(ERROR, msg); };
            void critical(std::string msg)
            { log(CRITICAL, msg); };
    };

    std::shared_ptr<Logging> getLogger(std::string name="");
}

#endif