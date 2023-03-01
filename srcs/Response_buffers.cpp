#include "../includes/webserv.hpp"

Response_buffers::Response_buffers()
    :_buffers_set_ptr(&_buffers_set), _index(0), _tmp_buffer(NULL)
{}

Response_buffers::~Response_buffers()
{
    if (_tmp_buffer)
        delete [] _tmp_buffer;
    for (std::vector<char*>::const_iterator    it = _tmp_buffer_set.begin(); it != _tmp_buffer_set.end(); it++)
    {
        if (*it != _tmp_buffer)
            delete [] *it;
    }
}

void    Response_buffers::add_buffer()
{

    _tmp_buffer = NULL;
    _tmp_buffer = new char[4096];
    _tmp_buffer_set.push_back(_tmp_buffer);
    _buffers_set.push_back(std::pair<char*, std::size_t>(_tmp_buffer, 0));
    _tmp_buffer = NULL;
    _index++;
}

std::vector<std::pair<char*, std::size_t> >  Response_buffers::getBuffersSet() const
{
    return _buffers_set;
}

std::vector<std::pair<char*, std::size_t> >  *Response_buffers::getBuffersSetPtr() const
{
    return _buffers_set_ptr;
}

std::size_t Response_buffers::getIndex() const
{
    return _index;
}

void    Response_buffers::setIndex(std::size_t index)
{
    _index = index;
}

FILE    *Response_buffers::getTmpFile() const
{
    return _tmpf;
}

void    Response_buffers::setTmpFile(FILE *tmpfile)
{
    _tmpf = tmpfile;
}

int Response_buffers::getHeaderEndType() const
{
    return _header_end_type;
}

void    Response_buffers::setHeaderEndType(int header_end_type)
{
    _header_end_type = header_end_type;
}

std::size_t Response_buffers::getHeaderEnd() const
{
    return _header_end;
}

void    Response_buffers::setHeaderEnd(std::size_t header_end)
{
    _header_end = header_end;
}

std::size_t Response_buffers::getCountBytes() const
{
    return _count_bytes;
}

void    Response_buffers::setCountBytes(std::size_t count_bytes)
{
    _count_bytes = count_bytes;
}