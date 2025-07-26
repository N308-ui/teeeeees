#ifndef CLASS_HEADER_HPP
#define CLASS_HEADER_HPP


#define MAX_EVENT 1000

#include <iostream>
#include <string>
#include <cstring>
#include <algorithm> 
#include <unistd.h>
#include <fcntl.h>

#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/un.h>
#include <sys/types.h>
#include <exception>
#include <netinet/in.h>
#include <netdb.h> 
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h> 

#include <vector>
#include <map>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include "../helper_function/my_lib.hpp"

enum e_Status {
    _FILE,
    _DIR,
    _UNKNOW,
    _FAILER,

    
};

#endif 
