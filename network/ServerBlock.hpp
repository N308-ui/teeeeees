#ifndef SERVERBLOCK_HPP
# define SERVERBLOCK_HPP

#include "../header/class_header.hpp"
#include "LocationBlock.hpp"

class ServerBlock {
    private:
        std::string _host;
        int _port;
        std::vector<std::string> _server_names;
        std::string _root;
        std::vector<std::string> _index_files;
        std::map<int, std::string> _error_pages;
        size_t _client_max_body_size;
        std::vector<LocationBlock> _locations;
    public:
        ServerBlock();
        ~ServerBlock();

        ServerBlock(const ServerBlock& other);

        ServerBlock& operator=(const ServerBlock& other);

        void setHost(const std::string& host);
        void setPort(int port);
        void addServerName(const std::string& name);
        void setRoot(const std::string& root);
        void addIndexFile(const std::string& index);
        void addErrorPage(int code, const std::string& page);
        void setClientMaxBodySize(size_t size);
        void addLocation(const LocationBlock& location);

        const std::string& getHost() const;
        int getPort() const;
        const std::vector<std::string>& getServerNames() const;
        const std::string& getRoot() const;
        const std::vector<std::string>& getIndexFiles() const;
        const std::map<int, std::string>& getErrorPages() const;
        size_t getClientMaxBodySize() const;
        const std::vector<LocationBlock>& getLocations() const;
};

#endif
