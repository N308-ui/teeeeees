CXX = c++ 
CXXFLAGS = -Wall -Wextra -Werror  -std=c++98 
NAME = HttpServer

FILECPP = $(wildcard *.cpp)\
			$(wildcard network/*.cpp)\
			$(wildcard helper_function/*.cpp)\
			$(wildcard method/*.cpp)

OBJCXX = $(FILECPP:.cpp=.o)

all: $(NAME)

$(NAME) : $(OBJCXX)
	$(CXX) $(CXXFLAGS) $(OBJCXX)  -o $(NAME)
	
%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean: 
	rm -rf $(OBJCXX)

fclean: clean 
	rm -rf $(NAME)
re: fclean all   
