#ifndef CONFIGPARSER_hpp
# define CONFIGPARSER_hpp

#include "../header/class_header.hpp"
#include "ServerBlock.hpp"
//#include "LocationBlock.hpp"

class ServerBlock;
class LocationBlock;
class ConfigParser {
    private:
        std::string _filename;
        std::vector<ServerBlock> _servers;
        std::vector<std::string> _content;
        bool readFile();
        bool parseContent();
        bool parseServerBlock(size_t& line_num);
        std::vector<std::string> tokenize(const std::string& line);
        std::string trimWhitespace(const std::string& str);
    
    public:
        ConfigParser(const std::string& filename);
        ~ConfigParser();

        const std::vector<ServerBlock>& getServers() const {
            return _servers;
        }

        bool parse();
};


#endif
