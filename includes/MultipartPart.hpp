#ifndef MULTIPARTPART_HPP
# define MULTIPARTPART_HPP
/*
# include <vector> 
# include <string> 
# include <utility>
# include <iostream>
*/

#include "webserv.hpp"

class MultipartPart
{
private:
    std::vector<std::pair<std::string, std::string> >   _headers_fields;
    std::size_t _body_start;
    std::size_t _body_end;
    std::string _header;

public:
    MultipartPart();
    MultipartPart(const char*);
    ~MultipartPart();


    void    parse_header();
    std::vector<std::pair<std::string, std::string> >   getHeaders() const;

    std::size_t getBodyStart() const;
    void    setBodyStart(std::size_t);
    std::size_t getBodyEnd() const;
    void    setBodyEnd(std::size_t);
};



#endif