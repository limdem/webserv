#ifndef RESPONSE_BUFFERS_HPP
# define RESPONSE_BUFFERS_HPP

//# include <vector>
#include "webserv.hpp"

class Response_buffers
{
private:
    std::vector<std::pair<char*, std::size_t> >  _buffers_set;
    std::vector<std::pair<char*, std::size_t> >  *_buffers_set_ptr;
    std::size_t _index;
    FILE    *_tmpf;

    std::size_t _header_end;
    std::size_t _count_bytes;
    int _header_end_type;
    char    *_tmp_buffer;
    std::vector<char*>  _tmp_buffer_set;
public:
    Response_buffers();
    ~Response_buffers();

    void    add_buffer();
    std::vector<std::pair<char*, std::size_t> >  getBuffersSet() const;
    std::vector<std::pair<char*, std::size_t> >  *getBuffersSetPtr() const;
    std::size_t getIndex() const;
    void    setIndex(std::size_t);
    FILE    *getTmpFile() const;
    void    setTmpFile(FILE*);
    int getHeaderEndType() const;
    void    setHeaderEndType(int);
    std::size_t getHeaderEnd() const;
    void    setHeaderEnd(std::size_t);
    std::size_t getCountBytes() const;
    void    setCountBytes(std::size_t);
};

#endif