#include "../includes/webserv.hpp"


Server::Server():_name(""),_ip("0.0.0.0"),_port(80),_clientBodySize(16000)
{
    std::vector<std::string> methods;
    methods.push_back("GET");
    methods.push_back("POST");
    methods.push_back("DELETE");
    _allowedMethods = methods;
}
Server::Server(const Server & A){*this = A;}
Server::~Server(){}
Server &   Server::operator=(const Server & A)
{
    _name = A._name;
    _ip = A._ip;
    _port = A._port;
    _root = A._root;
    _index = A._index;
    _errorPages = A._errorPages;
    _clientBodySize = A._clientBodySize;
    _allowedMethods = A._allowedMethods;
    _redirection = A._redirection;
    _locations = A._locations;
    _autoIndex = A._autoIndex;
    return (*this);
}



void            Server::parse_error_pages(std::vector<std::string> lineVector)
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
        {
            value = lineVector[i];
            value.erase(value.length() - 1);
        }
    }
    for (std::size_t i = 0; i < tab.size(); i++)
    {
        std::pair<int, std::string>   p(tab[i], value);
        _errorPages.insert(p);
    }
}

void            Server::parse_client_body_size(std::vector<std::string> lineVector)
{
    const char *c = lineVector[1].c_str();
    _clientBodySize = std::atoi(c) * 1000;
}

void            Server::parse_allow_methods(std::vector<std::string> lineVector)
{

    _allowedMethods.clear();
    for (std::size_t i = 1; i < lineVector.size(); i++)
    { 
      _allowedMethods.push_back(lineVector[i]);
    }
    _allowedMethods[_allowedMethods.size() - 1].erase(_allowedMethods[_allowedMethods.size() - 1].length() - 1);
}

unsigned int   Server::parse_location(std::vector<std::string> fileVector,unsigned int i)
{
    Location    location;

    location.set_root(_root);
    location.set_index(_index);
    std::vector<std::string>    lineVector;    
    lineVector = split(fileVector[i]);

    if (lineVector.size() == 4)
    {
        location.set_prefix(lineVector[1]);
        location.set_title(lineVector[2]);
    }
    else if (lineVector.size() == 3)
    {
        location.set_prefix("");
        location.set_title(lineVector[1]);
    }
    else
    {
        std::cout << "error in the configuration location\n";
        exit (1);
    }
    i++;
    i = location.parse(fileVector, i);
    if (!check_extension(location.getCgiPass()))
    {
        std::cout << "erreur dans l'extension d'un fichier\n";
        return 0;
    }

    /*int extension = check_good_extension(location.get_title());

    if (location.getCgiPass().size() != 0 && !extension)
    {
        std::cout << "erreur dans l'extension d'un fichier\n";
        return 0;
    }*/
    if (location.getCgiPass() == ".php")
    {
        std::ifstream binaryFile;
        binaryFile.open("/usr/bin/php-cgi");
        if (!binaryFile)
        {
            std::cout << "erreur: le binaire php-cgi n'existe pas" << std::endl;
            return 0;
        }
        location.set_binary_file("/usr/bin/php-cgi");
        binaryFile.close();

    }

    if (location.getCgiPass() == ".py")
    {
        std::ifstream binaryFile;
        binaryFile.open("/usr/bin/python3");
        if (!binaryFile)
        {
            std::cerr << "erreur: le binaire python3 n'existe pas" << std::endl;
            return 0;
        }
        location.set_binary_file("/usr/bin/python3");
        binaryFile.close();
    }

    if (location.getCgiPass() == ".pl")
    {
        std::ifstream binaryFile;
        binaryFile.open("/usr/bin/perl");
        if (!binaryFile)
        {
            std::cerr << "erreur: le binaire perl n'existe pas" << std::endl;
            return 0;
        }
        location.set_binary_file("/usr/bin/perl");
        binaryFile.close();
    }

    _locations.push_back(location);
    return i;
}

unsigned int    Server::parse(std::vector<std::string> fileVector, unsigned int i)
{
    while (fileVector[i] != "}")
    {
        std::vector<std::string>    lineVector;
        lineVector = split(fileVector[i]);

        if (lineVector[0] == "server_name")
        {
            lineVector[1].erase(lineVector[1].length() - 1);
            _name = lineVector[1];
        }
        else if (lineVector[0] == "listen")
        {
            std::vector<std::string> listen;
            listen = split2(lineVector[1], ':');
            if ((listen.size() == 1) && (listen[0].find('.') == std::string::npos))
            {
                const char *c = listen[0].c_str();
                _port = std::atoi(c);
            }
            else if ((listen.size() == 1) && (listen[0].find('.') != std::string::npos))
                _ip = listen[0]; 
            else
            {
                _ip = listen[0];
                const char *c = listen[1].c_str();
                _port = std::atoi(c);
            }
        }
        else if (lineVector[0] == "root")
        {
            lineVector[1].erase(lineVector[1].length() - 1);
            _root = lineVector[1];
        }
        else if (lineVector[0] == "index")
        {
            lineVector[1].erase(lineVector[1].length() - 1);
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
            lineVector[2].erase(lineVector[2].length() - 1);
            _redirection = lineVector[2];
        }
        else if (lineVector[0] == "location")
        {
            i = parse_location(fileVector, i);
            if (i == 0)
                return (0);
        }
        else if (lineVector[0] == "autoindex")
        {
            lineVector[1].erase(lineVector[1].length() - 1);
            _autoIndex = lineVector[1];
        }
            
        else 
        {
            if (lineVector[0] != "\n")
            {
                std::cout << "error in the configuration file: " << lineVector[0] << std::endl;
                return (0);
            }
        }
        i++;
    }
    return i;
}

void    Server::print_server()
{
    std::cout << "serveur name : " << _name << std::endl;
    std::cout << "ip       : " << _ip << std::endl;
    std::cout << "port       : " << _port << std::endl;
    std::cout << "root      : " << _root << std::endl;
    std::cout << "index      : " << _index << std::endl;
    
   for (std::map<int,std::string>::iterator it = _errorPages.begin(); it != _errorPages.end(); it++)
    {
        std::cout << "error pages : number : " << it->first << std::endl;
        std::cout << "error_page : uri :"  << it->second << std::endl;
    }
    
    std::cout << "client_body_size : "  << _clientBodySize << std::endl;

    for (std::size_t    i = 0; i < _allowedMethods.size(); i++)
        std::cout << "allow_methods : "  << _allowedMethods[i] << std::endl;

    std::cout << "redirection : "  << _redirection << std::endl;

    std::cout << "autoindex : "  << _autoIndex << std::endl;
    std::cout << "------------ fin de l'affichage du server --------------\n";

    //for (int i = 0; i < _locations.size(); i++)
    //    _locations[i].print_location();
}

        std::string                     Server::get_name() const {return _name;}
        int                             Server::get_port() const {return _port;};
        std::string                     Server::get_ip() const {return _ip;}
        std::vector<Location>           Server::get_locations() const {return _locations;}
        std::map<int, std::string>      Server::get_error_page() const {return _errorPages;}
        std::string                     Server::get_root() const {return _root;}
