#include "my_lib.hpp"


std::string to_string(int number) {
       std::stringstream ss;
       ss << number;
       std::string str_number = ss.str();
       return str_number;
}