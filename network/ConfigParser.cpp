/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hbarda <hbarda@student.42.fr>              #+#  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025-05-17 11:07:22 by hbarda            #+#    #+#             */
/*   Updated: 2025-05-17 11:07:22 by hbarda           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"

ConfigParser::ConfigParser(const std::string& filename)
            : _filename(filename) {}

ConfigParser::~ConfigParser() {}

bool ConfigParser::parse() {
    if (!readFile())
        return (false);
    return parseContent();
}

bool ConfigParser::readFile() {
    std::ifstream file(_filename.c_str());
    if (!file.is_open()) {
        std::cerr << "Error: could not open file " << _filename << std::endl;
        return false;
    }

    std::string line;
    while (getline(file, line)) {
        size_t comment_pos = line.find('#');
        if (comment_pos != std::string::npos) {
            line = line.substr(0, comment_pos);
        }

        if (!trimWhitespace(line).empty()) {
            _content.push_back(line);
        }
    }
    file.close();
    return true;
}

std::string ConfigParser::trimWhitespace(const std::string& str) {
    size_t first = str.find_first_not_of(" \n\t\r");
    if (first == std::string::npos)
        return "";
    size_t last = str.find_last_not_of(" \n\t\r");
    return (str.substr(first, last - first + 1));
}

bool ConfigParser::isValidCgiPath(const std::string& path) const {
    if (path.empty() || path[0] != '/') {
        return false;
    }
    if (path.find_first_of(" \t\n\r\"'{};") != std::string::npos) {
        return false;
    }
    return true;
}

bool ConfigParser::isValidLocationPath(const std::string& path) const {
    if (path == ".") {
        return false;
    }
    if (path[0] != '/' && (path.size() < 2 || path[0] != '~')) {
        return false;
    }
    return true;
}

bool ConfigParser::hasGarbageAfterSemicolon(const std::vector<std::string>& tokens) const {
    size_t semicolon_pos = 0;
    for (; semicolon_pos < tokens.size(); semicolon_pos++) {
        if (tokens[semicolon_pos] == ";") {
            break;
        }
    }
    return (semicolon_pos != tokens.size() - 1);
}

std::vector<std::string> ConfigParser::tokenize(const std::string& line) {
    std::vector<std::string> tokens;
    std::string token;
    bool in_quotes = false;

    for (size_t i = 0; i < line.length(); i++) {
        if (line[i] == '"') {
            in_quotes = !in_quotes;
            token += line[i];
        }
        else if ((line[i] == ' ' || line[i] == '\t') && !in_quotes) {
            if (!token.empty()) {
                tokens.push_back(token);
                token.clear();
            }
        }
        else if (line[i] == '{' || line[i] == '}') {
            if (!token.empty()) {
                tokens.push_back(token);
                token.clear();
            }
            tokens.push_back(std::string(1, line[i]));
        }
        else if (line[i] == ';') {
            if (!token.empty()) {
                tokens.push_back(token);
                token.clear();
            }
            tokens.push_back(std::string(1, line[i]));
        }
        else {
            token += line[i];
        }
    }

    if (!token.empty()) {
        tokens.push_back(token);
    }

    return tokens;
}

bool ConfigParser::parseContent() {
    size_t line_num = 0;
    while (line_num < _content.size()) {
        std::vector<std::string> tokens = tokenize(_content[line_num]);

        if (tokens.empty()) {
            line_num++;
            continue;
        }

        if (tokens[0] == "server") {
            if (!parseServerBlock(line_num)) {
                return false;
            }
        } else {
            std::cerr << "Error: expected 'server' directive at line " << line_num + 1 << std::endl;
            return false;
        }
    }

    for (size_t i = 0; i < _servers.size(); i++) {
        if (_servers[i].getPort() == -1) {
            std::cerr << "Error: Server must have a port" << std::endl;
            return false;
        }
    }

    return true;
}

bool ConfigParser::parseServerBlock(size_t& line_num) {
    std::vector<std::string> tokens = tokenize(_content[line_num]);
    if (tokens.size() != 2 || tokens[1] != "{") {
        std::cerr << "Error: Expected a brace '{' after 'server' at line " << line_num + 1 << std::endl;
        return false;
    }

    ServerBlock server;
    line_num++;

    while (line_num < _content.size()) {
        tokens = tokenize(_content[line_num]);
        
        if (tokens.empty()) {
            line_num++;
            continue;
        }
        
        if (tokens[0] == "}") {
            _servers.push_back(server);
            line_num++;
            return true;
        } 
        else if (tokens[0] == "listen") {
            if (tokens.size() < 2 || tokens.back() != ";") {
                std::cerr << "Error: Invalid listen directive at line " << line_num + 1 << std::endl;
                return false;
            }

            std::string listen = tokens[1];
            size_t colon_pos = listen.find(':');
            if (colon_pos != std::string::npos) {
                server.setHost(listen.substr(0, colon_pos));
                server.setPort(std::atoi(listen.substr(colon_pos + 1).c_str()));
            } else {
                server.setHost("0.0.0.0");
                server.setPort(std::atoi(listen.c_str()));
            }
        }
        else if (tokens[0] == "server_name") {
            if (tokens.size() < 2 || tokens.back() != ";") {
                std::cerr << "Error: Invalid server_name directive at line " << line_num + 1 << std::endl;
                return false;
            }

            for (size_t i = 1; i < tokens.size() - 1; i++) {
                server.addServerName(tokens[i]);
            }
        } 
        else if (tokens[0] == "root") {
            if (tokens.size() != 3 || tokens[2] != ";") {
                std::cerr << "Error: Invalid root directive at line " << line_num + 1 << std::endl;
                return false;
            }
            server.setRoot(tokens[1]);
        } 
        else if (tokens[0] == "index") {
            if (tokens.size() < 2 || tokens.back() != ";") {
                std::cerr << "Error: Invalid index directive at line " << line_num + 1 << std::endl;
                return false;
            }

            for (size_t i = 1; i < tokens.size() - 1; i++) {
                server.addIndexFile(tokens[i]);
            }
        } 
        else if (tokens[0] == "error_page") {
            if (tokens.size() != 4 || tokens[3] != ";") {
                std::cerr << "Error: Invalid error_page directive at line " << line_num + 1 << std::endl;
                return false;
            }

            int error_code = std::atoi(tokens[1].c_str());
            if (error_code < 100 || error_code > 599) {
                std::cerr << "Error: Invalid HTTP error code at line " << line_num + 1 << std::endl;
                return false;
            }
            server.addErrorPage(error_code, tokens[2]);
        } 
        else if (tokens[0] == "client_max_body_size") {
            if (tokens.size() != 3 || tokens[2] != ";") {
                std::cerr << "Error: Invalid client_max_body_size directive at line " << line_num + 1 << std::endl;
                return false;
            }

            std::string size_str = tokens[1];
            size_t size = std::atoi(size_str.c_str());
            char unit = size_str.empty() ? ' ' : size_str[size_str.size() - 1];

            if (unit == 'M' || unit == 'm') {
                size *= 1024 * 1024;
            } else if (unit == 'K' || unit == 'k') {
                size *= 1024;
            } else if (!std::isdigit(unit) && unit != ' ') {
                std::cerr << "Error: Invalid unit in client_max_body_size at line " << line_num + 1 << std::endl;
                return false;
            }

            server.setClientMaxBodySize(size);
        } 
        else if (tokens[0] == "location") {
            LocationBlock location;
            if (!parseLocationBlock(line_num, location)) {
                return false;
            }
            server.addLocation(location);
            line_num--;
        }
        else {
            std::cerr << "Error: Unknown directive '" << tokens[0] << "' at line " << line_num + 1 << std::endl;
            return false;
        }
        
        line_num++;
    }
    
    std::cerr << "Error: Unexpected end of file while parsing server block" << std::endl;
    return false;
}

bool ConfigParser::parseLocationBlock(size_t& line_num, LocationBlock& location) {
    std::vector<std::string> tokens = tokenize(_content[line_num]);
    
    if (tokens.size() < 3 || tokens[2] != "{" || !isValidLocationPath(tokens[1])) {
        std::cerr << "Error: Invalid location path '" << (tokens.size() > 1 ? tokens[1] : "") 
                  << "' at line " << line_num + 1 << std::endl;
        return false;
    }
    
    location.setPath(tokens[1]);
    line_num++;
    
    while (line_num < _content.size()) {
        tokens = tokenize(_content[line_num]);
        
        if (tokens.empty()) {
            line_num++;
            continue;
        }
        
        if (tokens[0] == "}") {
            line_num++;
            return true;
        }
        
        if (hasGarbageAfterSemicolon(tokens)) {
            std::cerr << "Error: Extra characters after semicolon at line " << line_num + 1 << std::endl;
            return false;
        }
        
        if (tokens[0] == "allowed_methods") {
            if (tokens.size() < 2 || tokens.back() != ";") {
                std::cerr << "Error: Invalid allowed_methods directive at line " << line_num + 1 << std::endl;
                return false;
            }
            
            std::vector<std::string> methods;
            for (size_t i = 1; i < tokens.size() - 1; i++) {
                // Convert method to uppercase for case-insensitive comparison
                std::string method = tokens[i];
                std::transform(method.begin(), method.end(), method.begin(), ::toupper);
                
                // Check if method is valid
                if (method != "GET" && method != "POST" && method != "DELETE") {
                    std::cerr << "Error: Invalid method '" << tokens[i] 
                              << "' at line " << line_num + 1 
                              << ". Only GET, POST, and DELETE are allowed." << std::endl;
                    return false;
                }
                methods.push_back(method);
            }
            location.setMethods(methods);
        }
        else if (tokens[0] == "root") {
            if (tokens.size() != 3 || tokens[2] != ";") {
                std::cerr << "Error: Invalid root directive at line " << line_num + 1 << std::endl;
                return false;
            }
            location.setRoot(tokens[1]);
        }
        else if (tokens[0] == "index") {
            if (tokens.size() < 2 || tokens.back() != ";") {
                std::cerr << "Error: Invalid index directive at line " << line_num + 1 << std::endl;
                return false;
            }
            
            std::vector<std::string> indexes;
            for (size_t i = 1; i < tokens.size() - 1; i++) {
                indexes.push_back(tokens[i]);
            }
            location.setIndexFiles(indexes);
        }
        else if (tokens[0] == "autoindex") {
            if (tokens.size() != 3 || tokens[2] != ";") {
                std::cerr << "Error: Invalid autoindex directive at line " << line_num + 1 << std::endl;
                return false;
            }
            
            if (tokens[1] != "on" && tokens[1] != "off") {
                std::cerr << "Error: autoindex must be 'on' or 'off' at line " << line_num + 1 << std::endl;
                return false;
            }
            location.setAutoindex(tokens[1] == "on");
        }
        else if (tokens[0] == "cgi_path") {
            if (tokens.size() != 3 || tokens[2] != ";") {
                std::cerr << "Error: Invalid cgi_path directive at line " << line_num + 1 << std::endl;
                return false;
            }
            
            if (!isValidCgiPath(tokens[1])) {
                std::cerr << "Error: Invalid CGI path '" << tokens[1] << "' at line " << line_num + 1 << std::endl;
                return false;
            }
            location.setCgiPath(tokens[1]);
        }
        else if (tokens[0] == "cgi_extension") {
            if (tokens.size() != 3 || tokens[2] != ";") {
                std::cerr << "Error: Invalid cgi_extension directive at line " << line_num + 1 << std::endl;
                return false;
            }
            
            if (tokens[1].empty() || tokens[1][0] != '.') {
                std::cerr << "Error: CGI extension must start with '.' at line " << line_num + 1 << std::endl;
                return false;
            }
            location.setCgiExtension(tokens[1]);
        }
        else if (tokens[0] == "upload_dir") {
            if (tokens.size() != 3 || tokens[2] != ";") {
                std::cerr << "Error: Invalid upload_dir directive at line " << line_num + 1 << std::endl;
                return false;
            }
            location.setUploadDir(tokens[1]);
        }
        else if (tokens[0] == "client_max_body_size") {
            if (tokens.size() != 3 || tokens[2] != ";") {
                std::cerr << "Error: Invalid client_max_body_size directive at line " << line_num + 1 << std::endl;
                return false;
            }

            std::string size_str = tokens[1];
            size_t size = std::atoi(size_str.c_str());
            char unit = size_str.empty() ? ' ' : size_str[size_str.size() - 1];

            if (unit == 'M' || unit == 'm') {
                size *= 1024 * 1024;
            } else if (unit == 'K' || unit == 'k') {
                size *= 1024;
            }

            location.setClientMaxBodySize(size);
        }
        else if (tokens[0] == "return") {
            if (tokens.size() != 3 || tokens[2] != ";") {
                std::cerr << "Error: Invalid return directive at line " << line_num + 1 << std::endl;
                return false;
            }
            location.setReturnUrl(tokens[1]);
        }
        else {
            std::cerr << "Error: Unknown location directive '" << tokens[0] << "' at line " << line_num + 1 << std::endl;
            return false;
        }
        
        line_num++;
    }
    
    std::cerr << "Error: Unexpected end of file while parsing location block" << std::endl;
    return false;
}