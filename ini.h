#pragma once

#include <string>
#include <unordered_map>
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
            explicit ConfigParser() = default;

            std::string
            get(const std::string& section,
                const std::string& key) const;

            bool
            hasSection(const std::string& name) const;

            ConfigSection
            getSection(const std::string& name) const;

            void 
            addSection(const std::string& name);

            void
            read(const std::string& filename);

            void
            set(const std::string& section, 
                const std::string& key, 
                const std::string& value);

        private:
            std::unordered_map<std::string, ConfigSection> sections;
            std::string default_section = "default";
    };
}
