#include "LocationBlock.hpp"


void  LocationBlock::just_this_error()
{
  std::cout << "network/LocationBlock.hpp:4:1: error: expected initializer before ‘class’";
}

LocationBlock::LocationBlock()
{}

// LocationBlock& LocationBlock::LocationBlock(LocationBlock const & obj)
// {
//   (void )obj;
//   return (LocationBlock &)obj;                  
// }

LocationBlock::LocationBlock(const LocationBlock& obj) {
  (void) obj;
}

LocationBlock& LocationBlock::operator=(const LocationBlock& obj) {
    if (this != &obj) {
        // Copy fields from obj to *this
        // Example:
        // this->someField = obj.someField;
    }
    return *this;
}

LocationBlock::~LocationBlock()
{}
