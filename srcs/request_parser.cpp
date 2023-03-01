#include "../includes/webserv.hpp"

Request_parser::Request_parser()
    :_reserved_meta_variables(NULL)
{}

Request_parser::Request_parser(client_buffer* cl_buf,int fd)
    :_fd(fd), _request_headers_ptr(&_request_headers), _is_short_body(cl_buf->is_short_body),
    _is_long_body(cl_buf->is_long_body), _bytes_inside_body(cl_buf->bytes_inside_body), _file_long_body(cl_buf->_file_long_body), 
    _bytes_inside_file_body(cl_buf->size), 
    _request_body_char(cl_buf->short_body), _host_count(0), _body_buffer(cl_buf->short_body),
    _body_buffer_ptr(&_body_buffer), _is_cookie_present(0)
{
    if (cl_buf->is_short_header)
        _request_header = std::string(cl_buf->short_header, 0, cl_buf->bytes_inside_short_header);
    if (cl_buf->is_long_header > -1)
    {
        for (int i = 0; cl_buf->long_header[i]; i++)
            _request_header += std::string(cl_buf->long_header[i]);
    }
    const char *reserved_var[] = {
        "AUTH_TYPE", "REDIRECT_STATUS", "SERVER_NAME",
        "SERVER_PORT", "SERVER_ADDR", "REMOTE_PORT",
        "REMOTE_ADDR","SERVER_SOFTWARE", "GATEWAY_INTERFACE",
        "REQUEST_SCHEME", "SERVER_PROTOCOL", "DOCUMENT_ROOT",
        "DOCUMENT_URI", "REQUEST_URI", "SCRIPT_NAME",
        "REQUEST_METHOD", "QUERY_STRING", "SCRIPT_FILENAME",
        "PATH_INFO", "CGI_ROLE", "PHP_SELF",
        "REQUEST_TIME_FLOAT", "REQUEST_TIME"
    };
    _reserved_meta_variables = new const char*[22];
    for (int i = 0; i < 22; i++)
        _reserved_meta_variables[i] = reserved_var[i];
}

Request_parser::Request_parser(const Request_parser& src)
    :_request_headers(src._request_headers), _is_short_body(src._is_short_body), _is_long_body(src._is_long_body),_bytes_inside_body(src._bytes_inside_body),_file_long_body(src._file_long_body),
    _bytes_inside_file_body(src._bytes_inside_file_body),_request_body_char(src._request_body_char),_host_count(src._host_count), _request_content_length(src._request_content_length),
    _request_content_type(src._request_content_type),
    _request_http_version(src._request_http_version), _request_query_string(src._request_query_string),_request_path_info(src._request_path_info), 
    _request_script_name(src._request_script_name), _request_uri(src._request_uri), _request_method(src._request_method), _body_buffer(src._body_buffer),
    _fd_long_body(src._fd_long_body), _root_directory(src._root_directory), _index(src._index),
    _title(src._title),_request_hostname(src._request_hostname), _request_script_filename(src._request_script_filename),
    _is_cookie_present(src._is_cookie_present)
{
    _request_headers_ptr = &_request_headers;
    _body_buffer_ptr = &_body_buffer;
    const char *reserved_var[] = {
        "AUTH_TYPE", "REDIRECT_STATUS", "SERVER_NAME",
        "SERVER_PORT", "SERVER_ADDR", "REMOTE_PORT",
        "REMOTE_ADDR","SERVER_SOFTWARE", "GATEWAY_INTERFACE",
        "REQUEST_SCHEME", "SERVER_PROTOCOL", "DOCUMENT_ROOT",
        "DOCUMENT_URI", "REQUEST_URI", "SCRIPT_NAME",
        "REQUEST_METHOD", "QUERY_STRING", "SCRIPT_FILENAME",
        "PATH_INFO", "CGI_ROLE", "PHP_SELF",
        "REQUEST_TIME_FLOAT", "REQUEST_TIME"
    };
    _reserved_meta_variables = new const char*[22];
    for (int i = 0; i < 22; i++)
        _reserved_meta_variables[i] = reserved_var[i];
}

Request_parser& Request_parser::operator=(const Request_parser& other)
{
    Request_parser  tmp(other);
    swap(*this, tmp);
    return *this;
}

Request_parser::~Request_parser()
{
    delete[] _reserved_meta_variables;   
}

void    Request_parser::parse_request_line(std::string line)
{
    std::size_t found0 = _request_header.find("\r\n");
    if (found0 == std::string::npos)
        throw RequestBadRequest();

    std::size_t found1 = line.find(" ");
    _request_method = line.substr(0, found1);
    if (found1 == std::string::npos)
        throw RequestBadRequest();
    for (; !line.compare(found1, 1, " "); found1++);
    std::size_t found2 = line.find(" ", found1);
    if (found2 == std::string::npos)
        throw RequestBadRequest();
    _request_uri = line.substr(found1, found2 - found1);
    for (; !line.compare(found2, 1, " "); found2++);
    if (found2 == line.length())
        throw RequestBadRequest();
        
    std::size_t found3 = line.find(" ", found2);
    _request_http_version = line.substr(found2, found3 - found2);
    if (_request_http_version[0] != 'H')
        throw RequestBadRequest();
    if (_request_http_version.compare(0, 8, "HTTP/1.1") != 0)
        throw RequestBadRequest();
    std::size_t    it = found2 + 8;
    for (; !line.compare(it, 1, " "); it++);
    if (it != line.length())
        throw RequestBadRequest();
}

void    Request_parser::parse_request_uri()
{
    std::size_t found1 = _request_uri.find("?");
    if (found1 != std::string::npos)
        _request_query_string = _request_uri.substr(found1 + 1, std::string::npos);
    _request_script_name = _request_uri.substr(0, found1);
    _request_query_string_start = found1;
}


int Request_parser::check_reserved_meta_variable(std::string var, std::string value)
{
    for (int i = 0;  i < 22; i++)
    {
        if (var == "CONTENT_TYPE")
        {
            _request_content_type = value;
            return 0;
        }
        if (var == "CONTENT_LENGTH")
        {
            _request_content_length = value;
            return 0;
        }
        if (var.find(_reserved_meta_variables[i]) != std::string::npos)
            return 1;     
    }
    return 0;
}

void    Request_parser::check_fatal_duplicate_meta_variable(std::string var, std::string value)
{
    if (var == "HOST" && value == "HOST")
        throw RequestBadRequest();
    if (var == "CONTENT_LENGTH" && value == "CONTENT_LENGTH")
        throw RequestBadRequest();
}

void    Request_parser::check_duplicate_meta_variable(std::string var)
{
    for (std::vector<std::pair<std::string, std::string> >::iterator    it = _request_headers.begin(); it != _request_headers.end(); it++)
    {
        check_fatal_duplicate_meta_variable(var, it->first);
        if (it->first == var)
        {
            _request_headers.erase(it);
            return;
        }
    }
}

int     Request_parser::check_request_header_field_name(std::string line, std::size_t found)
{
    for (std::size_t    i = 0; i < found; i++)
    {
        if (!(std::isalnum(line[i]) || line[i] == '-' ))
            return 1;   
    }
    return 0;
}

void    Request_parser::parse_request_header(std::string line)
{
    if (line.empty())
        throw RequestBadRequest();
    std::size_t found1 = line.find(":");
    if (found1 == std::string::npos)
        found1 = line.length();

    std::string field_name = line.substr(0, found1);
    std::string field_value;
    if (found1 != line.length())
    {
        found1++;
        if (found1 != std::string::npos)
        {
            for (;  !line.compare(found1, 1, " "); found1++);
            field_value = line.substr(found1, line.length() - found1);
            
        }
        for (std::string::iterator  it = field_name.begin(); it != field_name.end(); it++)
        {
            if (*it >= 'a' && *it <= 'z')
                *it -= 32;
            if (*it == '-')
                *it = '_';
        }
        if (check_reserved_meta_variable(field_name, field_value))
            return ;
        check_duplicate_meta_variable(field_name);
        if (field_name == "HOST")
        {
            _host_count++;
            _request_hostname = field_value;
        }
            
        if (_host_count > 1)
            throw RequestBadRequest();;
    }
    _request_headers.push_back(std::pair<std::string, std::string>(field_name, field_value));
}

void    Request_parser::parse_request()
{
    std::size_t found1 = _request_header.find("\r\n");
    if (found1 == std::string::npos)
        throw RequestBadRequest();
    parse_request_line(_request_header.substr(0, found1));
    std::size_t found2 = 0;
    while (found2 != std::string::npos)
    {
        found1 += 2;
        found2 = _request_header.find("\r\n", found1);

        if (found1 == found2)
            break;
        parse_request_header(_request_header.substr(found1, found2 - found1));
        found1 = found2;  
    }
    if (_host_count != 1)
        throw RequestBadRequest();
    parse_request_uri();
}

void    Request_parser::parse_request_cgi(std::string script_suff)
{
    std::size_t found2 = 0;
    const char* script_suf = &script_suff[0];

    while (found2 != std::string::npos)
    {
        found2 = _request_uri.find(script_suf, found2);
        if (found2 == std::string::npos)
        {
            _request_script_filename = _root_directory + _request_script_name;
            return ;
        }
        found2 += std::strlen(script_suf);
        if (found2 == _request_uri.length() || !_request_uri.compare(found2, 1, "/") || !_request_uri.compare(found2, 1, "?"))
        {
            _request_script_name = _request_uri.substr(0, found2);
            _request_path_info = _request_uri.substr(found2, _request_query_string_start - found2);
            _request_script_filename = _root_directory + _request_script_name;
            return ;
        }     
    }
}

void    Request_parser::swap(Request_parser& a, Request_parser& b)
{
    const char **save_reserved_meta_variables = a._reserved_meta_variables;
    std::string save_root_directoy = a._root_directory;
    std::string save_request_uri = a._request_uri;
    std::string save_index = a._index;
    int         save_is_cookie_present = a._is_cookie_present;
    const char  *save_request_body_char = a._request_body_char;
    std::vector<std::pair<std::string, std::string> >   save_request_headers = a._request_headers;
    std::vector<std::pair<std::string, std::string> >*  save_request_headers_ptr = a._request_headers_ptr;
    std::string save_title = a._title;
    std::size_t save_bytes_inside_body = a._bytes_inside_body;
    const char  *save_body_buffer = a._body_buffer;
    const char  **save_body_buffer_ptr = a._body_buffer_ptr;
    bool    save_is_short_body = a._is_short_body;
    bool    save_is_long_body = a._is_long_body;
    std::size_t save_bytes_inside_file_body = a._bytes_inside_file_body;
    FILE    *save_file_long_body = a._file_long_body;
    std::string save_request_script_name = a._request_script_name;
    std::string save_request_hostname = a._request_hostname;
    
    a._reserved_meta_variables = b._reserved_meta_variables;
    b._reserved_meta_variables = save_reserved_meta_variables;
    a._root_directory = b._root_directory;
    b._root_directory = save_root_directoy;
    a._request_uri = b._request_uri;
    b._request_uri = save_request_uri;
    a._index = b._index;
    b._index = save_index;
    a._request_body_char = b._request_body_char;
    b._request_body_char = save_request_body_char;
    a._request_headers = b._request_headers;
    b._request_headers = save_request_headers;
    a._request_headers_ptr = b._request_headers_ptr;
    b._request_headers_ptr = save_request_headers_ptr;
    a._title = b._title;
    b._title = save_title;
    a._bytes_inside_body = b._bytes_inside_body;
    b._bytes_inside_body = save_bytes_inside_body;
    a._body_buffer = b._body_buffer;
    b._body_buffer = save_body_buffer;
    a._body_buffer_ptr = b._body_buffer_ptr;
    b._body_buffer_ptr = save_body_buffer_ptr;
    a._is_short_body = b._is_short_body;
    b._is_short_body = save_is_short_body;
    a._is_long_body = b._is_long_body;
    b._is_long_body = save_is_long_body;
    a._bytes_inside_file_body = b._bytes_inside_file_body;
    b._bytes_inside_file_body = save_bytes_inside_file_body;
    a._request_hostname = b._request_hostname;
    b._request_hostname = save_request_hostname;
    a._is_cookie_present = b._is_cookie_present;
    b._is_cookie_present = save_is_cookie_present;
    a._file_long_body = b._file_long_body;
    b._file_long_body = save_file_long_body;
    a._request_script_name = b._request_script_name;
    b._request_script_name = save_request_script_name;
}

std::string Request_parser::getRequestMethod() const
{
    return _request_method;
}

void Request_parser::setRequestMethod(std::string method)
{
    _request_method = method;
}

std::string Request_parser::getRequestHostname() const
{
    return _request_hostname;
}

std::string Request_parser::getRequestUri() const
{
    return _request_uri;
}

std::string Request_parser::getRequestScriptName() const
{
    return _request_script_name;
}

std::string Request_parser::getRequestScriptFilename() const
{
    return _request_script_filename;
}

std::string Request_parser::getRequestQueryString() const
{
    return _request_query_string;
}

std::string Request_parser::getRequestPathInfo() const
{
    return _request_path_info;
}

std::vector<std::pair<std::string, std::string> >*   Request_parser::getRequestHeadersPtr() const
{
    return _request_headers_ptr;
}

std::string Request_parser::getRequestContentType() const
{
    return _request_content_type;
}

std::string Request_parser::getRequestContentLength() const
{
    return _request_content_length;
}


const char* Request_parser::getBodyBuffer() const
{
    return _body_buffer;
}

const char**    Request_parser::getBodyBufferPtr() const
{
    return _body_buffer_ptr;
}

const char* Request_parser::getRequestBodyChar() const
{
    return _request_body_char;
}

const char* Request_parser::getRequestHeaderBuffer() const
{
    return _header_buffer;
}

const char** Request_parser::getRequestPtr() const
{
    return ptr;
}

std::string Request_parser::getRootDirectory() const
{
    return _root_directory;
}

void    Request_parser::setRootDirectory(std::string root_directory)
{
    _root_directory = root_directory;
}

std::string Request_parser::getIndex() const
{
    return _index;
}

void    Request_parser::setIndex(std::string index)
{
    _index = index;
}

std::vector<std::pair<std::string, std::string> >   Request_parser::getRequestHeaders()
{
    return _request_headers;
}

std::string Request_parser::getTitle() const
{
    return _title;
}

void    Request_parser::setTitle(std::string title)
{
    _title = title;
}

std::size_t Request_parser::getBytesInsideBody() const
{
    return _bytes_inside_body;
}

bool    Request_parser::getIsShortBody() const
{
    return _is_short_body;
}

bool    Request_parser::getIsLongBody() const
{
    return _is_long_body;
}

int Request_parser::getFdLongBody() const
{
    return _fd_long_body;
}

FILE    *Request_parser::getFileLongBody() const
{
    return _file_long_body;
}

std::size_t Request_parser::getBytesInsideFileBody() const
{
    return _bytes_inside_file_body;
}

void    Request_parser::set_extension(std::string exten)
{
    _extension = exten;
}

std::string Request_parser::get_extension() const
{
    return _extension;
}

int     Request_parser::getCookieIsPresent() const
{
    return _is_cookie_present;
}
