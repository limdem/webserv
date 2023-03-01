#include "../includes/webserv.hpp"



Cluster::Cluster(){}
Cluster::Cluster(const Cluster & A){*this = A;}
Cluster::~Cluster(){}

Cluster &   Cluster::operator=(const Cluster & A)
{
    _fileVector = A.get_fileVector();
    _servers = A._servers;
    return (*this);
}

//getters
std::vector<std::string>   Cluster::get_fileVector() const {return _fileVector;}
std::vector<Server>     Cluster::get_servers() const { return _servers;}




void   Cluster::put_in_vector(std::string filepath)
{
    std::string                 line;
    std::ifstream file(&filepath[0]);  //On essaye d'ouvrir le fichier
    if(!file)
    {
        std::cout << "ERREUR: Impossible d'ouvrir le fichier." << std::endl;
        exit (1);
    }
     while(std::getline(file, line)) //Tant qu'on n'est pas à la fin, on lit
      {
         _fileVector.push_back(line);
      }
}

int   Cluster::config()
{
    unsigned int size;
    size = _fileVector.size();

    for (unsigned int i = 0; i < size; i++)
    {
        if (_fileVector[i] == "server")
        {
            i++;
            if (_fileVector[i] != "{")
            {
                std::cout << RED << "Error: expected '{' after server directive." << RESET << std::endl;
                return (0);
            }
            i++;
            Server server;
            i = server.parse(_fileVector, i);
            if (i == 0)
                return (0);
            _servers.push_back(server);
            i++;
        }
        else if (_fileVector[i] == "server {")
        {
            i++;
            Server server;
            i = server.parse(_fileVector, i);
            if (i == 0)
                return (0);
            _servers.push_back(server);
            i++;
        }
        else if (_fileVector[i] == "\0")
            i++;
        else 
        {
            std::cout << RED << "Error: unknown directive [" << _fileVector[i] << "]" << RESET << std::endl;
			return 0; 
        }
    }
    return 1;
}

void    Cluster::print_cluster()
{
    for (std::size_t i = 0; i < _servers.size(); i++)
    {
        _servers[i].print_server();
        //std::cout << "---------fin du serveur---------\n";
    }
}

int Cluster::check_point_virgule()
{
    unsigned int size;
    size = _fileVector.size();

    for (unsigned int i = 0; i < size; i++)
    {
        unsigned int length;
        length = _fileVector[i].size();
        if (_fileVector[i] != "\0" && _fileVector[i][length - 1] != ';' && _fileVector[i][length - 1] !='{'
            && !is_closed_chevron(_fileVector[i]) && _fileVector[i].substr(0,6) != "server")
            return (0);
    }
    return 1;
}
