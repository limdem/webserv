#include "../includes/webserv.hpp"

MultipartParser::MultipartParser()
    :_valid_part(true), _first_part(true), _tmp_header(NULL)
{}

MultipartParser::MultipartParser(const char* request_body, std::size_t body_length, std::string boundary)
    :_valid_part(true), _first_part(true), _tmp_header(NULL),
    _request_body(request_body), _body_length(body_length), _boundary("--" + boundary)
{}

MultipartParser::MultipartParser(FILE* file_request_body, std::size_t body_length, std::string boundary)
    :_valid_part(true), _first_part(true), _tmp_header(NULL),
    _body_length(body_length), _boundary("--" + boundary), _file_request_body(file_request_body)
{}

MultipartParser::~MultipartParser()
{
    if (_tmp_header)
    {
        delete [] _tmp_header;
    }
    for (std::vector<char*>::const_iterator it = _tmp_header_vect.begin(); it != _tmp_header_vect.end(); it++)
    {
        if (*it != _tmp_header)
            delete [] *it;
    }
}

int MultipartParser::primary_check(std::size_t index)
{
    if (!std::strncmp(getRequestBody() + index + _boundary.length(), "\n", 1))
        return 1;
    if (!std::strncmp(getRequestBody() + index + _boundary.length(), "\r\n", 2))
        return 2; 
    return 0;
}

int MultipartParser::further_validity_checks(std::size_t index)
{
    if (!std::strncmp(getRequestBody() + index - 1, "\n", 1))
        return 0;
    return 1;
}

std::size_t MultipartParser::find_boundary_start(std::size_t index)
{
    std::size_t add_index;

    for (; index < _body_length; index++)
    {
        if (!std::strncmp(getRequestBody() + index, _boundary.c_str(), _boundary.length()))
        {
            add_index = primary_check(index);
            if (_first_part)
            {
                if (add_index)
                {
                    _first_part = false;
                    return index + add_index + _boundary.length();
                }
            }
            else
            {
                if (add_index && !further_validity_checks(index))
                    return index + add_index + _boundary.length();
            }
        }        
    }
    return _body_length;
}

std::size_t MultipartParser::parse_part(std::size_t start)
{
    bool    found_body_start = false;
    bool    found_body_end = false;
    std::size_t next_boundary;
    std::string content_disposition;

    next_boundary = start;
    bool    final_boundary = false;

    _header_start = start;
    if (start == _body_length)
        return _body_length;
    for (std::size_t    i = start; i < start + 20; i++)
        content_disposition += std::toupper(getRequestBody()[i]);
    if (std::strncmp(content_disposition.c_str(), "CONTENT-DISPOSITION:", 20))
    {
        return next_boundary;
    }
    start += 21;
    for (; !found_body_start && start < _body_length; start++)
    {
        if (!std::strncmp(getRequestBody() + start, "\n\r\n", 3))
        {
            found_body_start = true;
            _header_end = start + 1;
            start += 2;
        }
        else if (!std::strncmp(getRequestBody() + start, "\n\n", 2))
        {
            found_body_start = true;
            _header_end = start + 1;
            start += 1;
        } 
    }
    _body_start = start;
    for (; !found_body_end && start < _body_length; start++)
    {
    
        if (!std::strncmp(getRequestBody() + start, _boundary.c_str(), _boundary.length()) && !std::strncmp(getRequestBody() + start - 1, "\n", 1))
        {
            found_body_end = true;
            if (!std::strncmp(getRequestBody() + start + _boundary.length(), "--", 2))
                final_boundary = true;
        }   
    }
    next_boundary = start - 1;
    _body_end = next_boundary;
    if (getRequestBody()[_body_end - 1] == '\n')
        _body_end--;
    if (getRequestBody()[_body_end - 1] == '\r')
        _body_end--;
    _headers_start_end.push_back(std::pair<std::size_t, std::size_t>(_header_start, _header_end));
    _bodies_start_end.push_back(std::pair<std::size_t, std::size_t>(_body_start, _body_end));
    if (final_boundary)
        return _body_length;
    return next_boundary;
}

void    MultipartParser::parse_parts_header(std::size_t start_header, std::size_t end_header, std::size_t start_body, std::size_t end_body)
{
    _tmp_header = new char[end_header - start_header + 1];
    _tmp_header_vect.push_back(_tmp_header);
    std::memcpy((void*)_tmp_header, getRequestBody() + start_header, end_header - start_header);
    _tmp_header[end_header - start_header] = 0;
    MultipartPart   part(_tmp_header);
    part.parse_header();
    part.setBodyStart(start_body);
    part.setBodyEnd(end_body);
    _parts.push_back(part);
}

int    MultipartParser::check_upload_validity(std::vector<MultipartPart> parts)
{
    std::vector<std::pair<std::string, std::string> >    headers;
    bool    filename;
    for (std::vector<MultipartPart>::const_iterator it1 = parts.begin(); it1 != parts.end(); it1++)
    {
        filename = false;
        headers = it1->getHeaders();
        for (std::vector<std::pair<std::string, std::string> >::const_iterator  it2 = headers.begin(); it2 != headers.end(); it2++)
        {
            if (it2->first == "filename")
            {
                filename = true;
                break;
            }
        }
        if (filename == false)
            return 0;
    }
    return 1;
}

std::size_t MultipartParser::getBodyLength() const
{
    return _body_length;
}

void MultipartParser::setBodyLength(std::size_t body_length)
{
    _body_length = body_length;
}

std::string MultipartParser::getBoundary() const
{
    return _boundary;
}

void    MultipartParser::setBoundary(std::string boundary)
{
    _boundary = boundary;
}

const char  *MultipartParser::getRequestBody() const
{
    return _request_body;
}

void    MultipartParser::setRequestBody(const char* request_body)
{
    _request_body = request_body;
}

FILE    *MultipartParser::getFileRequestBody() const
{
    return _file_request_body;
}

std::vector<std::pair<std::size_t, std::size_t> >   MultipartParser::getHeadersStartEnd() const
{
    return _headers_start_end;
}

std::vector<std::pair<std::size_t, std::size_t> >   MultipartParser::getBodiesStartEnd() const
{
    return _bodies_start_end;
}

std::vector<MultipartPart>  MultipartParser::getParts() const
{
    return _parts;
}