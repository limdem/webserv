#ifndef ALGOSERVER_HPP
#define ALGOSERVER_HPP

#include "webserv.hpp"

class   AlgoServer
{
    public:
        AlgoServer(){};
        AlgoServer(Cluster cluster, Request request):
            _cluster(cluster), _request(request){}
        AlgoServer(const AlgoServer & A){*this = A;}
        ~AlgoServer(){}

        AlgoServer &   operator=(const AlgoServer & A)
        {
            _cluster = A._cluster;
            _request = A._request;
            return (*this);
        }

        //getters
        Cluster     get_cluster(){return _cluster;}
        Request     get_request(){return _request;}

        Server    run();


    private:
        Cluster                 _cluster;
        Request                 _request;

};

#endif