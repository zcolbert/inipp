# inipp

A simple class for reading config files in INI format where:

Sections are designated in the format [SECTION]
Key-value pairs are located within each section header. 
Values may be delimited by an equal sign or a colon.

The following boolean values are supported by default: (yes/no), (true/false), (on/off), (1/0)

Example:
```ini
    [GENERAL]
    name = inipp
    purpose = A basic config parser
    
    # single line comments begin with '#'
    [SECTION]
    key: value
    string_key: Strings are not enclosed in quotes
    
```
