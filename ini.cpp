#include <fstream>
#include <sstream>
#include <stdexcept>

// TODO remove 
#include <iostream>
#include "ini.h"



static void 
resetStringStream(std::ostringstream& oss)
{
    oss.str("");  // reset string contents
    oss.clear();  // clear warning flags
}

std::string
ini::ConfigParser::get(const std::string& section,
                       const std::string& key) const
{
    ConfigSection s = getSection(section);
    auto value = s.find(key);
    if (value == s.end())
    {
        std::string msg("Section \"" + section + "\" does not contain key \"" + key + '"');
        throw std::invalid_argument(msg);
    }
    return value->second;
}

bool
ini::ConfigParser::hasSection(const std::string& name) const
{
    auto s = sections.find(name);
    return s != sections.end();
}

ini::ConfigSection
ini::ConfigParser::getSection(const std::string& name) const
{
    auto s = sections.find(name);
    if (s == sections.end())
    {
        std::string msg("Section \"" + name + "\" does not exist.");
        throw std::invalid_argument(msg);
    }
    return s->second;
}

void 
ini::ConfigParser::addSection(const std::string& name)
{
    ConfigSection section;
    sections.try_emplace(name, section);
}

void
ini::ConfigParser::read(const std::string& filename)
{
    std::ifstream fs;
    fs.open(filename);
    if (!fs.is_open())
    {
        std::string msg("Failed to read file: " + filename);
        throw std::runtime_error(msg);
    }

    ini::ParseState state = SEEKING_KEY;
    std::string currentSection = default_section;
    std::string currentKey;
    std::string currentValue;

    std::string line;
    std::ostringstream chars;
    while (std::getline(fs, line))
    {
        if (state == READING_VALUE)
        {
            currentValue = chars.str();
            resetStringStream(chars);
            set(currentSection, currentKey, currentValue);
        }
        state = SEEKING_KEY;

        for (auto c: line)
        {
            if (state != READING_COMMENT)
            {
                switch(c) 
                {
                    case '#':
                        state = READING_COMMENT;
                        break;

                    case ' ':
                    case '\t':
                        if (state == READING_VALUE)
                        {
                            chars << c;
                        }
                        break;

                    case '[':
                        state = READING_SECTION;
                        break;

                    case ']':
                        currentSection = chars.str();
                        resetStringStream(chars);
                        addSection(currentSection);
                        state = READING_KEY;
                        break;

                    case '=':
                    case ':':
                        if (state == READING_KEY)
                        {
                            currentKey = chars.str();
                            resetStringStream(chars);
                            state = SEEKING_VALUE;
                        }
                        break;

                    default:
                        chars << c;
                        if (state == SEEKING_KEY)
                        {
                            state = READING_KEY;
                        }
                        else if (state == SEEKING_VALUE)
                        {
                            state = READING_VALUE;
                        }
                        break;
                }
            }
        }
    }
}

void
ini::ConfigParser::set(const std::string& section, 
                       const std::string& key, 
                       const std::string& value)
{
    if (!hasSection(section))
    {
        addSection(section);
    }
    sections[section].insert_or_assign(key, value);
}

