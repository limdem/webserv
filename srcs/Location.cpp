#include "../includes/webserv.hpp"


Location::Location():_clientBodySize(0)
{
    std::vector<std::string> methods;
    methods.push_back("GET");
    methods.push_back("POST");
    methods.push_back("DELETE");
    _allowedMethods = methods;
}
Location::Location(const Location & A){*this = A;}
Location::~Location(){}
Location &   Location::operator=(const Location & A)
{
    _title = A._title;
    _prefix = A._prefix;
    _root = A._root;
    _index = A._index;
    _alias = A._alias;
    _errorPages = A._errorPages;
    _clientBodySize = A._clientBodySize;
    _allowedMethods = A._allowedMethods;
    _redirection = A._redirection;
    _autoIndex = A._autoIndex;
    _cgi_pass = A._cgi_pass;
    _cgi_params = A._cgi_params;
    _upload_store = A._upload_store;
    _binary_file = A._binary_file;
    _status = A._status;
    return (*this);
}

//getters


void            Location::parse_error_pages(std::vector<std::string> lineVector)
{
    int             key;
    std::string     value;

    std::vector<int> tab;

    for (std::size_t i = 1; i < lineVector.size(); i++)
    {
        if (lineVector[i][0] != '/')
        {
            const char *c = lineVector[i].c_str();
            key = std::atoi(c);
            tab.push_back(key);
        }
        else
            value = lineVector[i];
    }
    for (std::size_t i = 0; i < tab.size(); i++)
    {
        std::pair<int, std::string>   p(tab[i], value);
        _errorPages.insert(p);
    }
}

void            Location::parse_client_body_size(std::vector<std::string> lineVector)
{
    const char *c = lineVector[1].c_str();
    _clientBodySize = std::atoi(c) * 1000;
}

void            Location::parse_allow_methods(std::vector<std::string> lineVector)
{
    _allowedMethods.clear();
    for (std::size_t i = 1; i < lineVector.size(); i++)
    {
        _allowedMethods.push_back(lineVector[i]);
    }
    _allowedMethods[_allowedMethods.size() - 1].erase(_allowedMethods[_allowedMethods.size() - 1].length() - 1);
}

void            Location::parse_cgi_param(std::vector<std::string> lineVector)
{
    std::string A(lineVector[1]);
    std::string B(lineVector[2]);
    B.erase(B.length() - 1);
    std::pair<std::string,std::string>   p(A,B);
    _cgi_params.insert(p);
}


unsigned int    Location::parse(std::vector<std::string> fileVector, unsigned int i)
{
    _status = "200";

    //std::string::size_type pos = _title.find_last_of('/');
    //_index = _title.substr(pos + 1);
    int a = 0;
    int b = 0;

    while (!is_closed_chevron(fileVector[i]))
    {
        std::vector<std::string>    lineVector;
        
        lineVector = split(fileVector[i]);

        if (lineVector[0] == "root")
        {
            a = 1;
            lineVector[1].erase( lineVector[1].length() -1);
            _root = lineVector[1];
        }
        else if (lineVector[0] == "index")
        {
            b = 1;
            lineVector[1].erase( lineVector[1].length() -1);
            _index = lineVector[1];
        }
        else if (lineVector[0] == "error_page")
            parse_error_pages(lineVector);
        else if (lineVector[0] == "client_body_buffer_size")
            parse_client_body_size(lineVector);
        else if (lineVector[0] == "allow_methods")
            parse_allow_methods(lineVector);
        else if (lineVector[0] == "return")
        {
            lineVector[2].erase( lineVector[2].length() -1);
            _status = lineVector[1];
            _redirection = lineVector[2];
        }
        else if (lineVector[0] == "autoindex")
        {
            lineVector[1].erase( lineVector[1].length() -1);
            _autoIndex = lineVector[1];
        }
        else if (lineVector[0] == "alias")
        {
            lineVector[1].erase( lineVector[1].length() -1);
            _alias = lineVector[1];
        } 
        else if (lineVector[0] == "cgi_pass")
        {
            lineVector[1].erase( lineVector[1].length() -1);
            _cgi_pass = lineVector[1];
            if (_cgi_pass != ".php" && _cgi_pass != ".py" && _cgi_pass != ".pl")
            {
                std::cout << "erreur dans la configuration du cgi_pass" << std::endl;
                return (0);
            }
        }
        else if (lineVector[0] == "cgi_param")
            parse_cgi_param(lineVector); 
        else if (lineVector[0] == "upload_store")
        {
            lineVector[1].erase( lineVector[1].length() -1);
            _upload_store = lineVector[1];
        }
        else 
        {
            if (lineVector[0] != "\n")
            {
                std::cout << "error in the configuration file: " << lineVector[0] << std::endl;
                exit (1);
            }
        }
        i++;
    }
    if (!a && !b)
        _index = _title;
    return i;
}

void    Location::print_location()
{
    /*for (std::map<int,std::string>::iterator it = _errorPages.begin(); it != _errorPages.end(); it++)
    {
        std::cout << "--------error pages : number : " << it->first << std::endl;
        std::cout << "--------error_page : file_path :"  << it->second << std::endl;
    }*/
    
    //std::cout << "--------client_body_size : "  << _clientBodySize << std::endl;

    for (std::size_t i = 0; i < _allowedMethods.size(); i++)
        std::cout << "--------allow_methods : "  << _allowedMethods[i] << std::endl;

    std::cout << "--------redirection : "  << _redirection << std::endl;
    std::cout << "--------autoindex : "  << _autoIndex << std::endl;
    std::cout << "--------alias : "  << _alias << std::endl;
    std::cout << "--------cgi_pass : "  << _cgi_pass << std::endl;
    std::cout << "--------binary_file : "  << _binary_file << std::endl;

    for (std::map<std::string,std::string>::iterator it = _cgi_params.begin(); it != _cgi_params.end(); it++)
    {
        /*std::cout << "--------cgi_param : variable : " << it->first << std::endl;
        std::cout << "--------cgi_param : param :"  << it->second << std::endl;*/
    }

    std::cout << "--------upload_store : "  << _upload_store << std::endl;
    std::cout << "----------------fin----------------\n\n";
}

std::string Location::getCgiPass() const
{
    return _cgi_pass;
}

std::string Location::getRootDirectory() const
{
    return _root;
}

std::string Location::getIndex() const
{
    return _index;
}

std::vector<std::string> Location::getAllowedMethods() const
{
    return _allowedMethods;
}

std::string Location::get_binary_file() const
{
    return _binary_file;
}

std::string Location::get_redirection() const
{
    return _redirection;
}

std::string     Location::get_prefix() const
{
    return _prefix;
}

std::string     Location::get_title() const
{
    return _title;
}

void    Location::set_root(std::string root)
{
    _root = root;
}

void    Location::set_prefix(std::string pref)
{
    _prefix = pref;

}
void    Location::set_title(std::string title)
{
    _title = title;
}

void    Location::set_binary_file(std::string binary)
{
    _binary_file = binary;
}

void    Location::set_status(std::string status)
{
    _status = status;
}

std::string Location::get_status() const 
{
    return _status;
}

std::string Location::get_autoIndex() const
{
    return _autoIndex;
}

std::string Location::get_upload_store() const
{
    return _upload_store;
}

std::size_t Location::get_client_body_size() const
{
    return _clientBodySize;
}