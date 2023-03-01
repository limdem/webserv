#include "../includes/webserv.hpp"

AlgoLocation::AlgoLocation(){}

AlgoLocation::AlgoLocation(Server server, Request request)
    :_server(server), _request(request)
{}

AlgoLocation::AlgoLocation(const AlgoLocation & A)
{*this = A;}

AlgoLocation::~AlgoLocation(){}

AlgoLocation& AlgoLocation::operator=(const AlgoLocation & A)
{
    _server = A._server;
    _request = A._request;
    return (*this);
}

Location    AlgoLocation::run()
{

    std::vector<Location> prefix;
    std::vector<Location> regularExp;
    std::vector<Location>  locs(_server.get_locations());

    std::string uri = _request.get_path();

    int size = locs.size();

    for (int i = 0; i < size; i++)
    {
        if (locs[i].get_prefix() == "" || locs[i].get_prefix() == "=" || locs[i].get_prefix() == "^~")
            prefix.push_back(locs[i]);
        else
            regularExp.push_back(locs[i]);
    }

    for (size_t i = 0; i < prefix.size(); i++)
    {
        if (prefix[i].get_title() == uri && prefix[i].get_prefix() == "=")
            return (prefix[i]);
    }

    int record = 0;
    int nb = 0;

    for (size_t i = 0; i < prefix.size(); i++)
    {
        int length = prefix[i].get_title().size();

        if (uri.compare(0, length, prefix[i].get_title()) == 0 && prefix[i].get_prefix() != "=")
        {
            if (record < length)
            {
                record = length;
                nb = i;
            }
        }
    }
    if (prefix[nb].get_prefix() == "^~")
        return (prefix[nb]);
    
    int     path_size = uri.size();

    for(size_t i = 0; i < regularExp.size(); i++)
    {
        int             n = regularExp[i].get_title().size();
        std::string     str = regularExp[i].get_title().substr(1, n - 2);
        int             len = str.size();
        std::string     str2 = str;
        std::string     casse = uri;

        if (regularExp[i].get_prefix() == "~*")
        {
            std::transform(casse.begin(), casse.end(), casse.begin(), tolower);
            std::transform(str2.begin(), str2.end(), str2.begin(), tolower);
        }

        if (casse.compare(path_size - len, len, str2) == 0)
            return (regularExp[i]);
    }
    
    return (prefix[nb]);

}

Server         AlgoLocation::get_server() const
{return _server;}

Request      AlgoLocation::get_request() const
{return _request;}

void        AlgoLocation::set_request(Request & request)
{_request = request;}
