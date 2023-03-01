CC = c++
RM = rm -f
CFLAGS = -Wall -Werror -Wextra -std=c++98
NAME = webserv
SRCS = algolocation.cpp\
	AlgoServer.cpp\
	cgi_handler.cpp\
	Cluster.cpp\
	Location.cpp\
	main.cpp\
	MultipartParser.cpp\
	MultipartPart.cpp\
	request_execution.cpp\
	request_parser.cpp\
	Response_buffers.cpp\
	Server.cpp\
	static_response.cpp\
	utils.cpp

SRCS_PATH = $(addprefix srcs/, $(SRCS))
OBJS = $(SRCS_PATH:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) -o $(NAME) $(OBJS)

%.o: %.cpp
	$(CC) -g $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

re : clean all