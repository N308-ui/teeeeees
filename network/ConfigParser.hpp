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
        bool parseLocationBlock(size_t& line_num, LocationBlock& location);
        std::vector<std::string> tokenize(const std::string& line);
        std::string trimWhitespace(const std::string& str);


        bool isValidCgiPath(const std::string& path) const;
        bool isValidLocationPath(const std::string& path) const;
        bool hasGarbageAfterSemicolon(const std::vector<std::string>& tokens) const;
    
    public:
        ConfigParser(const std::string& filename);
        ~ConfigParser();

        const std::vector<ServerBlock>& getServers() const {
            return _servers;
        }

        bool parse();
};


#endif
