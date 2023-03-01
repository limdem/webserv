#ifndef CLUSTER_HPP
#define CLUSTER_HPP

#include "webserv.hpp"
/*
#include "Server.hpp"
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <cstring>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
typedef struct sockaddr SOCKADDR;
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
*/





class   Cluster
{
    public:
        Cluster();
        Cluster(const Cluster & A);
        ~Cluster();

        Cluster &   operator=(const Cluster & A);

        //getters
        std::vector<std::string>    get_fileVector() const;
        std::vector<Server>     get_servers() const;

        void                    put_in_vector(std::string filepath);
        int                     config();
        void                    print_cluster();
        int                     check_point_virgule();


    private:
        std::vector<std::string>    _fileVector;
        std::vector<Server>         _servers;
};
















#endif