#ifndef SERVER_HPP
#define SERVER_HPP

#include "../header/class_header.hpp"
#include "ServerBlock.hpp"
#include <map>
#include <vector>
#include <string>

#include "RequestParser.hpp"

class Server {
private:
    int socket_fd;
    std::string ip;
    std::string port;
    struct addrinfo hints;
    struct addrinfo* res;
    ServerBlock data;
    struct epoll_event ev;
    std::vector<int> clients;
    std::map<int, std::string> request;
    std::map<int, RequestParser > parsers;    // just modified to pointer
    std::map<int, std::string> responses;

    void processRequest(int client_fd);
    void handleGetRequest(int client_fd, const RequestParser& parser);
    void handlePostRequest(int client_fd, const RequestParser& parser);
    void handleDeleteRequest(int client_fd, const RequestParser& parser);
    std::string createErrorResponse(int code, const std::string& message);
    void sendErrorResponse(int client_fd, int code, const std::string& message);

    // method : help function 
    int  mathch_location(std::string);
    std::string listFile(std::string);

public:
    Server();
    Server(std::string ip, std::string port);
    Server(ServerBlock& data);
    Server(const Server& obj);
    ~Server();
    void handleFileUpload(int client_fd, const RequestParser& parser, const LocationBlock& location);
    void setup();
    std::string trimWhitespace(const std::string& str);
    int setNonBlocking(int fd);
    void addClient(int epoll);
    void set_in_epoll(int epoll);
    void removeClient(int client_fd, int epoll);
    bool client_is_here(int client_fd);
    std::string getIpPort() const;
    int getSocketFD() const;
    void run(struct epoll_event event, int epoll);
};

#endif