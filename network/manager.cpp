#include "manager.hpp"
#include "../header/class_header.hpp"
Manager::Manager(std::vector<ServerBlock> ser_)
   : data(ser_)
{

}

void Manager::start_server()
{
    for (int i = 0; i < (int)data.size(); i++)
        servers.push_back(Server(data[i]));

    for (int i = 0; i < (int)data.size(); i++)
    {
        try{
            servers[i].setup();
        }catch(std::exception& e)
        {
            std::cout << e.what();
            servers.erase(servers.begin()+ i);
            data.erase(data.begin() + i);
            i--;
        }
    }
}

void Manager::add_all_servers_to_epoll()
{
  for(int i = 0; i < (int)servers.size(); i++)
  {
    try
    {
      servers[i].set_in_epoll(this->epoll);
    }catch(std::exception& e)
    {
      std::cout << e.what();
      servers.erase(servers.begin() + i);
      data.erase(data.begin() + i);
      i--;
    }
  }
}

t_noName Manager::server_or_client(int fd)
{
  t_noName ok;
  std::memset(&ok,0, sizeof(ok));

  for (int i = 0;i < (int)servers.size(); i++)
  {
    if (servers[i].getSocketFD() == fd)
    {
      ok.flag = SERVER;
      ok.index = i;
      return ok;
    }
    if (servers[i].client_is_here(fd))
    {
      ok.flag = CLIENT;
      ok.index = i;
      return ok;
    }
  }
  return ok;
}

void Manager::epoll_management()
{
    this->epoll = epoll_create(1);
    if (this->epoll == -1)
    {
      std::cout << "cant create epoll" << std::endl;
      exit (1);
    }
    this->add_all_servers_to_epoll(); 
    while (1) {
      int files = epoll_wait(this->epoll, &this->ev[0], MAX_EVENT, 1000);
      for(int i = 0; i < files; i++)
      {
        t_noName ok = this->server_or_client(this->ev[i].data.fd); // TO DO
        if (ok.flag == SERVER)
        {
          this->servers[ok.index].addClient(this->epoll);
        }
        else {
            this->servers[ok.index].run(this->ev[i], this->epoll);
        }
      }
    }
}


Manager::~Manager()
{

}
