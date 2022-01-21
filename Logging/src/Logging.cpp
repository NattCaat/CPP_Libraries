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

#include "../include/Logging.hpp"
using namespace std;


/* ____________________________________________________________
 CLASS Logging METHODS
____________________________________________________________ */

// Constructor definition
Log::Logging::Logging(string fullName)
{
    Log::Logging::_level = Log::WARNING;
    Log::Logging::_format = "[{time}] [{level}] [{name}]: {msg}\n";
    Log::Logging::_formatTime = "{:%d-%m-%Y %H:%M:%S}";
    Log::Logging::_fullName = fullName;
    Log::Logging::_name = fullName.substr(fullName.rfind("/")+1);
    Log::Logging::_parent = nullptr;
}

// Create a new child logger and copy parent properties
void Log::Logging::createChild(std::string name)
{
    // Display warning if name is already in unordered map
    if (Log::Logging::_children.find(name) != Log::Logging::_children.end()) {
        Log::Logging::warning("Logger: Overwriting existing logger!");
    }
    // Create child and inherit properties from parent
    shared_ptr<Log::Logging> tmpLog(new Log::Logging(Log::Logging::_fullName+"/"+name));
    tmpLog->setParent(this);
    tmpLog->pullProperties();
    Log::Logging::_children[name] = tmpLog;
}

// Remove child loggers who are not referred to recursively
void Log::Logging::clean()
{
    vector<string> deadChildrenKeys;
    // Check if any children should be removed
    // Flagging is done by adding their key to "deadChildrenKeys"
    auto it = Log::Logging::_children.begin();
    while (it != Log::Logging::_children.end()) {
        it->second->clean();
        // Flag child if only parent is pointing to child and child does not have children
        if (it->second.use_count() < 2 && it->second->getChildrenCnt() == 0) {
            deadChildrenKeys.push_back(it->first);
        }
        it++;
    }
    // Remove flagged children
    for (auto key : deadChildrenKeys) {
        Log::Logging::_children.erase(key);
    }
}

// Get properties from parent logger
void Log::Logging::pullProperties()
{
    Log::Logging::setLevel(Log::Logging::_parent->getLevel());
    Log::Logging::setFormat(Log::Logging::_parent->getFormat());
    Log::Logging::setFormatTime(Log::Logging::_parent->getFormatTime());
}

// Push own properties recursively to children
void Log::Logging::pushProperties()
{
    // Type is unordered_map<string, shared_ptr<Log::Logging>>::iterator
    auto it = Log::Logging::_children.begin();
    // Make every child pull their parent's properties and push them to their children
    while (it != Log::Logging::_children.end()) {
        it->second->pullProperties();
        it->second->pushProperties();
        it++;
    }
}

// Set the log messages' minimum level required to be displayed
// Throw out_of_range error if log level does not exist
void Log::Logging::setLevel(int level)
{
    if (level < 0 || level > Log::MAX_LEVEL) {
        throw out_of_range("Invalid log level");
    }
    Log::Logging::_level = level;
}

// Set _format and clean argument format 
void Log::Logging::setFormat(string format)
{
    //* Hardcoding these format arguments here is probably a horrible idea
    //* But that is something I need to worry in the future, if I didn't ditch this logger
    const string formatArgs[] = {
        "{open}", "{close}", "{name}", "{time}", "{level}", "{msg}"
    };
    string tmpFormat;
    size_t found;
    vector<size_t> brackets;

    // Get every bracket belonging to a format argument group
    // Store their position into vector brackets
    for (size_t i = 0; i < sizeof(formatArgs)/sizeof(*formatArgs); i++) {
        tmpFormat = format;
        found = tmpFormat.rfind(formatArgs[i]);
        while (found != tmpFormat.npos) {
            brackets.push_back(found);
            brackets.push_back(found+formatArgs[i].size()-1);
            tmpFormat = tmpFormat.substr(0, found);
            found = tmpFormat.rfind(formatArgs[i]);
        }
    }

    // Replace every bracket not belonging to a format argument group
    for (int i = format.size()-1; i > -1; i--) {
        if (find(brackets.begin(), brackets.end(), i) != brackets.end()) {
            continue;
        }
        if (format[i] == '{') {
            format.replace(i, 1, "{open}");
        } else if (format[i] == '}') {
            format.replace(i, 1, "{close}");
        }
    }
    Log::Logging::_format = format;
}

// Set _formatTime and clean argument formatTime
void Log::Logging::setFormatTime(string formatTime)
{
    int pairPos[2] = {0, 0};
    int currNestLevel = 0;
    int pairNestLevel = -1;

    // Search bracket pair announcing the format
    // Store their position into int array pairPos
    for (size_t i = 0; i < formatTime.size(); i++) {
        if (formatTime[i] == '{') {
            if (formatTime[i+1] == ':') {
                pairPos[0] = i;
                pairNestLevel = currNestLevel;
            }
            currNestLevel++;
        } else if (formatTime[i] == '}') {
            currNestLevel--;
            if (pairNestLevel == currNestLevel) {
                pairPos[1] = i;
                break;
            }
        }
    }
    // Add missing brackets
    if (pairNestLevel == -1) {
        formatTime.insert(0, "{:");
    }
    if (pairPos[1] == 0) {
        pairPos[1] = formatTime.size();
        formatTime.append("}");
    }

    // Replace every bracket not belonging to a format argument group
    for (int i = formatTime.size()-1; i > -1; i--) {
        if (i == pairPos[0] || i == pairPos[1]) {
            continue;
        }
        if (formatTime[i] == '{' || formatTime[i] == '}') {
            formatTime.erase(i, 1);
        }
    }
    Log::Logging::_formatTime = formatTime;
}

// Link a parent to logger
void Log::Logging::setParent(Log::Logging *parent)
{
    // Only set parent if this object does not has a parent and is not root 
    if (Log::Logging::_parent == nullptr && Log::Logging::_fullName != "") {
        Log::Logging::_parent = parent;
    }
}

// Return pointer of child
// name contains local path to child from current object
shared_ptr<Log::Logging> Log::Logging::getChild(string name)
{

    // User wants to access parent
    if (name.substr(0, 2) == "..") {
        name.erase(0, 2);
        (name[0] == '/') && (name == name.erase(0, 1));
        // Check if this logger has a parent
        // Throw out_of_range on failure
        if (_parent == nullptr) {
            throw out_of_range("You cannot get the parent of a root node");
        }
        // Jump to parent logger to get wanted child
        return Log::Logging::_parent->getChild(name);
    }
    // Remove leading chars telling it is a local path, they are redundant
    while (name.substr(0, 2) == "./") {
        name.erase(0, 2);
    }
    // Return yourself when name is empty
    if (name == "" || name == ".") {
        return shared_from_this();
    }
    // Get first position of delimiter
    int delimiter = name.find("/");
    // If no delimiter, you are the parent
    // return child and create one if it does not exist
    if (delimiter == -1) {
        if (Log::Logging::_children.find(name) == Log::Logging::_children.end()) {
            Log::Logging::createChild(name);
        }
        return Log::Logging::_children[name];
    }
    // If delimiter, you are a grandparent (or even greater)
    // call your child to return wanted child
    shared_ptr<Log::Logging> tmpLog(
        Log::Logging::getChild(name.substr(0, delimiter))
    );
    return tmpLog->getChild(name.substr(delimiter+1));
}

// Display a log message
// Throw out_of_range error if log level does not exist
void Log::Logging::log(int level, string msg)
{
    // Check if level is valid 
    if (level < 0 || level > MAX_LEVEL-1) {
        throw out_of_range("Invalid log level");
    } else if (level < Log::Logging::_level) {
        return;
    }
    // Format and print the log message
    fmt::print(
        Log::Logging::_format, 
        fmt::arg("open", "{"),
        fmt::arg("close", "}"),
        fmt::arg("name", 
            (Log::Logging::_fullName == "") ? "/" : Log::Logging::_fullName
        ),
        fmt::arg(
            "time", 
            fmt::format(
                Log::Logging::_formatTime,
                fmt::localtime(time(nullptr))
            )
        ),
        fmt::arg(
            "level", 
            fmt::format(
                "{}{}{}",
                Log::COLORS[level], 
                Log::LEVELS[level], 
                Log::RESET
            )
        ),
        fmt::arg("msg", msg)
    );
}


/* ____________________________________________________________
 CLASS UTILITIES
____________________________________________________________ */

// Root of the internal logging tree
shared_ptr<Log::Logging> loggingTree(new Log::Logging());

// This is the main utility for accessing and creating instances of Logging objects
// Return pointer of wanted Logging object
shared_ptr<Log::Logging> Log::getLogger(string name)
{
    // Remove leading slash
    return loggingTree->getChild(name);
}