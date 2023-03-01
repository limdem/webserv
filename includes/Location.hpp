#ifndef LOCATION_HPP
#define LOCATION_HPP

#include "webserv.hpp"

class   Location
{
    public:
        Location();
        Location(const Location & A);
        ~Location();

        Location &   operator=(const Location & A);

        //getters
        std::string     get_prefix() const;
        std::string     get_title() const;

        //setters
        void    set_root(std::string root);
        void    set_prefix(std::string pref);
        void    set_title(std::string title);
        void    set_binary_file(std::string binary);
        void    set_status(std::string status);
        void    set_index(std::string index) {_index = index;}
        void    set_client_max_body_size(int size) {_clientBodySize = size;}

        unsigned int    parse(std::vector<std::string> fileVector, unsigned int i);
        void            parse_error_pages(std::vector<std::string>);
        void            parse_client_body_size(std::vector<std::string>);
        void            parse_allow_methods(std::vector<std::string>);
        void            parse_cgi_param(std::vector<std::string>);
        void            print_location();

        std::string getCgiPass() const;
        std::string getRootDirectory() const;
        std::string getIndex() const;
        std::vector<std::string> getAllowedMethods() const;
        std::string                 get_binary_file() const;
        std::string                 get_redirection() const;
        std::string                 get_status() const;
        std::string                 get_autoIndex() const;
        std::string                 get_upload_store() const;
        std::size_t                 get_client_body_size() const;

    private:
        std::string                 _title;
        std::string                 _prefix;
        std::string                 _root;
        std::string                 _index;
        std::string                 _alias;
        std::map<int, std::string>  _errorPages;
        std::size_t                _clientBodySize;
        std::vector<std::string>    _allowedMethods;
        std::string                 _redirection;
        std::string                 _autoIndex;
        std::string                 _cgi_pass;
        std::map<std::string,std::string>   _cgi_params;
        std::string                 _upload_store;
        std::string                 _binary_file;
        std::string                 _status;

};

#endif