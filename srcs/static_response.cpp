#include "../includes/webserv.hpp"

Static_response::Static_response()
    :_rsp_buf_tree_ptr(&_rsp_buf_tree), _tmp_rsp_buffers(NULL), _tmp_header_field(NULL), _is_bad_request(false)
{}

Static_response::Static_response(Request_parser req_pars, int fd)
    :_req_pars(req_pars), _allowed_method(true), _http_version("HTTP/1.1"), _fd(fd), _is_bad_request(false)
{}

Static_response::~Static_response()
{
    if (_static_file.is_open())
        _static_file.close();
    if (_tmp_header_field)
        delete [] _tmp_header_field;

    for (std::vector<char*>::const_iterator     it = _header_field_vect.begin(); it != _header_field_vect.end(); it++)
    {
        if (*it != _tmp_header_field)
            delete [] *it;
    }
    for (std::map<int, Response_buffers*>::const_iterator   it = _rsp_buf_tree_ptr->begin(); it != _rsp_buf_tree_ptr->end(); it++)
        delete it->second;
}

void    Static_response::add_client(int fd)
{
    _tmp_rsp_buffers = new Response_buffers;
    std::pair<int, Response_buffers*> new_client(fd, _tmp_rsp_buffers);
    _rsp_buf_tree.insert(new_client);

}

void    Static_response::delete_client(int fd)
{
    std::map<int, Response_buffers*>::const_iterator    it = _rsp_buf_tree_ptr->find(fd);
    if (it != _rsp_buf_tree_ptr->end())
    {
        delete it->second;
        _rsp_buf_tree_ptr->erase(it->first);
    }
}

std::string Static_response::getStatusCode()
{
    return _status_code;
}

void    Static_response::setStatusCode(const char* status_code)
{
    _status_code = status_code;
}

std::string Static_response::getReasonPhrase()
{
    return _reason_phrase;
}
void    Static_response::setReasonPhrase(const char* reason_phrase)
{
    _reason_phrase = reason_phrase;
}

void    Static_response::fill_buf_body_static(std::ifstream *file, std::size_t offset, std::size_t bytes_count)
{
    std::size_t i = 0;
    char    buf[4096];
    
    Response_buffers *resp_buf = _rsp_buf_tree_ptr->find(_fd)->second;
    while (bytes_count && i < 8)
    {
        if (bytes_count > 4096 - offset)
        {
            file->read((*resp_buf->getBuffersSetPtr())[i].first + offset, 4096 - offset); 
            bytes_count -= 4096 - offset;

            resp_buf->add_buffer();
            i++;
        }
        else
        {
            file->read((*resp_buf->getBuffersSetPtr())[i].first + offset, bytes_count);
            bytes_count = 0;
        }
    }
    if (bytes_count)
    {
        while (bytes_count)
        {
            if (bytes_count > 4096)
            {
                file->read(buf, 4096);
                write(fileno(resp_buf->getTmpFile()), buf, 4096);
                bytes_count -= 4096;
            }
            else
            {
                file->read(buf, bytes_count);
                write(fileno(resp_buf->getTmpFile()), buf, bytes_count);
                bytes_count = 0;
            }
        }
    }
}

void    Static_response::retrieve_static_resource()
{
    struct stat sb;
    std::string file_path;
    std::string     loc(_server_root);
    std::string     title(_req_pars.getTitle());
    std::string     index;
    int             directory;
    int             dif = 0;
    directory = 0;
    if (_autoIndex != "on")
        index = _req_pars.getIndex();

    loc += title;

    if (_req_pars.getRequestUri() != title)
    {
        dif = 1;
        if (_req_pars.getRequestUri()[title.length()] == '/')
        {
            if (loc[loc.length() - 1] != '/')
            loc += "/";
            loc += _req_pars.getRequestUri().substr(title.length() + 1, std::string::npos);
        }
        else
            loc += _req_pars.getRequestUri().substr(title.length(), std::string::npos);
    }
    if (stat(loc.c_str(), &sb) == 0 && (sb.st_mode & S_IFDIR))
    {
        file_path = _req_pars.getRootDirectory();
        if (file_path[file_path.length() - 1] != '/')
            file_path += "/";
        file_path += index;
        directory = 1;
    } 
    else
    {
        if (dif == 1)
            file_path = loc;
        else
        {
        file_path = _server_root;
        if (file_path[file_path.length() - 1] != '/')
            file_path += "/";
        file_path += title;
        }
    }
    if (directory == 1 && _autoIndex == "on")
    {
        listing_response(file_path);
        return;   
    }

    if (directory == 1 && index == "")
    {
        generic_server_response(500);
        return;   
    }
    _static_file.open(&file_path[0]);
    if (!_static_file.is_open())
    {
        generic_server_response(404);
        return;
    }
    _static_file.ignore(std::numeric_limits<std::streamsize>::max());
    std::size_t length = _static_file.gcount();
    _static_file.clear();
    _static_file.seekg(0, std::ios_base::beg);
    std::string response_line("HTTP/1.1");
    response_line += " ";
    response_line += "200";
    response_line += " ";
    response_line += "OK";
    response_line += "\r\n";

    if (_req_pars.getCookieIsPresent() == 0)
    {
        response_line += "Set-Cookie: cookie-name = cookie-value";
        response_line += "\r\n";
    }

    Response_buffers    *resp_buf = _rsp_buf_tree_ptr->find(_fd)->second;
    std::vector<std::pair<char *, std::size_t> > *buf_set = resp_buf->getBuffersSetPtr();

    std::string content_length("Content-Length: ");
    std::ostringstream ss;
    ss << length;
    content_length += ss.str();
    content_length += "\r\n\r\n";

    resp_buf->add_buffer();
    std::memcpy((*buf_set)[0].first, &response_line[0], response_line.length());
    std::memcpy((*buf_set)[0].first + response_line.length(), &content_length[0], content_length.length());
    
    fill_buf_body_static(&_static_file, response_line.length() + content_length.length(), length);
    resp_buf->setCountBytes(response_line.length() + content_length.length() + length);
    _static_file.close();
}

void    Static_response::delete_resource()
{
    struct stat sb;

    std::string resource_path;
    if (_req_pars.getRootDirectory()[_req_pars.getRootDirectory().length()] == '/')
        resource_path = _req_pars.getRootDirectory().substr(0, _req_pars.getRootDirectory().length() - 1);
    else
        resource_path = _req_pars.getRootDirectory();
    resource_path += _req_pars.getRequestScriptName();
    if (stat(&resource_path[0], &sb) != 0)
    {
        generic_server_response(404);
        return ;
    }
    if (stat(&resource_path[0], &sb) == 0 && sb.st_mode & S_IFDIR)
    {
        if (rmdir(&resource_path[0]) == -1)
        {
            generic_server_response(500);
            return ;
        }
    }
    else
    {
        if (std::remove(&resource_path[0]) != 0)
        {
            generic_server_response(500);
            return ;
        }
    }    
    generic_server_response(200);
}

int Static_response::check_filename_validity(std::string filename)
{
    if (filename == "\"\"")
        return 1;
    return 0;
}

std::string    Static_response::clean_upload_filename(std::string filename)
{
    if (filename.length() > 2 && filename[0] == '\"' && filename[filename.length() - 1] == '\"')
        return filename.substr(1, filename.length() - 2);
    return filename;
}

int Static_response::generate_unique_file(std::string filename)
{
    std::string first_part;
    std::string second_part;
    std::size_t found;

    if (filename == "\"\"")
        return -2;
    found = filename.find('.');
    if (found != std::string::npos)
        second_part = filename.substr(found, std::string::npos);
    first_part = filename.substr(0, found);
    first_part += "XXXXXX";
    first_part += second_part;
    _unique_file_fd = mkstemps(&first_part[0], second_part.length());
    return _unique_file_fd;
}

void    Static_response::open_file_and_write(std::string filename, const char* body, std::size_t len)
{
    int fd;

    filename = clean_upload_filename(filename);
    if (check_filename_validity(filename))
        return ;
    if (_upload_store[_upload_store.length() - 1] != '/')
        _upload_store += "/";
    filename = _upload_store + filename;
    fd = generate_unique_file(filename);
    if (fd == -1)
        throw UploadFileFailed();
    if (fd == -2)
        return ;
    write(fd, body, len);
    close(fd);
}

void    Static_response::execute_uploading(std::vector<MultipartPart> parts, const char* body)
{
    std::vector<std::pair<std::string, std::string> >    headers;
    for (std::vector<MultipartPart>::const_iterator it1 = parts.begin(); it1 != parts.end(); it1++)
    {
        headers = it1->getHeaders();
        for (std::vector<std::pair<std::string, std::string> >::const_iterator  it2 = headers.begin(); it2 != headers.end(); it2++)
        {
            if (it2->first == "filename")
            {
                open_file_and_write(it2->second, body + it1->getBodyStart(), it1->getBodyEnd() - it1->getBodyStart());
                break;
            }      
        }
    }
    return;    
}

int Static_response::retrieve_boundary(std::string value_field)
{
    std::size_t found;

    std::string upper_value_field;
    for (std::string::const_iterator    it = value_field.begin(); it != value_field.end(); it++)
        upper_value_field += std::toupper(*it);
    found = upper_value_field.find("MULTIPART/FORM-DATA;");
    if (found == std::string::npos)
        found = upper_value_field.find("MULTIPART/FORM-DATA ");
    if (found == std::string::npos)
        throw UploadFileBadRequest();
    found += 21;
    found = upper_value_field.find("BOUNDARY", found);
    if (found == std::string::npos)
        throw UploadFileBadRequest();
    found += 9;
    upper_value_field.find("=", found);
    if (found == std::string::npos)
        throw UploadFileBadRequest();
    std::string::const_iterator end_boundary = value_field.end();
    end_boundary -= 2;
    _boundary = value_field.substr(found, value_field.length() - 2);
    return 0;
}

std::ptrdiff_t Static_response::check_multipart_header()
{
    std::vector<std::pair<std::string, std::string> > headers = getReqpars().getRequestHeaders();  
                       
    std::vector<std::pair<std::string, std::string> >::const_iterator   it = headers.begin();

    for (; it != headers.end(); it++)
    {
        if (it->first == "CONTENT_TYPE")
            break;      
    }
    return (it - headers.begin());
}

void    Static_response::find_part()
{

    std::size_t index = 0;
    std::pair<std::size_t, std::size_t>   content_disp_header;

    struct stat sb;
    char    *file_mmap;
    MultipartParser mult_pars;

    if (_req_pars.getIsShortBody())
    {
        mult_pars.setRequestBody(_req_pars.getBodyBuffer());
        mult_pars.setBodyLength(_req_pars.getBytesInsideBody());
    }
    if (_req_pars.getIsLongBody())
    {
        if (fstat(fileno(_req_pars.getFileLongBody()), &sb) == -1)
            throw UploadFileFailed();
        file_mmap = (char*)mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fileno(_req_pars.getFileLongBody()), 0);
        mult_pars.setRequestBody(file_mmap);
        mult_pars.setBodyLength(_req_pars.getBytesInsideFileBody());
    }
    mult_pars.setBoundary("--" + _boundary);
    while (index < mult_pars.getBodyLength())
    {
        index = mult_pars.find_boundary_start(index);
        index = mult_pars.parse_part(index);
    }

    std::vector<std::pair<std::size_t, std::size_t> > header = mult_pars.getHeadersStartEnd();
    std::vector<std::pair<std::size_t, std::size_t> > body = mult_pars.getBodiesStartEnd();
    for (std::size_t    i = 0; i < header.size(); i++)
        mult_pars.parse_parts_header(header[i].first, header[i].second, body[i].first, body[i].second);
    std::vector<MultipartPart>  parts = mult_pars.getParts();
    if (mult_pars.check_upload_validity(parts))
        return;
    execute_uploading(parts, mult_pars.getRequestBody());
}

void    Static_response::upload_file()
{
    std::ptrdiff_t header = check_multipart_header();
    if (header < 0)
        throw UploadFileBadRequest();
    if (static_cast<std::size_t>(header) == _req_pars.getRequestHeaders().size())
        throw UploadFileBadRequest();
    if (retrieve_boundary(_req_pars.getRequestHeaders()[header].second))
        return;
    find_part(); 
}

std::size_t Static_response::find_end_header_field(Response_buffers *rsp_buf, std::size_t offset, std::size_t bytes_count, std::size_t index)
{
    std::size_t header_field_len = 0;

    while (header_field_len < bytes_count && index < 8)
    {
        if (offset > 4096)
        {
            index++;
            offset = 0;
        }
        if (!std::memcmp((*rsp_buf->getBuffersSetPtr())[index].first + offset, "\n", 1))
        {
            if (!std::memcmp((*rsp_buf->getBuffersSetPtr())[index].first + offset - 1, "\r", 1))
                header_field_len--;
            break ;
        }
        header_field_len++; 
        offset++;
    }
    return header_field_len;
}

char*    Static_response::generate_header_field(Response_buffers *rsp_buf, std::size_t *offset, std::size_t *index, std::size_t len)
{
    std::size_t i = 0;
    _tmp_header_field = NULL;
    _tmp_header_field = new char[len + 2];
    _header_field_vect.push_back(_tmp_header_field);
    std::size_t j = *offset;

    std::memcpy(_tmp_header_field + len, "\r\n", 2);
    while (len && *index < 8)
    {
        if (len > 4096 - j)
        {
            std::memcpy(_tmp_header_field + i, (*rsp_buf->getBuffersSetPtr())[*index].first + j, 4096 - j);
            (*index)++;
            i += 4096 - j;
            len -= 4096 - j;
            j = 0;
        }
        else
        {
            std::memcpy(_tmp_header_field + i, (*rsp_buf->getBuffersSetPtr())[*index].first + j, len);
            i += len;
            len = 0;
            j += len;
        }
    }
    *offset = j;
    return _tmp_header_field;
}

void    Static_response::add_cgi_header(std::size_t bytes_count, std::size_t offset_start, Response_buffers *resp_buf, Response_buffers *rsp_buf)
{
    std::pair<std::size_t, std::size_t> offsets(offset_start, 0);
    std::size_t i = resp_buf->getIndex() - 1;
    std::size_t j = 0;
    std::size_t header_field_len;
    std::size_t save_bytes_count = bytes_count;

    while (bytes_count > 0 && i < 8)
    {
        header_field_len = find_end_header_field(rsp_buf, offsets.second, save_bytes_count, j);
        char *test = generate_header_field(rsp_buf, &offsets.second, &j, header_field_len);
        offsets = fill_buf_header((*resp_buf->getBuffersSetPtr())[i].first, test, &header_field_len, offsets);
        _tmp_header_field = NULL;
        if (bytes_count < offsets.second)
            bytes_count = 0;
        else
            bytes_count -= offsets.second;
        if (!offsets.second)
        {
            offsets.second = 0;
            j++;
        }
        if (!offsets.first)
        {
            i++;

            resp_buf->add_buffer();
        }
    }
    if (bytes_count)
    {
        i++;
        fill_tmpfile(std::pair<Response_buffers*, Response_buffers*>(resp_buf, rsp_buf), bytes_count, j, offsets.second);
    }
    resp_buf->setIndex(i);
    resp_buf->setHeaderEnd(offsets.first);
}

void    Static_response::print_response_header(std::vector<std::pair<char*, std::size_t> >  *vect_bufs, FILE *tmpfile,  std::size_t bytes_count)
{
    std::cout << "PRINT_RESPONSE_HEADER" << std::endl;
    char    buf[4096];
    std::size_t ret = 1;

    for (std::size_t    i = 0; bytes_count && i < 8; i++)
    {
        std::cout << "BYTES_COUNT: " << bytes_count << std::endl;
        if (bytes_count < 4096)
        {
            std::cout.write((*vect_bufs)[i].first, bytes_count);
            return ;
        }   
        else
        {
            std::cout.write((*vect_bufs)[i].first, 4096);
            bytes_count -= 4096;
        }

    }
    std::cout << "BYTES_COUNT667: " << bytes_count << std::endl;
    if (bytes_count)
    {
        while (ret)
        {
            ret = read(fileno(tmpfile), buf, 4096);
            std::cout << "RET: " << ret << std::endl;
            write(1, buf, ret);
        } 
    }
    std::cout << "END" << std::endl;
}

void    Static_response::set_binary_file(std::string str)
{
    _binary_file = str;
}

std::string     Static_response::get_binary_file() const
{
    return _binary_file;
}

void    Static_response::set_upload_store(std::string upload_store)
{
    _upload_store = upload_store;
}

std::string Static_response::get_upload_store() const
{
    return _upload_store;
}

void    Static_response::set_error_pages(std::map<int,std::string> errorP)
{
    _errorPages = errorP;
}

void    Static_response::set_server_root(std::string serverRoot)
{
    _server_root = serverRoot;
}

void    Static_response::set_client_max_body_size(std::size_t client_max_body_size)
{
    _client_max_body_size = client_max_body_size;
}

std::size_t Static_response::get_client_body_size() const
{
    return _client_max_body_size;
}

std::pair<std::size_t, std::size_t>    Static_response::fill_buf_header(char *buf_dst, char *buf_src, std::size_t *bytes_count, std::pair<std::size_t, std::size_t> offsets)
{  
    std::size_t save_bytes_count = *bytes_count + 2;
    if (save_bytes_count > 4096)
        save_bytes_count = 4096;

    if (offsets.first > offsets.second)
    {
        if (offsets.first > save_bytes_count && save_bytes_count > 4096 - offsets.first)
            save_bytes_count = 4096;
        if (save_bytes_count - offsets.first < save_bytes_count)
        {
            std::memcpy(buf_dst + offsets.first, buf_src + offsets.second, save_bytes_count - offsets.first);
            *bytes_count -= save_bytes_count - offsets.first;
            offsets.second += save_bytes_count - offsets.first;
            offsets.first += save_bytes_count - offsets.first;
        }
            
        else
        {  
            std::memcpy(buf_dst + offsets.first, buf_src, save_bytes_count);
            offsets.second += *bytes_count + 1;
            *bytes_count -= save_bytes_count;
            offsets.first += save_bytes_count;
        }
    }       
    else
    {
        if (offsets.second > save_bytes_count && save_bytes_count > 4096 - offsets.second)
            save_bytes_count = 4096;
        if (4096 - offsets.second < save_bytes_count)
        {
            std::memcpy(buf_dst + offsets.first, buf_src + offsets.second, 4096 - offsets.second);
            *bytes_count -= 4096 - offsets.second;
            offsets.first += 4096 - offsets.second;
            offsets.second += 4096 - offsets.second;
            
            
        }
            
        else
        {
            std::memcpy(buf_dst + offsets.first, buf_src + offsets.second, save_bytes_count);
            offsets.second += *bytes_count + 1;
            *bytes_count -= save_bytes_count;
            offsets.first += save_bytes_count;
            
        }
    }
    return  offsets;
}

std::pair<std::size_t, std::size_t>    Static_response::fill_buf_body(char *buf_dst, char *buf_src, std::size_t *bytes_count, std::pair<std::size_t, std::size_t> offsets)
{  
    std::size_t save_bytes_count = *bytes_count;
    if (save_bytes_count > 4096)
        save_bytes_count = 4096;
    if (offsets.first > offsets.second)
    {
        if (offsets.first > save_bytes_count && save_bytes_count > 4096 - offsets.first)
            save_bytes_count = 4096;
        if (save_bytes_count - offsets.first < save_bytes_count)
        {
            std::memcpy(buf_dst + offsets.first, buf_src + offsets.second, save_bytes_count - offsets.first);
            *bytes_count -= save_bytes_count - offsets.first;
        }
            
        else
        {  
            std::memcpy(buf_dst + offsets.first, buf_src + offsets.second, save_bytes_count);
            *bytes_count -= save_bytes_count;
        }

        offsets.second += save_bytes_count - offsets.first;
        offsets.first += save_bytes_count - offsets.first;
    }       
    else
    {
        if (offsets.second > save_bytes_count && save_bytes_count > 4096 - offsets.second)
            save_bytes_count = 4096;
        if (save_bytes_count > offsets.second && save_bytes_count - offsets.second < save_bytes_count)
        {
            std::memcpy(buf_dst + offsets.first, buf_src + offsets.second, save_bytes_count - offsets.second);
            *bytes_count -= save_bytes_count - offsets.second;
        }
            
        else
        {
            std::memcpy(buf_dst + offsets.first, buf_src + offsets.second, save_bytes_count);
            *bytes_count -= save_bytes_count;
        }
        offsets.first += save_bytes_count - offsets.second;
        offsets.second += save_bytes_count - offsets.second;
    }
    return  offsets;
}

std::size_t    Static_response::add_cgi_length(Response_buffers *resp_buf, Response_buffers *rsp_buf)
{
    std::stringstream   ss;
    std::string content_length("Content-Length: ");
    std::size_t offset = resp_buf->getHeaderEnd();
    std::size_t bytes_count;
    std::size_t i = resp_buf->getIndex();
    FILE    *tmpf = NULL;

    ss << rsp_buf->getCountBytes() - rsp_buf->getHeaderEnd() - 2;
    content_length += ss.str();
    content_length += "\r\n\r\n";
    bytes_count = content_length.length();
    while (bytes_count && i < 8)
    {
        if (bytes_count < 4096 - offset)
        {
            std::memcpy((*resp_buf->getBuffersSetPtr())[i].first + offset, content_length.c_str(), bytes_count);
            bytes_count = 0;
        }   
        else
        {
            std::memcpy((*resp_buf->getBuffersSetPtr())[i].first + offset, content_length.c_str(), 4096 - offset);
            bytes_count -= 4096 - offset;
            i++;
        }
        
    }
    if (bytes_count)
    {
        i++;
        tmpf = std::tmpfile();
        write(fileno(tmpf), content_length.c_str(), bytes_count);
    }
    resp_buf->setIndex(i);
    resp_buf->setTmpFile(tmpf);
    return content_length.length();
}

void    Static_response::fill_tmpfile(std::pair<Response_buffers*, Response_buffers*> buffs, std::size_t bytes_count, std::size_t index, std::size_t offset)
{
    
    FILE    *tmpf1;
    FILE    *tmpf2;
    char    buf[4096];
    int     ret = 1;

    if (buffs.first->getIndex() == 8)
        buffs.first->setTmpFile(std::tmpfile());
    tmpf1 = buffs.first->getTmpFile();
    for (std::size_t    i = index; i < 8; i++)
    {
        if (bytes_count > 4096 - offset)
        {
            std::memcpy(buf, (*buffs.second->getBuffersSetPtr())[i].first + offset, 4096 - offset);
            write(fileno(tmpf1), buf, 4096 - offset);
            bytes_count -= 4096 - offset;
            offset = 0;
        }
        else
        {
            std::memcpy(buf, (*buffs.second->getBuffersSetPtr())[i].first + offset, bytes_count);
            write(fileno(tmpf1), buf, bytes_count);
            bytes_count = 0;
        }
    }
    if (bytes_count)
    {
        tmpf2 = buffs.second->getTmpFile();
        while (ret)
        {
            ret = read(fileno(tmpf2), buf, 4096);
            write(fileno(tmpf1), buf, ret);
        }
        std::rewind(tmpf1);
    }
}

void    Static_response::add_cgi_body(std::size_t bytes_count, std::size_t offset_start, Response_buffers *resp_buf, Response_buffers *rsp_buf)
{
    std::pair<std::size_t, std::size_t> offsets(offset_start, rsp_buf->getHeaderEnd() + rsp_buf->getHeaderEndType());
    std::size_t i = resp_buf->getIndex();
    std::size_t j = 0;

    while (bytes_count > 0 && i < 8)
    {

        offsets = fill_buf_body((*resp_buf->getBuffersSetPtr())[i].first, (*rsp_buf->getBuffersSetPtr())[j].first, &bytes_count, offsets);
        if (offsets.second == 4096)
        {
            offsets.second = 0;
            j++;
        }
        if (offsets.first == 4096)
        {
            offsets.first = 0;
            i++;
            resp_buf->add_buffer();
           
        }
    }
    if (bytes_count)
        fill_tmpfile(std::pair<Response_buffers*, Response_buffers*>(resp_buf, rsp_buf), bytes_count, j, offsets.second);   
}

void    Static_response::prepare_cgi_response(Response_buffers *rsp_buf)
{
    std::stringstream   ss;
    std::size_t length;
    std::string response_line("HTTP/1.1");

    response_line += " ";
    response_line += "200";
    response_line += " ";
    response_line += "OK";
    response_line += "\r\n";
    
    Response_buffers    *resp_buf = _rsp_buf_tree_ptr->find(_fd)->second;
    std::vector<std::pair<char *, std::size_t> > *buf_set = resp_buf->getBuffersSetPtr();
    resp_buf->add_buffer();
    std::memcpy((*buf_set)[0].first, response_line.c_str(), response_line.length());
    resp_buf->setHeaderEnd(rsp_buf->getHeaderEnd());
    add_cgi_header(resp_buf->getHeaderEnd(), 17, resp_buf, rsp_buf);
    length = add_cgi_length(resp_buf, rsp_buf);
    add_cgi_body(rsp_buf->getCountBytes() - rsp_buf->getHeaderEnd() - 2, resp_buf->getHeaderEnd() + length, resp_buf, rsp_buf);
    resp_buf->setCountBytes(resp_buf->getHeaderEnd() + length + rsp_buf->getCountBytes() - rsp_buf->getHeaderEnd() - 2);
    return ;
}

void    Static_response::setRequestParser(Request_parser req_pars)
{
    _req_pars = req_pars;
}

void    Static_response::send_response()
{
    std::map<int, Response_buffers *>::iterator it = getBufTreePtr()->find(_fd);
    std::size_t i = 0;
    std::size_t bytes_count = it->second->getCountBytes();
    char    buf[4096];
    std::size_t ret;
    

    for (; i <= it->second->getIndex() && i < 8 && bytes_count; i++)
    {
        if (bytes_count > 4096)
        {
            ret = send(_fd, (*it->second->getBuffersSetPtr())[i].first, 4096, 0);
            bytes_count -= 4096;
        } 
        else
        {
            ret = send(_fd, (*it->second->getBuffersSetPtr())[i].first, bytes_count, 0);
            bytes_count = 0;
        }
    }
    if (bytes_count)
    {
        while (ret)
        {
            if (bytes_count > 4096)
            {
                ret = read(fileno(it->second->getTmpFile()), buf, 4096);
                send(_fd, buf, 4096, 0);
                bytes_count -= 4096;
            }
            else
            {
                ret = read(fileno(it->second->getTmpFile()), buf, bytes_count);
                send(_fd, buf, bytes_count, 0);
                bytes_count = 0;
            }
        } 
    }
    it->second->setIndex(0);
    it->second->setCountBytes(0);
    it->second->getBuffersSetPtr()->clear();
}

void    Static_response::custom_response(int code)
{
    std::ifstream   file;
    std::string     file_path(_server_root);

    if (file_path[file_path.length() - 1] != '/')
        file_path += "/";
    file_path += _errorPages[code];
    file.open(file_path.c_str());
    if (!file.is_open())
    {
        default_server_response(501);
        return;
    }
    file.ignore(std::numeric_limits<std::streamsize>::max());
    std::size_t length = file.gcount();
    file.clear();
    file.seekg(0, std::ios_base::beg);
    std::string response_line("HTTP/1.1");
    response_line += " ";
    response_line += _status;
    response_line += " ";
    if (code == 200)
        response_line += "OK";
    if (code == 201)
        response_line += "Created";
    if (code == 400)
        response_line += "Bad Request";
    if (code == 403)
        response_line += "Forbidden";
    if (code == 404)
        response_line += "Not Found";
    if (code == 405)
        response_line += "Method Not Allowed";
    if (code == 413)
        response_line += "Payload Too Large";        
    if (code == 500)
        response_line += "Internal Server Error";
    response_line += "\r\n";

    if (_req_pars.getCookieIsPresent() == 0)
    {
        response_line += "Set-Cookie: cookie-name = cookie-value";
        response_line += "\r\n";
    }

    Response_buffers    *resp_buf = _rsp_buf_tree_ptr->find(_fd)->second;
    std::vector<std::pair<char *, std::size_t> > *buf_set = resp_buf->getBuffersSetPtr();
    std::string content_length("Content-Length: ");
    std::ostringstream ss;
    ss << length;
    content_length += ss.str();
    content_length += "\r\n\r\n";
    resp_buf->add_buffer();
    std::memcpy((*buf_set)[0].first, &response_line[0], response_line.length());
    std::memcpy((*buf_set)[0].first + response_line.length(), &content_length[0], content_length.length());
    
    fill_buf_body_static(&file, response_line.length() + content_length.length(), length);
    resp_buf->setCountBytes(response_line.length() + content_length.length() + length);
    file.close();
}

void    Static_response::generic_server_response(int code)
{
    if (_errorPages.find(code) != _errorPages.end())
        custom_response(code);
    else
        default_server_response(code);
}

void    Static_response::default_server_response(int code)
{
    std::string status_code;
    std::string reason_phrase;

    if (code == 200)
    {
        status_code = "200";
        reason_phrase = "OK";        
    }
    if (code == 201)
    {
        status_code = "201";
        reason_phrase = "Created";        
    }
    if (code == 400)
    {
        status_code = "400";
        reason_phrase = "Bad Request";        
    }
    if (code == 403)
    {
        status_code = "403";
        reason_phrase = "Forbidden";        
    }
    if (code == 404)
    {
        status_code = "404";
        reason_phrase = "Not Found";
    }
    if (code == 405)
    {
        status_code = "405";
        reason_phrase = "Method Not Allowed";        
    }
    if (code == 413)
    {
        status_code = "413";
        reason_phrase = "Payload Too Large";        
    }
    if (code == 500)
    {
        status_code = "500";
        reason_phrase = "Internal Server Error";         
    }
    std::string response_line("HTTP/1.1");
    response_line += " ";
    response_line += status_code;
    response_line += " ";
    response_line += reason_phrase;
    response_line += "\r\n";

    std::string body("<html>\r\n<head><title>");
    body += status_code;
    body += " ";
    body += reason_phrase;
    body += "</title></head>\r\n<body>\r\n<center><h1>";
    body += status_code;
    body += " ";
    body += reason_phrase;
    body += "</h1></center>\r\n<hr><center>webserv/1.0 (Ubuntu)</center>\r\n</body>\r\n</html>\r\n";
    std::string content_length("Content-Length: ");
    std::stringstream   str_convert;
    str_convert << body.length();
    content_length += str_convert.str();
    content_length += "\r\n\r\n";
    Response_buffers *resp_buf = _rsp_buf_tree_ptr->find(_fd)->second;
    resp_buf->add_buffer();
    std::memcpy((*resp_buf->getBuffersSetPtr())[0].first, response_line.c_str(), response_line.length());
    std::memcpy((*resp_buf->getBuffersSetPtr())[0].first + response_line.length(), content_length.c_str(), content_length.length());
    std::memcpy((*resp_buf->getBuffersSetPtr())[0].first + response_line.length() + content_length.length(), body.c_str(), body.length());
    resp_buf->setCountBytes(response_line.length() + content_length.length() + body.length());   
}

void    Static_response::listing_response(std::string path)
{
    std::string response_line("HTTP/1.1");
    response_line += " ";
    response_line += "200";
    response_line += " ";
    response_line += "\r\n";

    std::string body("<html>\r\n<head><title>");
    body += "Index of";
    body += "</title></head>\r\n<body>\r\n<center><h1>";
    body += "Index of";
    body += "</h1></center>\r\n<hr><center>webserv/1.0 (Ubuntu)</center>\r\n</body>\r\n</html>\r\n";
    body += "\r\n";

    if (_req_pars.getIndex().size())
    {
        std::string::size_type pos1 = path.find_last_of('/');
        path = path.substr(0, pos1);
    }

    std::string dirName(path);
    DIR *dir = opendir(path.c_str());

    if (dirName[0] != '/')
        dirName = "/" + dirName;

    std::string uri = _req_pars.getRequestUri();


    std::vector<std::string> h = split2(_req_pars.getRequestHostname(), ':');
    const char* pp = h[1].c_str();
    int dodi = std::atoi(pp);

    for (struct dirent *dirEntry = readdir(dir); dirEntry; dirEntry = readdir(dir)) {
        body += get_Link(std::string(dirEntry->d_name), uri, h[0], dodi);
    }
    closedir(dir);

    std::string content_length("Content-Length: ");
    std::stringstream   str_convert;
    str_convert << body.length();
    content_length += str_convert.str();
    content_length += "\r\n\r\n";
    Response_buffers *resp_buf = _rsp_buf_tree_ptr->find(_fd)->second;
    resp_buf->add_buffer();
    std::memcpy((*resp_buf->getBuffersSetPtr())[0].first, response_line.c_str(), response_line.length());
    std::memcpy((*resp_buf->getBuffersSetPtr())[0].first + response_line.length(), content_length.c_str(), content_length.length());
    std::memcpy((*resp_buf->getBuffersSetPtr())[0].first + response_line.length() + content_length.length(), body.c_str(), body.length());
    resp_buf->setCountBytes(response_line.length() + content_length.length() + body.length());
}

int Static_response::getFd()
{
    return _fd;
}

void    Static_response::setFd(int fd)
{
    _fd = fd;
}

void    Static_response::set_status(std::string status)
{
    _status = status;
}

void    Static_response::set_autoIndex(std::string autoindex)
{
    _autoIndex = autoindex;
}

Response_buffers    *Static_response::getTmpRspBuffers() const
{
    return _tmp_rsp_buffers;
}

std::streamsize Static_response::getLength() const
{
    return _length;
}

Request_parser  Static_response::getReqpars() const
{
    return _req_pars;
}

std::string Static_response::getBoundary() const
{
    return _boundary;
}

void    Static_response::setAllowedMethod(bool allowed_methods)
{
    _allowed_method = allowed_methods;
}

bool    Static_response::getAllowedMethods() const
{
    return _allowed_method;
}

std::map<int, Response_buffers*> Static_response::getBufTree() const
{
    return _rsp_buf_tree;
}

std::map<int, Response_buffers*> *Static_response::getBufTreePtr() const
{
    return _rsp_buf_tree_ptr;
}