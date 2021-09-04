#include <cctype>     // std::tolower(), std::toupper()
#include <fstream>
#include <stdexcept>
#include <sstream>

#include "ini.h"


static void resetStringStream(std::ostringstream& oss)
/*  Reset a string stream and clear it to its initial state */
{
    oss.str("");  // reset string contents
    oss.clear();  // clear warning flags
}

static std::string lowerCase(const std::string& orig)
/*  Return a lowercase copy of the original string */
{
    std::ostringstream oss;
    for (auto c: orig) {
        oss << static_cast<char>(tolower(static_cast<unsigned char>(c)));
    }
    return oss.str();
}

static std::string upperCase(const std::string& orig)
/*  Return an uppercase copy of the original string */
{
    std::ostringstream oss;
    for (auto c: orig) {
        oss << static_cast<char>(toupper(static_cast<unsigned char>(c)));
    }
    return oss.str();
}

ini::ConfigParser::ConfigParser() :
    default_section("default"),
    section_map(),
    true_values({"true", "yes", "1", "on"}),
    false_values({"false", "no", "0", "off"})
{}

void ini::ConfigParser::defineBoolean(const std::string& key, bool value)
/*  Define a new key that evaluatees to a boolean value */
{
    if (value) {
        true_values.insert(key);
    }
    else {
        false_values.insert(key);
    }
}

void ini::ConfigParser::undefineBoolean(const std::string& key)
/*  Undefine a key that evaluatees to a boolean value */
{
    true_values.erase(key);
    false_values.erase(key);
}

std::string ini::ConfigParser::get(const std::string& section, const std::string& key) const
/*  Retrieve the value of a key that exists in a given section */
{
    ConfigSection s = getSection(lowerCase(section));
    auto value = s.find(lowerCase(key));
    if (value == s.end())
    {
        std::string msg("Section \"" + section + "\" does not contain key \"" + key + '"');
        throw std::invalid_argument(msg);
    }
    return value->second;
}

bool ini::ConfigParser::getBool(const std::string& section, const std::string& key) const
/* 
    Retrieve the value of a key that exists in a given section, and attempt to 
    interpret the value as a boolean. If the key has not been defined as a boolean,
    a std::invalid_argument exception is thrown.
*/
{
    std::string value = lowerCase(get(section, key));

    if (true_values.find(value) != true_values.end()) {
        return true;
    }
    else if (false_values.find(value) != false_values.end()) {
        return false;
    }
    else {
        std::string msg = "Error reading [" + section + "]/" + "\"" + key\
                           + "\": value \"" + value + "\" cannot be "\
                           "interpreted as boolean.";
        throw std::invalid_argument(msg);
    }
}

int ini::ConfigParser::getInt(const std::string& section, const std::string& key) const
/*  Retrieve the value of a key from a given section, and attempt to interpret it as an int */
{
    std::string value = get(section, key);
    return stoi(value);
}

bool ini::ConfigParser::hasSection(const std::string& name) const
/*  Return true if the section exists in the config map */
{
    auto s = section_map.find(name);
    return s != section_map.end();
}

ini::ConfigSection ini::ConfigParser::getSection(const std::string& name) const
/* 
    Return a ConfigSection containing the key value pairs of the specified section.
    If the section does not exist, a std::invalid_argument exception is thrown.
*/
{
    auto s = section_map.find(name);
    if (s == section_map.end())
    {
        std::string msg("Section \"" + name + "\" does not exist.");
        throw std::invalid_argument(msg);
    }
    return s->second;
}

void ini::ConfigParser::addSection(const std::string& name)
/*  Add a new section to the config, initially containing no key-value pairs. */
{
    ConfigSection section;
    section_map.try_emplace(name, section);
}

void ini::ConfigParser::read(const std::string& filename)
/*  Populate the config from a file structured in INI format */
{
    std::ifstream fs;

    fs.open(filename);
    if (!fs.is_open())
    {
        std::string msg("Failed to read file: " + filename);
        throw std::runtime_error(msg);
    }
    read(fs);
}

void ini::ConfigParser::read(std::ifstream& fs)
/*  Populate the config from a file structured in INI format */
{
    ini::ParseState state = SEEKING_KEY;
    std::string currentSection = default_section;
    std::string currentKey;
    std::string currentValue;

    std::string line;
    std::ostringstream chars;
    while (std::getline(fs, line))
	{
        state = SEEKING_KEY;

        for (auto c: line)
        {
            if (state == READING_COMMENT) {
                break;
            }
            else if (state == READING_VALUE) {
                chars << c;
            }
            else  {
                // Update the parsing state based on the current character
                switch(c) 
                {
                    case '#':
                        state = READING_COMMENT;
                        break;

                    case ' ':
                    case '\t':
                        // Ignore whitespace
                        break;

                    case '[':
                        // Indicates beginning of a new section
                        state = READING_SECTION;
                        break;

                    case ']':
                        // Indicates end of a section
                        if (state != READING_SECTION)
                        {
                            std::string msg("Parse error: Encountered ']'"\
                                            " but was not reading section.");
                            throw std::runtime_error(msg);
                        }
                        // Add the new section to the config map
                        currentSection = chars.str();
                        resetStringStream(chars);
                        addSection(currentSection);
                        break;

                    case '=':
                    case ':':
                        // Indicates start of value string if currently reading key
                        // Ignored if reading a value (delimiter may exist in an arbitrary string value)
                        if (state == READING_KEY)
                        {
                            currentKey = chars.str();
                            resetStringStream(chars);
                            state = SEEKING_VALUE;
                        }
                        break;

                    default:
                        // Add the current character to the character buffer
                        chars << c;
                        if (state == SEEKING_KEY) {
                            state = READING_KEY;
                        }
                        else if (state == SEEKING_VALUE) {
                            state = READING_VALUE;
                        }
                        break;
                }
            }
        }
        // Add the contents of the character buffer to the current section
        if (state == READING_VALUE)
        {
            currentValue = chars.str();
            resetStringStream(chars);

            set(lowerCase(currentSection), 
                lowerCase(currentKey), 
                currentValue);
        }
	} 
}

void ini::ConfigParser::set(const std::string& section, const std::string& key, const std::string& value)
/*
    Set the value of a given key in the specified section. The section is created if it doesn't exist. 
    If the key exists its value will be replaced. Otherwise, the new key is added with the initial value.
*/
{
    if (!hasSection(section)) {
        addSection(section);
    }
    section_map[section].insert_or_assign(key, value);
}

