/******************************************************************************
* Name:     ini.h
* Purpose:  A basic INI configuration file parser
* Author:   Zachary Colbert
*
* Description:
*   Contains declarations for the ConfigParser class.
*   ConfigParser reads a file in INI format, and stores the key-value
*   pairs organized into sections.
*******************************************************************************/


#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>


namespace ini 
{
    enum ParseState
    {
        READING_COMMENT,
        READING_SECTION,
        SEEKING_KEY,
        READING_KEY,
        SEEKING_VALUE,
        READING_VALUE
    };

    using StringPair = std::pair<std::string, std::string>;
    using ConfigSection = std::unordered_map<std::string, std::string>;

    class ConfigParser
    {
        public:
            std::string default_section;

            explicit ConfigParser();

            void defineBoolean(const std::string& key, bool value);
            void undefineBoolean(const std::string &key);

            std::string get(const std::string& section, const std::string& key) const;
            bool getBool(const std::string& section, const std::string& key) const;
            int getInt(const std::string& section, const std::string& key) const;

            bool hasSection(const std::string& name) const;
            ConfigSection getSection(const std::string& name) const;
            void addSection(const std::string& name);

            void read(const std::string& filename);
            void read(std::ifstream& fs);

            void set(const std::string& section, const std::string& key, const std::string& value);

        private:
            std::unordered_map<std::string, ConfigSection> section_map;
            std::unordered_set<std::string> true_values;
            std::unordered_set<std::string> false_values;
    };
}
