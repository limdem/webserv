#ifndef ALGOLOCATION_HPP
#define ALGOLOCATION_HPP

#include "webserv.hpp"
/*
#include "Cluster.hpp"
#include "Request.hpp"
#include "Server.hpp"
#include <algorithm>
*/


class   AlgoLocation
{
    public:
        AlgoLocation();
        AlgoLocation(Server server, Request request);
        AlgoLocation(const AlgoLocation & A);
        ~AlgoLocation();

        AlgoLocation&   operator=(const AlgoLocation & A);


        //getters
        Server          get_server() const;
        Request         get_request() const;

        void            set_request(Request & request);
        Location        run();


    private:
        Server                  _server;
        Request                 _request;

};

#endif