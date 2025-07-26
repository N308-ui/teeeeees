#include "../network/server.hpp"
#include "../header/class_header.hpp"
#include "method_headher.hpp"


void Server::handleGetRequest(int client_fd, const RequestParser& parser) {
    std::string uri = parser.getPath();
    std::string line = parser.getUri();
    LocationBlock loc = data.getLocations()[mathch_location(uri)];
    size_t len = loc.getPath().size() == 1 ? 0 : loc.getPath().size() ;
    std::string file_path = loc.getRoot() + uri.substr(len);
    std::cout << "<<<GEt METHOD>>>" << std::endl;
    std::cout << parser.getUri() << std::endl;
    std::cout << uri << std::endl;
    std::cout << file_path << std::endl;
    std::cout << line << ":" << parser.getPath().size() << std::endl;

    e_Status s = status(file_path);
    if (s == _FILE)
    {

        std::ifstream file(file_path.c_str());
        if (file.is_open()) {
            std::string content((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());
            responses[client_fd] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n" + content;
        } else {
            sendErrorResponse(client_fd, 404, "Not Found");
        }
    }
    else if (s == _DIR)
    {
        
        if (line[parser.getPath().size()] != '/' && line != "/")
        {
            sendErrorResponse(client_fd, 301, "Moved Permanently");
            return;
        }
        if (loc.getAutoindex())
        {
            std::string content = listFile( file_path);
            responses[client_fd] = "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n" + content;
            return;
        }
        if (loc.getIndexFiles().empty())
        {
            sendErrorResponse(client_fd, 403, "Forbidden");
            return;
        }
        file_path += loc.getIndexFiles()[0];
        std::ifstream file(file_path.c_str());
        if (file.is_open()) {
            std::string content((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());
            // cgi 
            responses[client_fd] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n" + content;
        } else {
            sendErrorResponse(client_fd, 404, "Not Found");
        }
    }
    else
    {
        sendErrorResponse(client_fd, 404, "Not Found");
    }
}