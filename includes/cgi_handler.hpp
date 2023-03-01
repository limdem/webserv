#ifndef CGI_HANDLER_HPP
# define CGI_HANDLER_HPP

# include "webserv.hpp"

class   Cgi_handler
{
private:
    Request_parser  _req_parser;
    Static_response *_rsp_hdl;
    std::vector<std::string>    _headers;
    Response_buffers    _rsp_buf;
    Response_buffers   *_rsp_buf_ptr;
    std::size_t _length;
    char *_meta_user;
    char    **_meta_env;
    std::vector<std::pair<std::string, std::string> >   _response_headers;
    FILE    *_tmpf1;
    FILE    *_tmpf2;

    void    execute_get_request();
    void    execute_post_request();
    void    execute_delete_request();
    void    create_meta_variables(std::string, std::string, int *);
    void    add_request_line_meta_variables(int *);
    void    add_header_meta_variables(int *);
    void    add_server_meta_variables(int *);
    int is_reserved_meta_variable(std::string);
    void    receiveCgiResponse(int);
    std::size_t find_header_end(std::size_t);
    void    generate_header(std::size_t);
    void    write_body_in_tmpfile();
    void    print_cgi_response_bufs(std::size_t, FILE*);
    
public:
    Cgi_handler(Request_parser, Static_response&);
    ~Cgi_handler();
    void    execute_request();
    std::size_t getLength() const;
    Static_response *getResponseHandler() const;
    std::vector<std::string>    getHeaders() const;
    Response_buffers    *getResponseBuffersPtr() const;
    class   CgiScriptNotFound: public std::exception
    {
    public:
        const char *what() const throw ()
        {
            return "CgiScriptNotFound Exception";
        }
    };
    class   CgiScriptFailed: public std::exception
    {
    public:
        const char *what() const throw ()
        {
            return "CgiScriptFailed Exception";
        }
    };
    class   CgiScriptInvalid: public std::exception
    {
    public:
        const char *what() const throw ()
        {
            return "CgiScriptInvalid Exception";
        }
    };
    class   CgiScriptInvalidMethod: public std::exception
    {
    public:
        const char *what() const throw ()
        {
            return "CgiScriptInvalidMethod Exception";
        }
    };
    class   CgiScriptIsDirectory: public std::exception
    {
    public:
        const char *what() const throw ()
        {
            return "CgiScriptIsDirectory Exception";
        }
    };
};

#endif