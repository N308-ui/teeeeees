#include "server.hpp"
#include "RequestParser.hpp"
// #include "../method/methods.cpp"
#include <sys/stat.h>
#include <fstream>
#include <sstream>
#include <cstdlib>


Server::Server() : socket_fd(-1), ip("127.0.0.1"), port("8080"), res(NULL) {}

Server::Server(ServerBlock& data) : 
    socket_fd(-1), ip(data.getHost()), port(to_string(data.getPort())), res(NULL), data(data) {}

Server::Server(const Server& obj) : 
    socket_fd(-1), ip(obj.ip), port(obj.port), res(NULL), data(obj.data) {}

Server::~Server() {
    if (res) {
        freeaddrinfo(res);
        res = NULL;
    }
    if (socket_fd >= 0) {
        close(socket_fd);
        socket_fd = -1;
    }
}

int Server::setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void Server::setup() {
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(ip.c_str(), port.c_str(), &hints, &res)) {
        throw std::runtime_error("getaddrinfo error");
    }

    socket_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (socket_fd < 0) {
        throw std::runtime_error("socket creation error");
    }

    int opt = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        throw std::runtime_error("setsockopt error");
    }

    if (bind(socket_fd, res->ai_addr, res->ai_addrlen) < 0) {
        throw std::runtime_error("bind error");
    }

    if (setNonBlocking(socket_fd)) {
        throw std::runtime_error("fcntl error");
    }

    if (listen(socket_fd, SOMAXCONN) < 0) {
        throw std::runtime_error("listen error");
    }

    ev.data.fd = socket_fd;
    ev.events = EPOLLIN;
    std::cout << "Server started on " << ip << ":" << port << std::endl;
}

void Server::set_in_epoll(int epoll_fd) {
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &ev) < 0) {
        std::string msg = "Cannot add server " + getIpPort() + " to epoll: ";
        msg += strerror(errno);
        throw std::runtime_error(msg);
    }
}

void Server::addClient(int epoll_fd) {
    struct sockaddr client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(socket_fd, &client_addr, &client_len);

    if (client_fd < 0) {
        throw std::runtime_error("accept error");
    }

    if (setNonBlocking(client_fd)) {
        close(client_fd);
        throw std::runtime_error("fcntl error");
    }

    struct epoll_event client_ev;
    client_ev.events = EPOLLIN ;
    client_ev.data.fd = client_fd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &client_ev) < 0) {
        close(client_fd);
        throw std::runtime_error("epoll_ctl error");
    }

    clients.push_back(client_fd);
    request[client_fd] = "";
}

void Server::removeClient(int client_fd, int epoll_fd) {
    if (!client_is_here(client_fd)) return;

    clients.erase(std::remove(clients.begin(), clients.end(), client_fd), clients.end());
    request.erase(client_fd);
    parsers.erase(client_fd);
    responses.erase(client_fd);

    if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL) < 0) {
        perror("epoll_ctl del");
    }

    close(client_fd);
}

bool Server::client_is_here(int client_fd) {
    return std::find(clients.begin(), clients.end(), client_fd) != clients.end();
}

void Server::run(struct epoll_event event, int epoll_fd) {
    int client_fd = event.data.fd;
    if (event.events == EPOLLIN) {
        char buffer[4096];
        ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer), 0);

        if (bytes_read <= 0) {
            removeClient(client_fd, epoll_fd);
            return;
        }
        request[client_fd].append(buffer, bytes_read);
        if (parsers.find(client_fd) == parsers.end()) {
            parsers[client_fd] = RequestParser();
        }
        if (request[client_fd].find("\r\n\r\n") != std::string::npos)
        {
            RequestParser& parser = parsers[client_fd];
            if (parser.parse(request[client_fd])) {
                if (parser.isValid()) {
                    processRequest(client_fd);
                } else {
                    sendErrorResponse(client_fd, 400, "Bad Request");
                    removeClient(client_fd, epoll_fd);
                }
            }
            std::cout << "chang status of file " << std::endl;
            struct epoll_event ev;
            ev.events = EPOLLOUT ;
            ev.data.fd = client_fd;
            if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, ev.data.fd, &ev) < 0)
                std::cerr << "something happend in epoll after change";
        }
    } 
    else if (event.events == EPOLLOUT ) {
        if (responses.find(client_fd) != responses.end()) {
            std::string& response = responses[client_fd];
            ssize_t bytes_sent = send(client_fd, response.c_str(), response.size(), 0);
            // removeClient(client_fd, epoll_fd);
            if (bytes_sent > 0) {
                response.erase(0, bytes_sent);
                if (response.empty()) {
                    removeClient(client_fd, epoll_fd);
                }
            } else {
                removeClient(client_fd, epoll_fd);
            }
        } else {
            std::cout << "hhh" << std::endl;    
            sendErrorResponse(client_fd, 500, "Internal Server Error");
            removeClient(client_fd, epoll_fd);
        }
    }
    else {
        removeClient(client_fd, epoll_fd);
    }
}

void Server::processRequest(int client_fd) {
    RequestParser& parser = parsers[client_fd];
    std::string response;
    int index = mathch_location(parser.getUri());
     LocationBlock lo = this->data.getLocations()[index];
     if (index != -1)
    {
        if (std::find(lo.getMethods().begin(), lo.getMethods().end(), parser.getMethod()) == lo.getMethods().end())
        {
            response = createErrorResponse(405, "Method Not Allowed");
            responses[client_fd] = response;
            return;
        }
        if (parser.getMethod() == "GET" ) {
            handleGetRequest(client_fd, parser);
        } else if (parser.getMethod() == "POST" ) {
            handlePostRequest(client_fd, parser);
        } else if (parser.getMethod() == "DELETE" ) {
            handleDeleteRequest(client_fd, parser);
        } else {
            response = createErrorResponse(405, "Method Not Allowed");
            responses[client_fd] = response;
        }
    }
    else
    {
        responses[client_fd] = createErrorResponse(404, "Not Found");
    }

}



std::string Server::createErrorResponse(int code, const std::string& message) {
    std::string body = "<html><body><h1>" + to_string(code) + " " + message + "</h1></body></html>";
    return "HTTP/1.1 " + to_string(code) + " " + message + "\r\nContent-Type: text/html\r\n\r\n" + body;
}

void Server::sendErrorResponse(int client_fd, int code, const std::string& message) {
    responses[client_fd] = createErrorResponse(code, message);
}

std::string Server::getIpPort() const {
    return ip + ":" + port;
}

int Server::getSocketFD() const {
    return socket_fd;
}