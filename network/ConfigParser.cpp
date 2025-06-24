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

std::vector<std::string> ConfigParser::tokenize(const std::string& line) {
    std::vector<std::string> tokens;
    std::string token;
    bool in_quotes = false;

    for (size_t i = 0; i < line.length(); i++) {
        // If the current character is a double quote, toggle the quote flag and add it to the token
        if (line[i] == '"') {
            in_quotes = !in_quotes;
            token += line[i];
        }
        // If it's a space or tab and we're not inside quotes, treat it as a token separator
        else if ((line[i] == ' ' || line[i] == '\t') && !in_quotes) {
            if (!token.empty()) {
                tokens.push_back(token);
                token.clear();
            }
        }
        // If it's a curly brace, push current token (if any), then push the brace as a token
        else if (line[i] == '{' || line[i] == '}') {
            if (!token.empty()) {
                tokens.push_back(token);
                token.clear();
            }
            tokens.push_back(std::string(1, line[i]));
        }
        // For all other characters, add to the current token
        else {
            token += line[i];
        }
    }

    // After the loop, if there's any token left, push it
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
    // Check for open brace '{'
    std::vector<std::string> tokens = tokenize(_content[line_num]);
    if (tokens.size() != 2 || tokens[1] != "{") {
        std::cerr << "Error: Expected a brace '{' after 'server' at line " << line_num + 1 << std::endl;
        return false;
    }

    ServerBlock server;
    line_num++;

    // Parse server block content until we find a closing brace
    while (line_num < _content.size()) {
        tokens = tokenize(_content[line_num]);
        
        if (tokens.empty()) {
            line_num++;
            continue;
        }
        
        if (tokens[0] == "}") {
            // End of server block
            _servers.push_back(server);
            line_num++;
            return true;
        } 
        else if (tokens[0] == "listen") {
            if (tokens.size() >= 2) {
                // Handle IP:PORT format
                size_t colon_pos = tokens[1].find(':');
                if (colon_pos != std::string::npos) {
                    server.setHost(tokens[1].substr(0, colon_pos));
                    server.setPort(std::atoi(tokens[1].substr(colon_pos + 1).c_str()));
                } else {
                    // Handle just port number (default host)
                    server.setHost("0.0.0.0"); // Default host
                    server.setPort(std::atoi(tokens[1].c_str()));
                }
            } else {
                std::cerr << "Error: 'listen' directive requires an argument at line " << line_num + 1 << std::endl;
                return false;
            }
        }
        else if (tokens[0] == "server_name") {
            for (size_t i = 1; i < tokens.size(); i++) {
                server.addServerName(tokens[i]);
            }
        } 
        else if (tokens[0] == "root" && tokens.size() >= 2) {
            server.setRoot(tokens[1]);
        } 
        else if (tokens[0] == "index") {
            for (size_t i = 1; i < tokens.size(); i++) {
                server.addIndexFile(tokens[i]);
            }
        } 
        else if (tokens[0] == "error_page" && tokens.size() >= 3) {
            int error_code = std::atoi(tokens[1].c_str());
            if (error_code < 100 || error_code > 599) {
                std::cerr << "Error: Invalid HTTP error code at line " << line_num + 1 << std::endl;
                return false;
            }
            server.addErrorPage(error_code, tokens[2]);
        } 
        else if (tokens[0] == "client_max_body_size" && tokens.size() >= 2) {
            size_t size = std::atoi(tokens[1].c_str());
            char unit;
            if (tokens[1].size())
                unit = tokens[1][tokens[1].size() -1];
            else
              unit = ' ';
            if (unit == 'M' || unit == 'm') {
                size *= 1024 * 1024;
            } else if (unit == 'K' || unit == 'k') {
                size *= 1024;
            }
            server.setClientMaxBodySize(size);
        } 
        else if (tokens[0] == "location") {
            if (tokens.size() < 2) {
                std::cerr << "Error: 'location' directive requires a path at line " << line_num + 1 << std::endl;
                return false;
            }
            
            LocationBlock location;
            // Skip location block for now (you'll need to implement this)
            // if (!parseLocationBlock(line_num, location)) {
            //     return false;
            // }

            // server.addLocation(location);


            int brace_count = 0;
            bool found_open_brace = false;
            
            while (line_num < _content.size()) {
                tokens = tokenize(_content[line_num]);
                
                for (size_t i = 0; i < tokens.size(); i++) {
                    if (tokens[i] == "{") {
                        found_open_brace = true;
                        brace_count++;
                    } else if (tokens[i] == "}") {
                        brace_count--;
                        if (brace_count == 0 && found_open_brace) {
                            break;
                        }
                    }
                }
                
                if (brace_count == 0 && found_open_brace) {
                    server.addLocation(location);
                    break;
                }
                
                line_num++;
            }
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
