#include "../includes/webserv.hpp"

Cgi_handler::Cgi_handler(Request_parser req_parser, Static_response& rsp_hdl)
    :_req_parser(req_parser), _rsp_hdl(&rsp_hdl), _rsp_buf_ptr(&_rsp_buf)
{}

Cgi_handler::~Cgi_handler()
{
    if (_tmpf1)
        std::fclose(_tmpf1);

}

void    Cgi_handler::execute_request()
{
    if (_req_parser.getRequestMethod() == "GET")
        execute_get_request();
    else if (_req_parser.getRequestMethod() == "POST")
        execute_post_request();
    else
        throw CgiScriptInvalidMethod();
}

void    Cgi_handler::create_meta_variables(std::string var, std::string value, int *index)
{
    var += "=";
    var += value;
    _meta_env[*index] = new char[var.length() + 1];
    std::size_t    i = 0;
    for (; i != var.length(); i++)
    {
        _meta_env[*index][i] = var[i];
    }
    _meta_env[*index][i] = 0;  
    (*index)++;
}

void    Cgi_handler::add_header_meta_variables(int *index)
{
    for (std::vector<std::pair<std::string, std::string> >::const_iterator  it = _req_parser.getRequestHeadersPtr()->begin(); it != _req_parser.getRequestHeadersPtr()->end(); it++)
    {
        if (!it->first.empty())
            create_meta_variables("HTTP_" + it->first, it->second, index);
    }
}

void    Cgi_handler::add_request_line_meta_variables(int *index)
{
    create_meta_variables("REQUEST_METHOD", _req_parser.getRequestMethod(), index);
    create_meta_variables("QUERY_STRING", _req_parser.getRequestQueryString(), index);
    create_meta_variables("PATH_INFO", _req_parser.getRequestPathInfo(), index);
    create_meta_variables("SCRIPT_NAME", _req_parser.getRequestScriptName(), index);
    create_meta_variables("SCRIPT_FILENAME", _req_parser.getRequestScriptFilename(), index);
}

void    Cgi_handler::add_server_meta_variables(int *index)
{
    create_meta_variables("REDIRECT_STATUS", "200", index);
    create_meta_variables("CONTENT_TYPE", _req_parser.getRequestContentType(), index);
    create_meta_variables("CONTENT_LENGTH", _req_parser.getRequestContentLength(), index);
    create_meta_variables("GATEWAY_INTERFACE", "CGI/1.1", index);
    create_meta_variables("REMOTE_ADDR", "", index);
    create_meta_variables("REMOTE_HOST", "", index);
    create_meta_variables("SERVER_NAME", "", index);
    create_meta_variables("SERVER_PORT", "", index);
    create_meta_variables("SERVER_PROTOCOL", "HTTP/1.1", index);
    create_meta_variables("SERVER_PROTOCOL", "HTTP/1.1", index);
    create_meta_variables("SERVER_SOFTWARE", "webserv", index);
}

void    Cgi_handler::execute_get_request()
{
    int status;
    struct stat sb;

    std::string script_filename(_req_parser.getRequestScriptFilename());
    std::string ttmp = _rsp_hdl->get_binary_file();
    const char* _cgi_launcher = &ttmp[0];
    _tmpf1 = std::tmpfile();
    if (!_tmpf1)
    {
        std::fclose(_tmpf1);
        _tmpf1 = NULL;
        throw std::exception();
    }
    if (stat(&script_filename[0], &sb) != 0)
    {
        std::fclose(_tmpf1);
        _tmpf1 = NULL;
        throw CgiScriptNotFound();
    }
    if (stat(&script_filename[0], &sb) == 0 && sb.st_mode & S_IFDIR )
    {
        std::fclose(_tmpf1);
        _tmpf1 = NULL;
        throw CgiScriptIsDirectory();
    }

    int pid = fork();
    if (!pid)
    {
        int i = 0;
        try
        {
            _meta_env = new char*[16 + _req_parser.getRequestHeadersPtr()->size() + 1];
        }
        catch(const std::exception& e)
        {
            std::fclose(_tmpf1);
            _tmpf1 = NULL;
            throw ;
        }
        add_header_meta_variables(&i);
        add_request_line_meta_variables(&i);
        add_server_meta_variables(&i);
        _meta_env[i] = NULL;
        char *args[] = {(char*)_cgi_launcher, &script_filename[0], NULL};
        dup2(fileno(_tmpf1), STDOUT_FILENO);
        std::fclose(_tmpf1);
        _tmpf1 = NULL;
        close(STDIN_FILENO);
        execve(args[0], args, _meta_env);
        exit(1);
    }
    waitpid(pid, &status, 0);
    if (WEXITSTATUS(status))
    {
        close(fileno(_tmpf1));
        throw CgiScriptFailed();
    }
    std::rewind(_tmpf1);
    receiveCgiResponse(fileno(_tmpf1));
    std::fclose(_tmpf1);
    _tmpf1 = NULL;
}

void    Cgi_handler::write_body_in_tmpfile()
{
    std::size_t offset = 0;
    std::size_t bytes_count = _req_parser.getBytesInsideBody();
    int fd_pipe[2];
    std::size_t pipe_bytes = 0;
    std::size_t ret;
    char    buf[4096];
        
    if (pipe(fd_pipe) == -1)
        return ;
    std::size_t pipe_bytes_max = fcntl(fd_pipe[1], F_GETPIPE_SZ);
    dup2(fd_pipe[0], STDIN_FILENO);
    close(fd_pipe[0]);

    if (_req_parser.getIsShortBody())
    {
        while (bytes_count && offset < 16 * 1024)
        {
            if (bytes_count > 4096)
            {
                write(fd_pipe[1], _req_parser.getBodyBuffer(), 4096);
                bytes_count -= 4096;
            }
            else
            {
                write(fd_pipe[1], _req_parser.getBodyBuffer(), bytes_count);
                bytes_count = 0;
            }        
        }  
    }
    if (_req_parser.getIsLongBody())
    {
        int tmpf_fd = _req_parser.getFdLongBody();
        while (bytes_count)
        {
            if (pipe_bytes > pipe_bytes_max)
            {
                close(fd_pipe[1]);
                if (pipe(fd_pipe) == -1)
                    return ;
                dup2(fd_pipe[0], STDIN_FILENO);
                close(fd_pipe[0]);
                pipe_bytes = 0;
            }
            ret = read(tmpf_fd, buf, 4096);
            write(fd_pipe[1], buf, ret);
        }
        close(fd_pipe[1]);
    }
    else
        close(fd_pipe[1]);
}

void    Cgi_handler::execute_post_request()
{
    int status;
    struct stat sb;
    
    std::string script_filename(_req_parser.getRequestScriptFilename());
    std::string ttmp = _rsp_hdl->get_binary_file();
    const char* _cgi_launcher = &ttmp[0];
    
    _tmpf1 = std::tmpfile();
    if (!_tmpf1)
    {
        std::fclose(_tmpf1);
        _tmpf1 = NULL;
        throw std::exception();
    }
    if (stat(&script_filename[0], &sb) != 0)
    {
        std::fclose(_tmpf1);
        _tmpf1 = NULL;
        throw CgiScriptNotFound();
    }
    if (stat(&script_filename[0], &sb) == 0 && sb.st_mode & S_IFDIR)
    {
        std::fclose(_tmpf1);
        _tmpf1 = NULL;
        throw CgiScriptIsDirectory();
    }
    int pid = fork();
    if (!pid)
    {
        _tmpf2 = std::tmpfile();
        if (!_tmpf2)
        {
            std::fclose(_tmpf1);
            _tmpf1 = NULL;
            exit(1);
        }
        int i = 0;
        try
        {
            _meta_env = new char*[16 + _req_parser.getRequestHeadersPtr()->size() + 1];
        }
        catch(const std::exception& e)
        {
            std::fclose(_tmpf1);
            _tmpf1 = NULL;
            std::fclose(_tmpf2);
            _tmpf2 = NULL;
            exit(1);
        }
        add_header_meta_variables(&i);
        add_request_line_meta_variables(&i);
        add_server_meta_variables(&i);
        _meta_env[i] = NULL;
        write_body_in_tmpfile();
        char *args[] = {(char *)_cgi_launcher, &script_filename[0], NULL};
        dup2(fileno(_tmpf1), STDOUT_FILENO);
        std::fclose(_tmpf1);
        execve(args[0], args, _meta_env);
        delete [] _meta_env;
        exit(1);
    }
    waitpid(pid, &status, 0);
    if (WEXITSTATUS(status))
    {
        std::fclose(_tmpf1);
        throw CgiScriptFailed();
    }
    std::rewind(_tmpf1);
    receiveCgiResponse(fileno(_tmpf1));
    std::fclose(_tmpf1);
    _tmpf1 = NULL;
}

std::size_t Cgi_handler::find_header_end(std::size_t count_bytes)
{
    std::size_t header_end = 0;
    int i = 0;
    std::size_t j;
    std::size_t ret = 1;
    char    buf[4096];

    for (; header_end < count_bytes && i < 8; header_end++)
    {
        j = 0;
        for (; j < (*_rsp_buf.getBuffersSetPtr())[i].second; j++)
        {
            if (j + 4 <= (*_rsp_buf.getBuffersSetPtr())[i].second && !std::memcmp((*_rsp_buf.getBuffersSetPtr())[i].first + j, "\r\n\r\n", 4))
            {
                _rsp_buf_ptr->setHeaderEndType(2);
                header_end += j;
                return header_end + 2;
            }
            if (j + 2 <= (*_rsp_buf.getBuffersSetPtr())[i].second && !std::memcmp((*_rsp_buf.getBuffersSetPtr())[i].first + j, "\n\n", 2))
            {
                _rsp_buf_ptr->setHeaderEndType(1);
                header_end += j;
                return header_end + 1;
            }
        }
        i++;
        header_end += j;
       
    }
    if (_rsp_buf_ptr->getIndex() == 8)
    {
        while (ret)
        {
            ret = read(fileno(_rsp_buf_ptr->getTmpFile()), buf, 4096);
            for (std::size_t    i = 0; i < ret; i++)    
            {
                if (!std::memcmp((*_rsp_buf.getBuffersSetPtr())[i].first + j, "\r\n\r\n", 4))
                {
                    header_end += j;
                    return header_end + 2;
                }
                if (!std::memcmp((*_rsp_buf.getBuffersSetPtr())[i].first + j, "\n\n", 2))
                {
                    header_end += j;
                    return header_end + 1;
                }
            }
        }
    }
    throw CgiScriptInvalid();
    return 0;
}

void    Cgi_handler::receiveCgiResponse(int fd)
{
    int ret;
    std::size_t count_bytes = 0;
    std::size_t i = 0;
    FILE    *tmpf = NULL;
    struct stat sb;
    char    buf[4096];

    if (fstat(fd, &sb) == -1)
    {
        close(fd);
        throw std::exception();
    }
    if (!sb.st_size)
    {
        close(fd);
        throw CgiScriptFailed();
    }
    while (i < 8)
    {
        try
        {
            
            _rsp_buf.add_buffer();
        }
        catch(const std::exception& e)
        {
            close(fd);
            throw std::exception();
        }
        ret = read(fd, (*_rsp_buf.getBuffersSetPtr())[i].first, 4096);
        if (!ret)
        {
            _rsp_buf.getBuffersSetPtr()->pop_back();
            break ;
        }  
        count_bytes += ret;
        (*_rsp_buf.getBuffersSetPtr())[i].second = ret;
        if (ret == 4096)
            i++;
    }

    if (sb.st_size > 4096 * 8)
    {
        tmpf = std::tmpfile();
        if (!tmpf)
        {
            close(fd);
            throw std::exception();
        }
            
        while (ret)
        {
            ret = read(fd, buf, 4096);
            if (ret == -1)
            {
                close(fd);
                close(fileno(tmpf));
                throw std::exception();
            }
            count_bytes += ret;
            write(fileno(tmpf), buf, ret);
        }
        std::rewind(tmpf);
        i++; 
    }
    _rsp_buf.setIndex(i);
    _rsp_buf_ptr->setCountBytes(count_bytes);
    _rsp_buf_ptr->setTmpFile(tmpf);
    _rsp_buf_ptr->setHeaderEnd(find_header_end(count_bytes));
}

std::size_t Cgi_handler::getLength() const
{
    return _length;
}

Static_response *Cgi_handler::getResponseHandler() const
{
    return _rsp_hdl;
}

std::vector<std::string>    Cgi_handler::getHeaders() const
{
    return _headers;
}

Response_buffers    *Cgi_handler::getResponseBuffersPtr() const
{
    return _rsp_buf_ptr;
}