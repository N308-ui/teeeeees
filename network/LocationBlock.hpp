#ifndef LOCATIONBLOCK_HPP
#define LOCATIONBLOCK_HPP
#include "../header/class_header.hpp"

class LocationBlock {
  private:
    std::string _path;
    std::vector<std::string> _methods;
    std::string _root;
    std::vector<std::string> _index_files;
    bool _autoindex;
    std::string _cgi_path;
    std::string _cgi_extension;
    std::string _upload_dir;
    size_t _client_max_body_size;
    std::string _return_url;
  public:
    LocationBlock();
    LocationBlock(LocationBlock const &obj);
    LocationBlock& operator=(const LocationBlock& obj);
    ~LocationBlock();
    void just_this_error();

    // getters
    std::string getPath() const;
    std::vector<std::string> getMethods() const;
    std::string getRoot() const;
    std::vector<std::string> getIndexFiles() const;
    bool getAutoindex() const;
    std::string getCgiPath() const;
    std::string getCgiExtension() const;
    std::string getUploadDir() const;
    size_t getClientMaxBodySize() const;
    std::string getReturnUrl() const;

    // setters
    void setPath(const std::string& path);
    void setMethods(const std::vector<std::string>& methods);
    void setRoot(const std::string& root);
    void setIndexFiles(const std::vector<std::string>& indexFiles);
    void setAutoindex(bool autoindex);
    void setCgiPath(const std::string& cgiPath);
    void setCgiExtension(const std::string& cgiExtension);
    void setUploadDir(const std::string& uploadDir);
    void setClientMaxBodySize(size_t size);
    void setReturnUrl(const std::string& returnUrl);
};

#endif