#ifndef MULTIPARTPARSER_HPP
# define MULTIPARTPARSER_HPP

#include "webserv.hpp"

class   MultipartParser
{
private:
    bool    _valid_part;
    bool    _first_part;
    char    *_tmp_header;
    const char  *_request_body;
    std::size_t _body_length;
    std::string _boundary;
    

    FILE    *_file_request_body;
    
    

    std::vector<MultipartPart>  _parts;
    std::vector<std::pair<std::size_t, std::size_t> >   _headers_start_end;
    std::vector<std::pair<std::size_t, std::size_t> >   _bodies_start_end;

    std::vector<char*>  _tmp_header_vect;
    
    std::size_t _header_start;
    std::size_t _header_end;
    std::size_t _body_start;
    std::size_t _body_end;

    
   
    int primary_check(std::size_t index);
    
public:
    MultipartParser();
    MultipartParser(const char*, std::size_t, std::string);
    MultipartParser(FILE*, std::size_t, std::string);
    ~MultipartParser();

    std::size_t find_boundary_start(std::size_t);
    std::size_t parse_part(std::size_t);
    
    void    parse_parts_header(std::size_t, std::size_t, std::size_t, std::size_t);
    int    check_upload_validity(std::vector<MultipartPart>);
    int     get_content_disposition_header(std::size_t*);
    int further_validity_checks(std::size_t);

    std::size_t getBodyLength() const;
    void setBodyLength(std::size_t);
    std::string getBoundary() const;
    void    setBoundary(std::string);
    const char  *getRequestBody() const;
    void    setRequestBody(const char*);
    FILE    *getFileRequestBody() const;

    std::vector<std::pair<std::size_t, std::size_t> >   getHeadersStartEnd() const;
    std::vector<std::pair<std::size_t, std::size_t> >   getBodiesStartEnd() const;
    std::vector<MultipartPart>  getParts() const;
};

#endif