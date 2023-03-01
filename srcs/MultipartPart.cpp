#include "../includes/webserv.hpp"

MultipartPart::MultipartPart()
{}

MultipartPart::MultipartPart(const char* str)
:_header(str)
{}

MultipartPart::~MultipartPart()
{}

void    MultipartPart::parse_header()
{
    std::size_t i = 0;
    
    while (i < _header.size() - 1)
    {
        std::string field_name;
        std::string field_value;
        
        std::size_t found0 = _header.find("\r\n", i);
 
        std::size_t found1 = _header.find(':', i);
        if (found1 != std::string::npos && found1 < found0)
            i = found1 + 1;
        std::size_t found2 = _header.find(';', i);
        if (found2 == std::string::npos)
            found2 = _header.find("\r\n", i);
        std::size_t found3 = _header.find('=', i);
        if (found3 < found2)
        {
            for (; i < found3; i++)
            {
                if (_header[i] != ' ')
                    break;
            }
            field_name = _header.substr(i, found3 - i);
            i = found3 + 1;
            for (; i < found2; i++)
            {
                if (!std::isalnum(_header[i]))
                    break;
            }
            found3++;
            field_value = _header.substr(i, found2 - i);
        }
        else
        {
            for (; i < found2; i++)
            {
                if (_header[i] != ' ')
                    break;
            }
            field_name = _header.substr(i, found2 - i);
        
        }
        _headers_fields.push_back(std::pair<std::string, std::string>(field_name, field_value));
        i = found2 + 1;   
    }
}

std::vector<std::pair<std::string, std::string> >   MultipartPart::getHeaders() const
{
    return _headers_fields;
}

std::size_t MultipartPart::getBodyStart() const
{
    return _body_start;
}

void    MultipartPart::setBodyStart(std::size_t body_start)
{
    _body_start = body_start;
}

std::size_t MultipartPart::getBodyEnd() const
{
    return _body_end;
}

void    MultipartPart::setBodyEnd(std::size_t body_end)
{
    _body_end = body_end;
}