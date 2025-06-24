#ifndef SERVER_HPP
#define SERVER_HPP

#include "../header/class_header.hpp"
#include "ServerBlock.hpp"


class Server
{
private:
    int socket_fd ;
    std::string ip;
    std::string port;
    struct addrinfo hints ;
    struct addrinfo * res ;
    ServerBlock data;
    struct epoll_event ev;
    std::vector<int> clients;
    std::map<int, std::string> request; 
public:
    Server(); 
    Server(std::string _ip , std::string _port);
    Server(ServerBlock& _data);
    Server(Server const& obj);
    void setup();   
    void run(struct epoll_event event, int epoll); // need update  
    int setNonBlocking(int fd);
    void addClient(int epoll);
    void set_in_epoll(int epoll);
    void removeClient(int fd_client, int epoll);
    bool client_is_here(int clinet);
    std::string getIpPort();
    int getSocketFD() const;
    ~Server();
};



#endif  
