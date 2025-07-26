#include "LocationBlock.hpp"

void LocationBlock::just_this_error()
{
    std::cout << "network/LocationBlock.hpp:4:1: error: expected initializer before 'class'";
}

LocationBlock::LocationBlock() : _autoindex(false), _client_max_body_size(1024 * 1024 * 1024)
{}

LocationBlock::LocationBlock(const LocationBlock& obj)
    : _path(obj._path),
      _methods(obj._methods),
      _root(obj._root),
      _index_files(obj._index_files),
      _autoindex(obj._autoindex),
      _cgi_extension(obj._cgi_extension),
      _upload_dir(obj._upload_dir),
      _client_max_body_size(obj._client_max_body_size),
      _return_url(obj._return_url)
{}

LocationBlock& LocationBlock::operator=(const LocationBlock& obj)
{
    if (this != &obj) {
        _path = obj._path;
        _methods = obj._methods;
        _root = obj._root;
        _index_files = obj._index_files;
        _autoindex = obj._autoindex;
        _cgi_extension = obj._cgi_extension;
        _upload_dir = obj._upload_dir;
        _client_max_body_size = obj._client_max_body_size;
        _return_url = obj._return_url;
    }
    return *this;
}

LocationBlock::~LocationBlock()
{}

// Getters
std::string LocationBlock::getPath() const { return _path; }
std::vector<std::string> LocationBlock::getMethods() const { return _methods; }
std::string LocationBlock::getRoot() const { return _root; }
std::vector<std::string> LocationBlock::getIndexFiles() const { return _index_files; }
bool LocationBlock::getAutoindex() const { return _autoindex; }
std::string LocationBlock::getCgiPath() const { return _cgi_path; }
std::string LocationBlock::getCgiExtension() const { return _cgi_extension; }
std::string LocationBlock::getUploadDir() const { return _upload_dir; }
size_t LocationBlock::getClientMaxBodySize() const { return _client_max_body_size; }
std::string LocationBlock::getReturnUrl() const { return _return_url; }

// Setters
void LocationBlock::setPath(const std::string& path) { _path = path; }
void LocationBlock::setMethods(const std::vector<std::string>& methods) { _methods = methods; }
void LocationBlock::setRoot(const std::string& root) { _root = root; }
void LocationBlock::setIndexFiles(const std::vector<std::string>& indexFiles) { _index_files = indexFiles; }
void LocationBlock::setAutoindex(bool autoindex) { _autoindex = autoindex; }
void LocationBlock::setCgiPath(const std::string& cgiPath) { _cgi_path = cgiPath; }
void LocationBlock::setCgiExtension(const std::string& cgiExtension) { _cgi_extension = cgiExtension; }
void LocationBlock::setUploadDir(const std::string& uploadDir) { _upload_dir = uploadDir; }
void LocationBlock::setClientMaxBodySize(size_t size) { _client_max_body_size = size; }
void LocationBlock::setReturnUrl(const std::string& returnUrl) { _return_url = returnUrl; }