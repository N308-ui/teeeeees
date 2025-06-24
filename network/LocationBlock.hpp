#ifndef LOCATIONBLOCK_HPP
#define LOCATIONBLOCK_HPP
#include "../header/class_header.hpp"

class LocationBlock {
    public:
    LocationBlock();
    LocationBlock(LocationBlock const &obj);
  LocationBlock& operator=(const LocationBlock& obj);
    ~LocationBlock();
    void  just_this_error();
};

#endif
