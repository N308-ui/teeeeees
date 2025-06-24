
#include "header.hpp"

int main(int ac, char **av)
{

   if (ac != 2) {
         std::cerr << "Usage: " << av[0] << " <config_file>" << std::endl;
         return 1;
    }

    ConfigParser parser(av[1]);

    if (!parser.parse()) {
        std::cerr << "Failed to parse configuration file" << std::endl;
        return 1;
    }

    std::cout << "Successfully parsed configuration file" << std::endl;
    Manager http(parser.getServers()); 
    try
    {
        http.start_server();
        http.epoll_management();
    }
    catch(const std::runtime_error& e)
    {
        std::cerr << e.what();
    }
    return 0;
}
