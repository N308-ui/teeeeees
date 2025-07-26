#ifndef REQUESTPARSER_HPP
#define REQUESTPARSER_HPP

#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <stdexcept>

class RequestParser {
private:
    std::string _method;
    std::string _uri;
    std::string _path;
    std::string _version;
    std::map<std::string, std::string> _headers;
    std::string _body;
    std::string _query_string;
    std::map<std::string, std::string> _query_params;
    bool _is_complete;
    bool _is_valid;
    bool _is_chunked;
    size_t _expected_body_size;

    void parseFirstLine(const std::string& line);
    void parseHeader(const std::string& line);
    void parseQueryString();
    void parsePath();
    std::string urlDecode(const std::string& src);
    std::string trimWhitespace(const std::string& str);
    std::string toLowerCase(const std::string& str) const;
    bool validateMethod();
    bool validateHttpVersion();
    bool validateHeaders();
    void normalizePath();

public:
    RequestParser();
    ~RequestParser();

    bool parse(const std::string& raw_request);
    void reset();

    // Getters
    const std::string& getMethod() const;
    const std::string& getUri() const;
    const std::string& getPath() const;
    const std::string& getVersion() const;
    const std::map<std::string, std::string>& getHeaders() const;
    const std::string& getBody() const;
    const std::string& getQueryString() const;
    const std::map<std::string, std::string>& getQueryParams() const;
    std::string getHeader(const std::string& name) const;
    bool hasHeader(const std::string& name) const;

    // Validation methods
    bool isComplete() const;
    bool isValid() const;
    bool isKeepAlive() const;
    bool isChunked() const;
    bool parseChunkedBody();
    bool isRequestWellFormed() const;
    bool isUriValid() const;
    bool isUriTooLong() const;
    bool isBodySizeValid(size_t client_max_body_size) const;
    bool isMethodAllowed(const std::vector<std::string>& allowed_methods) const;
    bool isTransferEncodingValid() const;
    bool isMethodImplemented() const;

    // HTTP specific getters
    size_t getContentLength() const;
    std::string getContentType() const;
    std::string getHost() const;
    std::string getConnection() const;
    bool keepAlive() const;

    // CGI support
    std::map<std::string, std::string> getCgiEnvironment() const;
};

#endif