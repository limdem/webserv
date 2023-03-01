#include "../includes/webserv.hpp"

Server    AlgoServer::run()
{
    int             request_port = _request.get_port();
    std::string     request_ip = _request.get_ip();
    std::string     request_host = _request.get_host();
    
    std::vector<Server>  servers = this->get_cluster().get_servers();
 
    for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); it++)
    {
        if (it->get_port() != request_port)
        {
            it = servers.erase(it);
            it--;
        }
    }
    
    for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); it++)
    {
        if (it->get_ip() != request_ip && it->get_ip() != "0.0.0.0")
        {
            it = servers.erase(it);
            it--;
        }
    }

    
    int exact_ip = 0;
    for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); it++)
    {
        if (it->get_ip() == request_ip)
            exact_ip = 1;
    }

    if (exact_ip)
    {
        for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); it++)
        {
            if (it->get_ip() != request_ip)
            {
                it = servers.erase(it);
                it--;
            }
        }
    }

    for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); it++)
    {
        if (it->get_name() == request_host)
            return (*it);
    }

    int max = 0;
    int temp = 0;
    for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); it++)
    {
        temp = end_equality(it->get_name(), request_host);
        if (max < temp)
            max = temp;
    }
    if (max != 0)
    {
        for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); it++)
        {
        temp = end_equality(it->get_name(), request_host);
        if (max == temp)
            return (*it);
        }
    }

    max = 0;
    for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); it++)
    {
        temp = begin_equality(it->get_name(), request_host);
        if (max < temp)
            max = temp;
    }
    if (max != 0)
    {
        for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); it++)
        {
        temp = begin_equality(it->get_name(), request_host);
        if (max == temp)
            return (*it);
        }
    }
    if (servers.empty())
    {
        std::vector<Server>  servers2 = this->get_cluster().get_servers();
        return (*servers2.begin());
    }
    else
        return (*servers.begin());

}
