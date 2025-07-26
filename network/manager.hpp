#ifndef MANAGER_HPP
#define MANAGER_HPP
#include "ServerBlock.hpp"
#include "../header/class_header.hpp"
#include "server.hpp"

enum s_flag{
  SERVER,
  CLIENT
};

typedef struct s_NoName{
  s_flag flag;
  int index;
}t_noName;


class Manager
{
    private:
        std::vector<ServerBlock> data;
        std::vector<Server> servers;
        struct epoll_event ev[MAX_EVENT];
        int epoll;
    public:
        Manager(std::vector<ServerBlock> ser_);
        ~Manager();
        void epoll_management();
        void add_all_servers_to_epoll();
        int is_serve();
        void start_server();
        t_noName server_or_client(int fd);
        int index_of_server(int fd_client);
};

#endif
