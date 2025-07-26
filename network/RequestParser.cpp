#include "RequestParser.hpp"
#include <cstdlib>
#include <cstring>
#include <stdexcept>

RequestParser::RequestParser() 
    : _is_complete(false), _is_valid(false), _is_chunked(false), _expected_body_size(0) {}

RequestParser::~RequestParser() {}

bool RequestParser::parse(const std::string& raw_request) {
    reset();
    
    if (raw_request.empty()) {
        _is_valid = false;
        return false;
    }

    size_t header_end = raw_request.find("\r\n\r\n");
    if (header_end == std::string::npos) {
        _is_complete = false;
        _is_valid = false;
        return false;
    }

    _is_complete = true;
    std::string headers_part = raw_request.substr(0, header_end);
    _body = raw_request.substr(header_end + 4);

    // Parse headers
    std::istringstream iss(headers_part);
    std::string line;
    
    // First line
    if (!std::getline(iss, line)) {
        _is_valid = false;
        return false;
    }
    parseFirstLine(line);

    // Remaining headers
    while (std::getline(iss, line)) {
        if (line.empty() || line == "\r") continue;
        parseHeader(line);
    }

    // Validate request components
    if (!validateMethod() || !validateHttpVersion() || !validateHeaders()) {
        _is_valid = false;
        return false;
    }

    // Handle chunked transfer encoding
    if (isChunked()) {
        if (!parseChunkedBody()) {
            _is_valid = false;
            return false;
        }
    }
    // Handle content length
    else if (hasHeader("content-length")) {
        size_t content_length = getContentLength();
        if (_body.length() < content_length) {
            _is_complete = false;
            _is_valid = false;
            return false;
        }
        _body = _body.substr(0, content_length);
    }

    // Parse query string and path
    parseQueryString();
    parsePath();
    normalizePath();

    _is_valid = true;
    return true;
}

void RequestParser::parseFirstLine(const std::string& line) {
    std::istringstream iss(line);
    std::string method, uri, version;
    
    if (!(iss >> method >> uri >> version)) {
        _is_valid = false;
        return;
    }

    _method = method;
    _uri = uri;
    _version = version;

     _uri = urlDecode(uri);

     if (_version != "HTTP/1.1" && _version != "HTTP/1.0") {
        _is_valid = false;
    }
}

void RequestParser::parseHeader(const std::string& line) {
    size_t colon_pos = line.find(':');
    if (colon_pos == std::string::npos) return;

    std::string name = trimWhitespace(line.substr(0, colon_pos));
    std::string value = trimWhitespace(line.substr(colon_pos + 1));
    
    _headers[toLowerCase(name)] = value;

    if (toLowerCase(name) == "transfer-encoding" && 
        value.find("chunked") != std::string::npos) {
        _is_chunked = true;
    }
}

void RequestParser::parseQueryString() {
    size_t query_pos = _uri.find('?');
    if (query_pos == std::string::npos) return;

    _path = _uri.substr(0, query_pos);
    _query_string = _uri.substr(query_pos + 1);

    std::istringstream iss(_query_string);
    std::string pair;
    
    while (std::getline(iss, pair, '&')) {
        size_t eq_pos = pair.find('=');
        if (eq_pos != std::string::npos) {
            std::string key = urlDecode(pair.substr(0, eq_pos));
            std::string value = urlDecode(pair.substr(eq_pos + 1));
            _query_params[key] = value;
        }
    }
}

void RequestParser::parsePath() {
    if (_path.empty()) {
        size_t query_pos = _uri.find('?');
        _path = (query_pos == std::string::npos) ? _uri : _uri.substr(0, query_pos);
    }
}

void RequestParser::normalizePath() {
    std::string normalized;
    std::vector<std::string> parts;
    std::istringstream iss(_path);
    std::string part;
    
    while (std::getline(iss, part, '/')) {
        if (part.empty() || part == ".") continue;
        if (part == "..") {
            if (!parts.empty()) parts.pop_back();
        } else {
            parts.push_back(part);
        }
    }
    
    for (size_t i = 0; i < parts.size(); ++i) {
        normalized += "/" + parts[i];
    }
    
    if (normalized.empty()) normalized = "/";
    _path = normalized;
}

bool RequestParser::validateMethod() {
    if (_method.empty()) return false;
    
    // Check for allowed methods (GET, POST, DELETE)
    const std::string allowed[] = {"GET", "POST", "DELETE"};
    for (size_t i = 0; i < 3; ++i) {
        if (_method == allowed[i]) return true;
    }
    return false;
}

bool RequestParser::validateHttpVersion() {
    if (_version != "HTTP/1.1" && _version != "HTTP/1.0") {
        return false;
    }
    
    // HTTP/1.1 requires Host header
    if (_version == "HTTP/1.1" && !hasHeader("host")) {
        return false;
    }
    
    return true;
}

bool RequestParser::validateHeaders() {
    // Check for invalid headers
    for (std::map<std::string, std::string>::const_iterator it = _headers.begin();
         it != _headers.end(); ++it) {
        if (it->first.empty() || it->second.empty()) {
            return false;
        }
    }
    return true;
}

std::string RequestParser::urlDecode(const std::string& src) {
    std::string result;
    result.reserve(src.length()); // Optimize by reserving space
    
    for (size_t i = 0; i < src.length(); i++) {
        if (src[i] == '%' && i + 2 < src.length()) {
            
            char hex[3] = { src[i+1], src[i+2], '\0' };
            
            
            char *end;
            long value = strtol(hex, &end, 16);
            
            
            if (*end == '\0') {
                result += static_cast<char>(value);
                i += 2;
            } else {
                
                result += src[i];
            }
        } 
        else if (src[i] == '+') {
            result += ' ';
        }
        else {
            result += src[i];
        }
    }
    return result;
}

// std::string RequestParser::urlDecode(const std::string& src) {
//     std::string result;
//     for (size_t i = 0; i < src.length(); i++) {
//         if (src[i] == '%' && i + 2 < src.length()) {
//             int value;
//             std::istringstream iss(src.substr(i + 1, 2));
//             if (iss >> std::hex >> value) {
//                 result += static_cast<char>(value);
//                 i += 2;
//             } else {
//                 result += src[i];
//             }
//         } else if (src[i] == '+') {
//             result += ' ';
//         } else {
//             result += src[i];
//         }
//     }
//     return result;
// }

std::string RequestParser::trimWhitespace(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
        return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}

std::string RequestParser::toLowerCase(const std::string& str) const {
    std::string result = str;
    for (size_t i = 0; i < result.size(); ++i) {
        result[i] = tolower(result[i]);
    }
    return result;
}

void RequestParser::reset() {
    _method.clear();
    _uri.clear();
    _path.clear();
    _version.clear();
    _headers.clear();
    _body.clear();
    _query_string.clear();
    _query_params.clear();
    _is_complete = false;
    _is_valid = false;
    _is_chunked = false;
    _expected_body_size = 0;
}

bool RequestParser::isRequestWellFormed() const {
    if (hasHeader("transfer-encoding")) {
        std::string te = toLowerCase(getHeader("transfer-encoding"));
        if (te != "chunked") {
            return false;
        }
    }
    
    if (_method == "POST" && !hasHeader("content-length") && !hasHeader("transfer-encoding")) {
        return false;
    }
    
    return true;
}

bool RequestParser::isUriValid() const {
    const std::string allowed_chars = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~:/?#[]@!$&'()*+,;=%";
    
    for (size_t i = 0; i < _uri.length(); i++) {
        if (allowed_chars.find(_uri[i]) == std::string::npos) {
            return false;
        }
    }
    return true;
}

bool RequestParser::isUriTooLong() const {
    return _uri.length() > 2000;
}

bool RequestParser::isBodySizeValid(size_t client_max_body_size) const {
    if (hasHeader("content-length")) {
        size_t content_length = getContentLength();
        return content_length <= client_max_body_size;
    }
    return true;
}

bool RequestParser::isMethodAllowed(const std::vector<std::string>& allowed_methods) const {
    for (std::vector<std::string>::const_iterator it = allowed_methods.begin(); 
         it != allowed_methods.end(); ++it) {
        if (*it == _method) {
            return true;
        }
    }
    return false;
}

bool RequestParser::isMethodImplemented() const {
    return _method == "GET" || _method == "POST" || _method == "DELETE";
}

bool RequestParser::isTransferEncodingValid() const {
    if (hasHeader("transfer-encoding")) {
        std::string te = toLowerCase(getHeader("transfer-encoding"));
        return te == "chunked";
    }
    return true;
}

bool RequestParser::isComplete() const { return _is_complete; }
bool RequestParser::isValid() const { return _is_valid; }
bool RequestParser::isKeepAlive() const { return keepAlive(); }
bool RequestParser::isChunked() const { return _is_chunked; }

size_t RequestParser::getContentLength() const {
    std::string length_str = getHeader("content-length");
    return length_str.empty() ? 0 : atoi(length_str.c_str());
}

std::string RequestParser::getContentType() const {
    return getHeader("content-type");
}

std::string RequestParser::getHost() const {
    return getHeader("host");
}

std::string RequestParser::getConnection() const {
    return getHeader("connection");
}

bool RequestParser::keepAlive() const {
    std::string connection = getConnection();
    return connection != "close";
}

bool RequestParser::hasHeader(const std::string& name) const {
    return _headers.find(toLowerCase(name)) != _headers.end();
}

std::string RequestParser::getHeader(const std::string& name) const {
    std::map<std::string, std::string>::const_iterator it = _headers.find(toLowerCase(name));
    if (it != _headers.end()) {
        return it->second;
    }
    return "";
}

const std::string& RequestParser::getMethod() const {
    return _method;
}

const std::string& RequestParser::getUri() const {
    return _uri;
}

const std::string& RequestParser::getPath() const {
    return _path;
}

const std::string& RequestParser::getVersion() const {
    return _version;
}

const std::map<std::string, std::string>& RequestParser::getHeaders() const {
    return _headers;
}

const std::string& RequestParser::getBody() const {
    return _body;
}

const std::string& RequestParser::getQueryString() const {
    return _query_string;
}

const std::map<std::string, std::string>& RequestParser::getQueryParams() const {
    return _query_params;
}

bool RequestParser::parseChunkedBody() {
    std::string unchunked_body;
    size_t pos = 0;
    
    while (pos < _body.length()) {
        // Find chunk size
        size_t line_end = _body.find("\r\n", pos);
        if (line_end == std::string::npos) return false;
        
        std::string hex_size = _body.substr(pos, line_end - pos);
        size_t chunk_size;
        std::istringstream iss(hex_size);
        if (!(iss >> std::hex >> chunk_size)) return false;
        
        if (chunk_size == 0) {
            // End of chunks
            break;
        }
        
        pos = line_end + 2;
        if (pos + chunk_size > _body.length()) return false;
        
        unchunked_body += _body.substr(pos, chunk_size);
        pos += chunk_size + 2; // Skip \r\n after chunk data
    }
    
    _body = unchunked_body;
    return true;
}

std::map<std::string, std::string> RequestParser::getCgiEnvironment() const {
    std::map<std::string, std::string> env;
    
    // Required variables
    env["REQUEST_METHOD"] = _method;
    env["QUERY_STRING"] = _query_string;
    env["CONTENT_LENGTH"] = hasHeader("content-length") ? getHeader("content-length") : "";
    env["CONTENT_TYPE"] = getContentType();
    env["SERVER_PROTOCOL"] = _version;
    env["PATH_INFO"] = _path;
    env["PATH_TRANSLATED"] = _path; // Will be updated by server with actual path
    env["SCRIPT_NAME"] = _path; // Will be updated by server
    
    // HTTP headers
    for (std::map<std::string, std::string>::const_iterator it = _headers.begin();
         it != _headers.end(); ++it) {
        std::string env_name = "HTTP_" + toLowerCase(it->first);
        std::replace(env_name.begin(), env_name.end(), '-', '_');
        env[env_name] = it->second;
    }
    
    return env;
}