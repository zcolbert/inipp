#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cctype>     // std::tolower(), std::toupper()
#include "ini.h"


static void 
resetStringStream(std::ostringstream& oss)
{
    oss.str("");  // reset string contents
    oss.clear();  // clear warning flags
}

static std::string
lowerCase(const std::string& orig)
{
    std::ostringstream oss;
    for (auto c: orig)
    {
        oss << static_cast<char>(tolower(static_cast<unsigned char>(c)));
    }
    return oss.str();
}

static std::string
upperCase(const std::string& orig)
{
    std::ostringstream oss;
    for (auto c: orig)
    {
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

std::string
ini::ConfigParser::get(const std::string& section,
                       const std::string& key) const
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

bool 
ini::ConfigParser::getBool(const std::string& section,
                           const std::string& key) const
{
    std::string value = lowerCase(get(section, key));
    if (true_values.find(value) != true_values.end())
    {
        return true;
    }
    else if (false_values.find(value) != false_values.end())
    {
        return false;
    }
    else
    {
        std::string msg = "Error reading [" + section + "]/" + "\"" + key\
                           + "\": value \"" + value + "\" cannot be "\
                           "interpreted as boolean.";
        throw std::invalid_argument(msg);
    }
}

int
ini::ConfigParser::getInt(const std::string& section,
       const std::string& key) const
{
    std::string value = get(section, key);
    return stoi(value);
}

bool
ini::ConfigParser::hasSection(const std::string& name) const
{
    auto s = section_map.find(name);
    return s != section_map.end();
}

ini::ConfigSection
ini::ConfigParser::getSection(const std::string& name) const
{
    auto s = section_map.find(name);
    if (s == section_map.end())
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
    section_map.try_emplace(name, section);
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
    read(fs);
}

void
ini::ConfigParser::read(std::ifstream& fs)
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
            if (state == READING_COMMENT)
            {
                break;
            }
            else if (state == READING_VALUE)
            {
                chars << c;
            }
            else 
            {
                switch(c) 
                {
                    case '#':
                        state = READING_COMMENT;
                        break;

                    case ' ':
                    case '\t':
                        break;

                    case '[':
                        state = READING_SECTION;
                        break;

                    case ']':
                        if (state != READING_SECTION)
                        {
                            std::string msg("Parse error: Encountered ']'"\
                                            " but was not reading section.");
                            throw std::runtime_error(msg);
                        }
                        currentSection = chars.str();
                        resetStringStream(chars);
                        addSection(currentSection);
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

void
ini::ConfigParser::set(const std::string& section, 
                       const std::string& key, 
                       const std::string& value)
{
    if (!hasSection(section))
    {
        addSection(section);
    }
    section_map[section].insert_or_assign(key, value);
}

