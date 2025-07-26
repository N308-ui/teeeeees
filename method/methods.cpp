#include "../network/server.hpp"
#include "../header/class_header.hpp"




int Server::mathch_location(std::string path)
{
    const std::vector<LocationBlock>& locations = this->data.getLocations();
    int max = -1;
    for(size_t i = 0 ; i < locations.size(); i++)
    {
        if (locations[i].getPath() == path)
            return i;
        int index = longPrefix(locations[i].getPath(), path );
        if (path[index] == '/' && max < index)
            max = i;
    }
    
    return max;
}

e_Status status(std::string & path)
{
    struct stat s;
    if(stat(path.c_str(), &s) == -1)
        return _FAILER;
    switch (s.st_mode &S_IFMT )
    {
    case S_IFREG: return _FILE;
        break;
    case S_IFDIR:  return _DIR;
        break;
    default: return _UNKNOW;
        break;
    }
}
bool pathExists(const char * path)
{
    return (access(path, F_OK) == 0);
}

std::string Server::listFile(std::string path)
{
    DIR* dir = opendir(path.c_str());
    struct  dirent* a;
    std::string message = "<html><head></head><body>";

    while ((a = readdir(dir)) != NULL)
    {
        std::string name(a->d_name);
        if (name == "." || name == "..")
            continue;

        message += "<li><a href=\"" +  name + "\">" + name + "</a></li>";
    }
    message += "</body></html>";
    std::cout << message << std::endl;
    return message;
    
}






