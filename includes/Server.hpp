#ifndef SERVER_HPP
#define SERVER_HPP

#include "webserv.hpp"



class   Server
{
    public:
        Server();
        Server(const Server & A);
        ~Server();

        Server &   operator=(const Server & A);

        //getters
        std::string                     get_name() const;
        int                             get_port() const;
        std::string                     get_ip() const;
        std::vector<Location>           get_locations() const;
        std::map<int, std::string>      get_error_page() const;
        std::string                     get_root() const;
        int                             get_client_body_size() const {return _clientBodySize;}
        

        unsigned int    parse(std::vector<std::string> fileVector, unsigned int i);
        void            parse_error_pages(std::vector<std::string>);
        void            parse_client_body_size(std::vector<std::string>);
        void            parse_allow_methods(std::vector<std::string>);
        unsigned int    parse_location(std::vector<std::string> fileVector,unsigned int i);
        void            print_server();



    private:
        std::string                 _name;
        std::string                 _ip;
        int                         _port;
        std::string                 _root;
        std::string                 _index;
        std::map<int, std::string>  _errorPages;
        unsigned int                _clientBodySize;
        std::vector<std::string>    _allowedMethods;
        std::string                 _redirection;
        std::vector<Location>       _locations;
        std::string                  _autoIndex;

};

#endif