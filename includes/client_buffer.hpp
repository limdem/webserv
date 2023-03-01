#ifndef CLIENT_BUFFER_HPP
#define CLIENT_BUFFER_HPP

# include "webserv.hpp"

class client_buffer 
{
    public :

    client_buffer() : bytes_inside_rest_header(0), respond(0), buffer(NULL), is_short_header(false), bytes_inside_short_header(0), short_header(NULL), is_long_header(0), is_short_body(false), bytes_inside_body(0), short_body(NULL), is_long_body(false), bytes_inside_fd_body(0), size(0), state_suspended(false)
    {
        currently[0] = true;
        for (int i = 1; i < 5; i++)
            currently[i] = false;

        buffer = new char[1024 + 1]();
        short_header = new char[1024 + 1]();

        bytes_inside_rest_header = 0;

        for (int i = 0; i < 1024; i++)
            rest_header[i] = 0;

        is_short_body = false;
        is_short_header = false;
        is_long_header = -1;
        is_long_body = false; 
        total = 0;
        copy = true;
        utils = 0;
        is_chunk = false;

        end_connection = false;

        memset(&y2k1, 0, sizeof(y2k1));

        y2k1.tm_hour = 0;   y2k1.tm_min = 0; y2k1.tm_sec = 0;
        y2k1.tm_year = 100; y2k1.tm_mon = 0; y2k1.tm_mday = 1;

        time(&timer);

        secondsy2k1 =  difftime(timer,mktime(&y2k1));

        last_EPOLL = 0;
        last_EPOLL2 = 0;

        body_size = 0;

        for (int i = 0; i < 4; i++)
            bytes_inside_long_header[i] = 0;

        request_is_invalid = false;
        
       
        
    }

    ~client_buffer()
    {
        //std::cout << "client_buffer destructor" << std::endl;
    }

    char            rest_header[1024];
    int             bytes_inside_rest_header;
    int             respond;

    bool            currently[5];
    char            *buffer;

    bool            is_short_header;
    int             bytes_inside_short_header;   
    char            *short_header;

    int             is_long_header;
    int             bytes_inside_long_header[4];
    char            **long_header;

    bool            is_short_body;
    int             bytes_inside_body;
    char            *short_body;

    bool            is_long_body;
    long            bytes_inside_fd_body;
    FILE            *_file_long_body;
    int             size;

    int             chunktowrite;
    int             bytes_writed;
    bool            is_chunk;
    int             bytes_inside_chunk_body;
    bool            am_i_inside_chunk;
    int             total;

    std::fstream    file;
    FILE    *filetest;
    std::string     tester;
    int             body_size;
    bool            state_suspended;
    bool            end_connection;
    bool            copy;
    int             utils;
    bool            request_is_invalid;

    time_t timer;
    struct tm y2k1;
    double secondsy2k1;
    int last_EPOLL;
    int last_EPOLL2;
};

#endif