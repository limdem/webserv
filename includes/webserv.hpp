#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <fcntl.h>
#include <time.h>
#include <sys/mman.h>
#include <dirent.h>

#include <sys/wait.h>
#include <algorithm>
#include <fcntl.h>
 #include <fcntl.h>
 #include <sys/types.h>
 #include <sys/stat.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <cstring>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

#include "Response_buffers.hpp"
#include "client_buffer.hpp"
#include "request_parser.hpp"
#include "MultipartPart.hpp"
#include "MultipartParser.hpp"
#include "static_response.hpp"
#include "cgi_handler.hpp"
#include "Location.hpp"
#include "Server.hpp"
#include "Cluster.hpp"
#include "Request.hpp"
#include "algolocation.hpp"
#include "AlgoServer.hpp"




#include "socket_class.hpp"



typedef struct sockaddr SOCKADDR;
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

//colors
# define RED "\033[31m"
# define GREEN "\033[32m"
# define YELLOW "\033[33m"
# define RESET "\033[0m"




std::vector<std::string>		split(const std::string& str);
std::vector<std::string>		split2(const std::string& str, char c);
int                             is_closed_chevron(std::string str);
void    handle_client_request(const char *);
int end_equality(std::string name, std::string requete_name);
int begin_equality(std::string name, std::string requete_name);
void    handle_request_execution(Request_parser& req_pars, Location& location_chosen, Static_response& rsp_hdl, int fd);
int		check_extension(std::string s);
int		check_good_extension(std::string s);
std::string			get_Link(std::string const &dirEntry, std::string const &dirName, std::string const &host, int port);
int		check_file(char *str);

#endif