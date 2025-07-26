#include "my_lib.hpp"


int longPrefix(std::string local, std::string link)
{
    if (local == "/")
        return 0;
    int i =0;
    for ( ; local[i] == link[i] ; i++);
    return i;
}