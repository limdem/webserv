#ifndef REQUEST_PARSER_HPP
# define REQUEST_PARSER_HPP

#include "webserv.hpp"


class Request_parser
{
private:
    int _fd;
    std::vector<std::pair<std::string, std::string> >*  _request_headers_ptr;
    std::vector<std::pair<std::string, std::string> >   _request_headers;
    bool    _is_short_body;
    bool    _is_long_body;
    std::size_t _bytes_inside_body;
    FILE    *_file_long_body;
    std::size_t _bytes_inside_file_body;
    const char  *_request_body_char;
    int _host_count;
    std::string _request_content_length;
    std::string _request_content_type;
    std::string _request_http_version;
    std::string _request_query_string;
    std::string _request_path_info;
    std::string _request_script_name;
    std::string _request_uri;
    std::string _request_method;
    const char    **_reserved_meta_variables;
    std::string _request_header;
    const char  *_header_buffer;

    const char  *_body_buffer;
    const char  **_body_buffer_ptr;
    
    
    const char **ptr;
    

    
    
    int     _fd_long_body;

    std::string _root_directory;
    std::string _index;
    std::string _title;
    std::string _request_hostname;
    
    
    
    std::string _request_script_filename;
    
    
    
    
    
    int         _is_cookie_present;
    std::string _extension;
    
    
    
    std::size_t _request_query_string_start;
    
    

    int     check_request_header_field_name(std::string, std::size_t);
    int     check_reserved_meta_variable(std::string, std::string);
    void    check_fatal_duplicate_meta_variable(std::string, std::string);
    void    check_duplicate_meta_variable(std::string);
    void    parse_request_line(std::string);
    void    parse_request_uri();
    void    parse_request_header(std::string);
    void    parse_request_body(std::string);
    void    complete_meta_variables();
public:
    //Request_parser(std::string);
    Request_parser();
    //Request_parser(const char *, int);
    Request_parser(client_buffer*, int);
    Request_parser(const Request_parser& src);
    Request_parser& operator=(const Request_parser& other);

    ~Request_parser();
    void    parse_request();
    void    parse_request_cgi(std::string);
    void    upload_file();
    void    swap(Request_parser&, Request_parser&);
    std::string getRequestMethod() const;
    std::string getRequestHostname() const;
    std::string getRequestUri() const;
    std::string getRequestScriptName() const;
    std::string getRequestScriptFilename() const;
    std::string getRequestQueryString() const;
    std::string getRequestPathInfo() const;
    std::string getRequestContentType() const;
    std::string getRequestContentLength() const;
    std::vector<std::pair<std::string, std::string> >*   getRequestHeadersPtr() const;
    std::string getRequestBody() const;
    const char* getRequestBodyChar() const;
    const char* getRequestHeaderBuffer() const;
    const char** getRequestPtr() const;
    std::string getRootDirectory() const;
    void    setRootDirectory(std::string);
    std::string getIndex() const;
    void    setIndex(std::string);
    std::string getTitle() const;
    void    setTitle(std::string);

    const char* getBodyBuffer() const;
    const char**    getBodyBufferPtr() const;
    std::size_t getBytesInsideBody() const;
    bool    getIsShortBody() const;
    bool    getIsLongBody() const;
    void setRequestMethod(std::string method);

    std::vector<std::pair<std::string, std::string> >   getRequestHeaders();
    int getFdLongBody() const;
    FILE    *getFileLongBody() const;
    std::size_t getBytesInsideFileBody() const;
    void    set_extension(std::string);
    std::string     get_extension() const;
    int     getCookieIsPresent() const;
    class   RequestBadRequest: public std::exception
    {
    public:
        const char *what() const throw ()
        {
            return "RequestBadRequest Exception";
        }
    };
};
#endif
