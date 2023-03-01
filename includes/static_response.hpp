#ifndef STATIC_RESPONSE_HPP
# define STATIC_RESPONSE_HPP
// RESPONSE_HANDLER
#include "webserv.hpp"

class   Static_response
{
private:
    
    Request_parser  _req_pars;

    std::map<int, Response_buffers*> _rsp_buf_tree;
    std::map<int, Response_buffers*> *_rsp_buf_tree_ptr;
    bool    _allowed_method;
    std::string _http_version;
    std::string _status_code;
    std::string _reason_phrase;
    int _fd;
    std::map<int, std::string>  _errorPages;
    std::string                 _server_root;
    int _unique_file_fd;
    Response_buffers*   _tmp_rsp_buffers;
    std::streamsize _length;
    std::string _binary_file;
    std::size_t _client_max_body_size;
    std::string _boundary;
    std::size_t _header_end;
    std::string _upload_store;
    std::string _status;
    std::string _autoIndex;
    std::ifstream   _static_file;
    char    *_tmp_header_field;
    std::vector<char*>  _header_field_vect;
    bool    _is_bad_request;

    std::vector<std::pair<std::string, std::string> >   _response_headers;

    std::ptrdiff_t    check_multipart_header();
    int retrieve_boundary(std::string);
    void    find_part();
    std::size_t    find_boundary_start(std::size_t, int);
    std::size_t    find_boundary_end(std::size_t);
    int check_final_boundary(std::size_t);
    std::size_t find_end_header_field(Response_buffers*, std::size_t, std::size_t, std::size_t);
    char*    generate_header_field(Response_buffers*, std::size_t*, std::size_t*, std::size_t);
    std::pair<std::size_t, std::size_t>    fill_buf_header(char *, char *, std::size_t*, std::pair<std::size_t, std::size_t>);
    std::pair<std::size_t, std::size_t>    fill_buf_body(char *, char *, std::size_t*, std::pair<std::size_t, std::size_t>);
    void    fill_buf_body_static(std::ifstream*, std::size_t, std::size_t);
    void    fill_tmpfile(std::pair<Response_buffers*, Response_buffers*>, std::size_t, std::size_t, std::size_t);
    void    add_cgi_header(std::size_t, std::size_t, Response_buffers*, Response_buffers*);
    void    check_fix_cgi_header(Response_buffers*, Response_buffers*);
    std::size_t    add_cgi_length(Response_buffers *, Response_buffers *);
    void    add_cgi_body(std::size_t, std::size_t, Response_buffers*, Response_buffers*);
    int check_filename_validity(std::string filename);
    std::string    clean_upload_filename(std::string);
    int generate_unique_file(std::string);
public:

    Static_response();
    Static_response(Request_parser, int);
    ~Static_response();


    void    add_client(int);
    void    delete_client(int fd);
    std::string getStatusCode();
    void    setStatusCode(const char*);

    std::string getReasonPhrase();
    void    setReasonPhrase(const char*);

    void    create_response_header();

    void    retrieve_static_resource();
    void    delete_resource();
    void    execute_uploading(std::vector<MultipartPart>, const char*);
    void    open_file_and_write(std::string, const char*, std::size_t);
    void    upload_file();

    void    prepare_cgi_response(Response_buffers*);

    void    setRequestParser(Request_parser);
    void    send_response();
    void    generic_server_response(int);
    void    default_server_response(int);
    void    listing_response(std::string);
    void    custom_response(int);

    int getFd();
    void    setFd(int);

    void    set_status(std::string);
    void    set_autoIndex(std::string autoindex);

    Response_buffers    *getTmpRspBuffers() const;

    std::streamsize getLength() const;

    Request_parser  getReqpars() const;

    std::string getBoundary() const;
    void    setAllowedMethod(bool);
    bool    getAllowedMethods() const;
    std::map<int, Response_buffers*> getBufTree() const;
    std::map<int, Response_buffers*> *getBufTreePtr() const;

    void    print_response_header(std::vector<std::pair<char*, std::size_t> > *, FILE*,  std::size_t);
    void    set_binary_file(std::string);
    std::string     get_binary_file() const;
    void    set_upload_store(std::string);
    std::string get_upload_store() const;
    void    set_error_pages(std::map<int,std::string>);
    void    set_server_root(std::string);
    void    set_client_max_body_size(std::size_t);
    std::size_t get_client_body_size() const;
    class   UploadFileFailed: public std::exception
    {
    public:
        const char *what() const throw ()
        {
            return "UploadFileFailed Exception";
        }
    };
    class   UploadFileBadRequest: public std::exception
    {
    public:
        const char *what() const throw ()
        {
            return "UploadFileBadRequest Exception";
        }
    };
};

#endif