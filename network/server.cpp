#include "server.hpp"
#include "../header/class_header.hpp"

Server::Server():
    socket_fd(-1), ip("127.0.0.1"), port("8080"), res(NULL)
{
}

Server::Server(ServerBlock& _data) :
    socket_fd(-1), ip(_data.getHost()), port(to_string(_data.getPort())), res(NULL), data(_data)
{
}

Server::Server(Server const & obj) :
    socket_fd(-1), ip(obj.ip), port(obj.port), res(NULL), data(obj.data)
{
}
// Server::Server(Server const & obj) 
//    port(obj.port), ip(obj.ip),data(obj.data), socket_fd(-1),res(NULL)
// {
//     //this->port = obj.port;
//     //this->ip = obj.ip;
//     //this->data = obj.data;
//     //this->socket_fd = -1;
//     //this->res = NULL;
// }

int Server::setNonBlocking(int fd)
{
    int flag = fcntl(fd , F_GETFD, 0);
    return (flag < 0 || fcntl(fd , F_SETFD, flag | O_NONBLOCK) < 0);
}

void Server::setup()
{
    //setup adddrinfo var
    std::memset(&this->hints, 0, sizeof(this->hints));
    this->hints.ai_family = AF_INET;
    this->hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(this->ip.c_str(), this->port.c_str(),&this->hints, &this->res))
        throw std::runtime_error("getaddrinfo error\n");
    //create socket 
    this->socket_fd = socket(this->res->ai_family,this->res->ai_socktype, 0);
    if(this->socket_fd < 0)
        throw std::runtime_error("error: can not create socket\n");
    int opt = 1;
  if (setsockopt(this->socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
      perror("setsockopt(SO_REUSEADDR) failed");
    }
    // bind the socket with port and ip        
    if (bind(this->socket_fd, this->res->ai_addr, this->res->ai_addrlen) < 0)
        throw std::runtime_error("error: can not bind socket\n");
    // set non bolocking 
    if (this->setNonBlocking(this->socket_fd) )
        throw std::runtime_error("error: fnctl\n");
    // listen for new connection     
    if (listen(this->socket_fd, 1) < 0)
        throw std::runtime_error("error: somthing wrong in listen function\n");
    ev.data.fd = this->socket_fd;
    ev.events = EPOLLIN;
    std::cout << "http server: "<< ip << ":" << port << std::endl;
}

void  Server::run(struct epoll_event event, int epoll)
{ 
    std::string message ;
    if(event.events  == EPOLLIN)
    {
        int size = data.getClientMaxBodySize();
        std::vector<char> buffer(size);
        int b = read(event.data.fd,&buffer[0], size);
        if(b < 0)
            throw std::runtime_error("read() can read client file\n");  
        request[event.data.fd].append(buffer.begin(), buffer.begin() + b);
        if (request[event.data.fd].find("\r\n\r\n") != std::string::npos)
        {
          //here should parsing the requset {requset[event.data.fd]};
          struct epoll_event ev;
          ev.events = EPOLLOUT ;
          ev.data.fd = event.data.fd;
          epoll_ctl(epoll, EPOLL_CTL_MOD, event.data.fd, &ev);
          std::cout << "chang mod of event";
        }
        std::cout <<request[event.data.fd] <<  "read requset" << b << std::endl;  
  }
    else if (event.events == EPOLLOUT)
    {
      // here should code methed and cgi;
      send(event.data.fd , request[event.data.fd].c_str(),
           request[event.data.fd].size() , 0);
      this->removeClient(event.data.fd, epoll);
      std::cout << "send respos\n";
    }
    else
    {
    std::cout << "there is a problem in epoll or in client maybe" << std::endl;
    }

}

void Server::addClient(int epoll)
{
    int client;
    struct sockaddr client_addr;
    socklen_t socklen  = sizeof(client_addr);
    struct epoll_event event;
    client = accept(this->socket_fd, (struct sockaddr *)&client_addr, &socklen);
    if ( client < 0)
        throw std::runtime_error("accept error");// I need to add check if should exit or not 
    this->setNonBlocking(client);
    event.events = EPOLLIN ;
    event.data.fd = client;
    if (epoll_ctl(epoll, EPOLL_CTL_ADD, client, &event) < 0)
        throw std::runtime_error("can not add client to epoll\n");
    request[client] = "";
    this->clients.push_back(client);
}

void Server::removeClient(int client , int epoll)
{
    if (!this->client_is_here(client))
      return;
    this->clients.erase(
            std::remove(this->clients.begin(), this->clients.end(), client),
            this->clients.end());
    if (epoll_ctl(epoll,EPOLL_CTL_DEL,client, NULL) < 0)
      throw std::runtime_error("can not remove client from interest files");
    close(client);
}

std::string Server::getIpPort()
{
    return (this->ip + ":" + this->port);
}

bool Server::client_is_here(int client)
{
  return (std::find(clients.begin(),clients.end(), client ) != clients.end());
}

void Server::set_in_epoll(int epoll)
{
  std::cout << this->socket_fd << "\n";

  if (epoll_ctl(epoll, EPOLL_CTL_ADD, this->socket_fd, &this->ev) < 0)
  {
     std::string msg = "cannot add server {" + this->getIpPort() + "} to epoll: ";
     msg += strerror(errno);
     throw std::runtime_error(msg);
  }
}

int Server::getSocketFD() const
{
  return this->socket_fd;
}
Server::~Server()
{
    if (this->res){
        freeaddrinfo(this->res);
        this->res = NULL;
    }
    if (this->socket_fd >= 0 )
    {
        close(this->socket_fd);
        this->socket_fd = -1;
    }
}
