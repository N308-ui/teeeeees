#include "ServerBlock.hpp"
#include "LocationBlock.hpp"
ServerBlock::ServerBlock() 
            : _port(-1), _client_max_body_size(1024 * 1024 * 1024) {}

ServerBlock::~ServerBlock() {}

ServerBlock::ServerBlock(const ServerBlock& other) 
    : _host(other._host),
      _port(other._port),
      _server_names(other._server_names),
      _root(other._root),
      _index_files(other._index_files),
      _error_pages(other._error_pages),
      _client_max_body_size(other._client_max_body_size),
      _locations(other._locations) {
}

ServerBlock& ServerBlock::operator=(const ServerBlock& other) {
    if (this != &other) {
        _host = other._host;
        _port = other._port;
        _server_names = other._server_names;
        _root = other._root;
        _index_files = other._index_files;
        _error_pages = other._error_pages;
        _client_max_body_size = other._client_max_body_size;
        _locations = other._locations;
    }
    return *this;
}

void ServerBlock::setHost(const std::string& host) {
    _host = host;
}

void ServerBlock::setPort(int port) {
    _port = port;
}

void ServerBlock::addServerName(const std::string& name) {
    _server_names.push_back(name); 
}

void ServerBlock::setRoot(const std::string& root) {
    _root = root;
}

void ServerBlock::addIndexFile(const std::string& index) {
    _index_files.push_back(index); 
}

void ServerBlock::addErrorPage(int code, const std::string& page) {
    _error_pages[code] = page;
}

void ServerBlock::setClientMaxBodySize(size_t size) {
    _client_max_body_size = size;
}

void ServerBlock::addLocation(const LocationBlock& location) {
  _locations.push_back(location);
}


const std::string& ServerBlock::getHost() const {
    return _host; 
}

int ServerBlock::getPort() const { 
    return _port; 
}

const std::vector<std::string>& ServerBlock::getServerNames() const {
    return _server_names; 
}

const std::string& ServerBlock::getRoot() const {
    return _root;
}

const std::vector<std::string>& ServerBlock::getIndexFiles() const {
    return _index_files;
}

const std::map<int, std::string>& ServerBlock::getErrorPages() const {
    return _error_pages;
}

size_t ServerBlock::getClientMaxBodySize() const {
    return _client_max_body_size;
}

const std::vector<LocationBlock>& ServerBlock::getLocations() const {
    return _locations;
}
