#ifndef SOCKET_CLASS_HPP
#define SOCKET_CLASS_HPP

#include <iostream>
#include "client_buffer.hpp"
#include <errno.h>
#include <time.h>
#include <stdexcept>
#include <limits>
#include <stdio.h>

class BadRequest : public std::exception {
    public:
    const char * what () const throw()
    {
        return (char*)"Bad Request";
    }
};



class socket_class 
{
    public :

    socket_class()
    {
        //std::cout << "socket_class constructor" << std::endl;
    }

    ~socket_class()
    {
        //std::cout << "socket_class destructor" << std::endl;
    }

    //////////////////////////////// MAP OPERATION ///////////////////////////////

    void add_client(int socket_client)
    {

        client_buffer *client = new client_buffer;
        if (!client)
            throw std::runtime_error ("new client_buffer fail");
        //std::cout << "Le CLIENT numero " << socket_client << " est ajoute" << std::endl << std::endl;
        if ((tree.insert(std::pair<int, client_buffer*>(socket_client, client))).second == false)
        {
            //std::cout << "CLIENT ALREADY EXIST IN TREE " << std::endl;
            if (!tree.insert(std::pair<int, client_buffer*>(socket_client, client)).first->second->state_suspended)
            {
                //std::cout << "CLIENT IS NOT SUSPENDED SO HE IS SUPPRESSED " << std::endl << std::endl;
                tree.erase(tree.insert(std::pair<int, client_buffer*>(socket_client, client)).first);
                tree.insert(std::pair<int, client_buffer*>(socket_client, client));
            }
        }
        
    }

    int traitement(int file_descriptor)
    {
        std::map<int, client_buffer*>::iterator it = tree.find(file_descriptor);

        time(&it->second->timer);
        it->second->secondsy2k1 = difftime(it->second->timer,mktime(&it->second->y2k1));

        it->second->last_EPOLL2 = 0;
        it->second->last_EPOLL = 0;

        int    (socket_class::*f[5])( std::map<int, client_buffer*>::iterator ) = { &socket_class::get_header, &socket_class::drive_body, &socket_class::get_short_body, &socket_class::get_long_body, &socket_class::get_chunk_body } ;
        
        for (int i = 0; i < 5; i++)
        {
            if (it->second->currently[i] == true)
                if (!(this->*f[i])(it))
                    return 0;
        }

        return 1;
    }

    ////////////////////////////// BUFFER OPERATION ////////////////////////////////////


    int is_there_a_body(char* buffer, int size)
    {
        for ( int i = 0; i < size; i++)
        {
            if (!strncmp( buffer + i, "Content-Length", 14 ))
                return atoi( buffer + i + 15 );             
        }
        return 0;
    }

    bool is_there_a_chunk(char* buffer, int size)
    {
        for ( int i = 0; i < size; i++)
        {
            if (!strncmp( buffer + i, "Transfer-Encoding: chunked", 26 ))
                return true;             
        }
        return false;
    }
    
    int		ft_iswhitespace(char const c)
    {
    	if (c == ' ' || c == '\n' || c == '\t' || c == '\v'
    		|| c == '\r' || c == '\f')
    		return (1);
    	return (0);
    }
    char    to_lower(char c)
    {
        if (c >= 'A' && c <= 'Z')
            return (c + ('a' - 'A'));
        return (c);
    }
    
    int     get_digit(char c, int digits_in_base)
    {
        int max_digit;
        if (digits_in_base <= 10)
            max_digit = digits_in_base + '0';
        else
            max_digit = digits_in_base - 10 + 'a';
    
        if (c >= '0' && c <= '9' && c <= max_digit)
            return (c - '0');
        else if (c >= 'a' && c <= 'f' && c <= max_digit)
            return (10 + c - 'a');
        else
            return (-1);
    }
    
    int     ft_atoi_base(const char *str, int str_base)
    {
        int result = 0;
        int sign = 1;
        int digit;
    
        if (*str == '-')
        {
            sign = -1;
            ++str;
        }
    
        while ((digit = get_digit(to_lower(*str), str_base)) >= 0)
        {
            result = result * str_base;
            result = result + (digit * sign);
            ++str;
        }
        return (result);
    }


    //************************************************************************************************************************************************************
    //************************************************APRES TRAITEMENT FAIRE ATTENTTION AU STATE SUSPENDED *************************************************************//
    //*******************************************************************************************************************************************************************


    int get_header ( std::map<int, client_buffer*>::iterator it )
    {       
        int ret = 0;
        int i = 0;

        memset (it->second->buffer, 0, 1025);
        ret = recv(it->first, it->second->buffer, 1024, 0);
        if (ret == -1)
        {
            it->second->state_suspended = true;
            return 1;
        }
        if (ret == 0)
        {
            it->second->state_suspended = true;
            return 0;
        }

        for (i = 0; i < ret && it->second->bytes_inside_short_header < 1024 && it->second->is_long_header == - 1; i++)
        {
            it->second->short_header[it->second->bytes_inside_short_header] = it->second->buffer[i];
            it->second->bytes_inside_short_header++;

            if (it->second->bytes_inside_short_header > 3)
            {
                if ( !strncmp( it->second->short_header + it->second->bytes_inside_short_header - 4, "\r\n\r\n", 4 ) )
                {
                    it->second->is_short_header = true;
                    it->second->state_suspended = false;
                    it->second->currently[0] = false;
                    it->second->currently[1] = true;
                    i++;
                    if (i < ret)
                    {
                        for (int j = 0; i < ret; i++, j++)
                        {
                            it->second->rest_header[j] = it->second->buffer[i];
                            it->second->bytes_inside_rest_header++;
                        }
                    }
                    return 1;
                }
            }
        }

        if (it->second->is_long_header == -1 && it->second->bytes_inside_short_header == 1024)
        {
            it->second->is_long_header = false;
            it->second->long_header = new char*[5]();
            if (!it->second->long_header)
                throw std::runtime_error ("new long_header fail");
            for (int i = 0; i < 4; i++)
                it->second->long_header[i] = NULL;
            it->second->long_header[0] = new char[8 * 1024 + 2]();
            if (!it->second->long_header[0])
                throw std::runtime_error ("new long_header[0] fail");

            for (int i = 0; i < 1024; i++)
                it->second->long_header[0][i] = it->second->short_header[i];

            it->second->bytes_inside_long_header[0] = it->second->bytes_inside_short_header;

            while (i < ret)
            {
                it->second->long_header[0][i] = it->second->buffer[i];
                it->second->bytes_inside_long_header[0]++;
            }

            it->second->is_long_header = 0;
            it->second->state_suspended = true;

            return (1);
        }

        if (it->second->is_long_header > -1)
        {
            for (i = 0; i < ret; i++)
            {
                if (it->second->bytes_inside_long_header[it->second->is_long_header] == (8 * 1024))
                {
                    it->second->is_long_header++;
                    if (it->second->is_long_header == 4)
                    {
                        it->second->state_suspended = false;
                        it->second->request_is_invalid = true;
                        throw BadRequest();
                    }
                    it->second->long_header[it->second->is_long_header] = new char[8 * 1024 + 1]();
                    if (!it->second->long_header[it->second->is_long_header])
                        throw std::runtime_error ("new long_header[it->second->is_long_header] fail");
                }

                it->second->long_header[it->second->is_long_header][it->second->bytes_inside_long_header[it->second->is_long_header]] = it->second->buffer[i];
                (it->second->bytes_inside_long_header[it->second->is_long_header])++;
                
                if (it->second->bytes_inside_long_header[it->second->is_long_header] > 3)
                {
                    if ( !strncmp( it->second->long_header[it->second->is_long_header] + it->second->bytes_inside_long_header[it->second->is_long_header] - 4, "\r\n\r\n", 4 ) )
                    {
                        it->second->state_suspended = false;
                        it->second->is_short_header = false;
                        it->second->currently[0] = false;
                        it->second->currently[1] = true;
                        i++;
                        if (i < ret)
                            for (int j = 0; i < ret; i++, j++, it->second->bytes_inside_rest_header++)
                                it->second->rest_header[j] = it->second->buffer[i];
                        return 1;
                    }
                }
            }
        }
        it->second->state_suspended = true;
        return 1;
    }


    int drive_body ( std::map<int, client_buffer*>::iterator it )
    {
        if (it->second->is_short_header)
        {
            it->second->body_size = is_there_a_body(it->second->short_header, it->second->bytes_inside_short_header);
            if (it->second->body_size < 16 * 1024 && it->second->body_size)
                it->second->currently[2] = true;
            if (it->second->body_size >= 16 * 1024)
                it->second->currently[3] = true;
            if (is_there_a_chunk(it->second->short_header, it->second->bytes_inside_short_header))
            {
                it->second->is_chunk = true;
                it->second->currently[4] = true;
            }
            it->second->currently[1] = false;
            return 1;
        }
        if (it->second->is_long_header > -1)
        {
            for (int ret = it->second->is_long_header; ret; ret--)
                it->second->body_size = it->second->body_size + is_there_a_body(it->second->long_header[ret], it->second->bytes_inside_long_header[ret]);
            
            if (it->second->body_size < 16 * 1024 && it->second->body_size)
                it->second->currently[2] = true;
            
            if (it->second->body_size >= 16 * 1024)
                it->second->currently[3] = true;
            
            for (int ret = it->second->is_long_header; ret; ret--)
                if (is_there_a_chunk(it->second->long_header[ret], it->second->bytes_inside_long_header[ret]))
                    it->second->currently[4] = true;

            it->second->currently[1] = false;

            return 1;
        }
        return 1;
    }

    int get_chunk_body ( std::map<int, client_buffer*>::iterator it )
    {
        it->second->is_chunk = true;
        int ret = 0;
        int number = 0;

            ///////////////////////////////////////////////////REST DU HEADER//////////////////////////////////////////////

            if (!it->second->state_suspended)
            {
                
                int test = 0;
                it->second->am_i_inside_chunk = false;
                it->second->bytes_inside_chunk_body = 0;

                it->second->short_body = new char[16 * 1024 + 1]();
                if (!it->second->short_body)
                        throw std::runtime_error ("new short_body fail");
                memset(it->second->short_body, 0, 16 * 1024);

                if (it->second->bytes_inside_rest_header)
                {
                    for (int i = 0; ; i++ )
                    {
                        if (i > 1)
                        {
                        if (!strncmp(it->second->rest_header + i - 2, "\r\n", 2))
                        {
                            it->second->chunktowrite = number = ft_atoi_base(it->second->rest_header, 16);
                            test = i;
                            break;
                        }
                        }
                    }
                    if (number == 0)
                    {
                        it->second->state_suspended = false;
                        return 1;
                    }
                    ///////////////////////////////////////////////////
                    for (int i = test; i < it->second->bytes_inside_rest_header; i++)
                    {
                            it->second->short_body[it->second->bytes_inside_chunk_body] = it->second->rest_header[i];
                            it->second->bytes_inside_chunk_body++;
                            it->second->total++;
                            it->second->chunktowrite--;
                    }
                    it->second->copy = false;
                }
                delete [] it->second->buffer;
                it->second->buffer = new char[16 * 1024 + 1]();
                if (!it->second->buffer)
                        throw std::runtime_error ("new buffer fail");
                it->second->is_long_body = false;
                it->second->is_short_body = true;
                it->second->state_suspended = true;
                if (it->second->chunktowrite)
                    it->second->am_i_inside_chunk = true;
                return (1);
            }

        ///////////////////////////////////////////////////LECTURE//////////////////////////////////////////////

        memset( it->second->buffer, 0, 16 * 1024);
        number = 0;
        ret = recv(it->first, it->second->buffer, 16 * 1024 , 0);

        if (ret > 3)
        {
            if (!strncmp(it->second->buffer, "\r\n\r\n", 4))
            {
                it->second->state_suspended = false;
                return 1;

            }
        }

        ///////////////////////////////////////////////////ECRITURE//////////////////////////////////////////////

        for (int i = 0; i < ret; i++)
        {
            ///////////////////////////////////////////////////DANS LE FD//////////////////////////////////////////////

            if (it->second->bytes_inside_chunk_body == 1024 * 16)
            {
                it->second->is_short_body = false;
                it->second->is_long_body = true;

                if (!it->second->bytes_inside_fd_body)
                {
                    it->second->_file_long_body = std::tmpfile();                
                }
                
                    

                    write(fileno(it->second->_file_long_body), it->second->short_body, (1024 * 16) - 10);
                    it->second->bytes_inside_fd_body += (1024 * 16) - 10;

                    for (int i = 0; i < 10; i++)
                    {
                        it->second->short_body[i] = it->second->short_body[it->second->bytes_inside_chunk_body - 10 + i];
                    }
                    it->second->bytes_inside_chunk_body = 10;
            }

            ///////////////////////////////////////////////////DANS LE BODY//////////////////////////////////////////////

            it->second->short_body[it->second->bytes_inside_chunk_body] = it->second->buffer[i];
            it->second->bytes_inside_chunk_body++;
            it->second->total++;
            it->second->chunktowrite--;
          

            if (it->second->bytes_inside_chunk_body > 1)
            {
                if (!strncmp(it->second->short_body + it->second->bytes_inside_chunk_body - 2, "\r\n", 2))
                { 
                    if ( it->second->chunktowrite < 0 && it->second->copy == false )
                    {
                        it->second->am_i_inside_chunk = false;
                        it->second->utils = it->second->bytes_inside_chunk_body;
                        it->second->copy = true;
                        continue;
                    }
                    if ( it->second->copy == true )
                    {
                        number = ft_atoi_base(it->second->short_body + it->second->utils, 16);
                        if (number == 0)
                        {
                            memset(it->second->short_body + it->second->bytes_inside_chunk_body - 3, 0, 3);
                            if (it->second->is_long_body)
                            {
                                for (int i = 0; i < it->second->bytes_inside_chunk_body - 3; i++)
                                {
                                    it->second->file << it->second->short_body[i];
                                    it->second->bytes_inside_fd_body++;
                                }
                            }
                            it->second->state_suspended = false;
                            return 1;
                        }
                        it->second->chunktowrite = number;
                        memset(it->second->short_body + it->second->utils, 0, it->second->bytes_inside_chunk_body - it->second->utils);
                        it->second->total = it->second->total - (it->second->bytes_inside_chunk_body - it->second->utils);
                        it->second->bytes_inside_chunk_body = it->second->bytes_inside_chunk_body - (it->second->bytes_inside_chunk_body - it->second->utils);
                        it->second->am_i_inside_chunk = false;
                        it->second->copy = false;
                    }
                }
            }
        }
        return (1);
        
    }

    int get_short_body ( std::map<int, client_buffer*>::iterator it )
    {
        int ret = 0;
        
        if (!it->second->state_suspended)
        {
            delete [] it->second->buffer;
            
            it->second->buffer = new char[16 * 1024 + 1]();
            if (!it->second->buffer)
                throw std::runtime_error ("new buffer fail");
            it->second->state_suspended = true;
            it->second->short_body = new char[16 * 1024 + 1]();
            if (!it->second->short_body)
                throw std::runtime_error ("new short_body fail");
            
            for (int i = 0; i < it->second->bytes_inside_rest_header; i++)
            {
                it->second->short_body[i] = it->second->rest_header[i];
                it->second->bytes_inside_body++;
            }
            
            if (it->second->bytes_inside_body == it->second->body_size)
            {
                it->second->is_short_body = true;
                it->second->state_suspended = false;
                it->second->currently[2] = false;
            }
            return 1;
        }

        ret = recv(it->first, it->second->buffer, 16 * 1024, 0);

        for (int i = 0; i < ret; i++, it->second->bytes_inside_body++)
            it->second->short_body[it->second->bytes_inside_body] = it->second->buffer[i];

        if (it->second->bytes_inside_body == it->second->body_size)
        {
                it->second->is_short_body = true;
                it->second->state_suspended = false;
                it->second->currently[2] = false;
                return 1;
        }
        it->second->state_suspended = true;
        return 1;
    }

    int get_long_body ( std::map<int, client_buffer*>::iterator it )
    {
        struct stat sb;
        int ret = 0;

        if (it->second->body_size >= 1000000)
        {
            it->second->state_suspended = false;
            it->second->request_is_invalid = true;
            throw BadRequest();
        }
        if (!it->second->state_suspended)
        {
            it->second->_file_long_body = std::tmpfile();
            write(fileno(it->second->_file_long_body), it->second->rest_header, it->second->bytes_inside_rest_header);
            
            delete [] it->second->buffer;
            
            it->second->buffer = new char[5 * 1024 + 1]();
            if (!it->second->buffer)
                throw std::runtime_error ("new buffer fail");

            fstat(fileno(it->second->_file_long_body), &sb);
            it->second->size = sb.st_size;

            if (it->second->size == it->second->body_size)
            {
                it->second->state_suspended = false;
                it->second->is_long_body = true;
                it->second->currently[3] = false;
                return (1);
            }
        }
        memset ( it->second->buffer, 0, 1024 );
        ret = recv(it->first, it->second->buffer, 1024, 0);
        usleep (500);
        write(fileno(it->second->_file_long_body), it->second->buffer, ret);
        usleep (500);

        fstat(fileno(it->second->_file_long_body), &sb);
        it->second->size = sb.st_size;
        
        if (it->second->size == it->second->body_size)
        {
            it->second->state_suspended = false;
            it->second->is_long_body = true;
            it->second->currently[3] = false;
            std::rewind(it->second->_file_long_body);
            return (1);
        }
        it->second->state_suspended = true; 
        return 1;
    }

    void refresh_client(int fd)
    {

        std::map<int, client_buffer*>::iterator it = tree.find(fd);


        if (it->second->is_long_header > -1)
        {

            for (; it->second->is_long_header != -1; it->second->is_long_header--)
            {
                delete [] it->second->long_header[it->second->is_long_header];
            }
            delete [] it->second->long_header;
        }

        if (it->second->is_short_body)
        {
            delete [] (it->second->short_body);
            it->second->short_body = NULL;
        }
        if (it->second->is_chunk)
        {
            if (it->second->is_long_body)
            {
                delete [] (it->second->short_body);
                std::fclose (it->second->_file_long_body);
                remove("temporary");
                it->second->is_long_body = false;

            }
            if (it->second->is_short_body) 
            {
                delete [] (it->second->short_body);
                it->second->short_body = NULL;
            }
        }
        if (it->second->is_long_body)
        {
            std::fclose (it->second->_file_long_body);
        }

        it->second->bytes_inside_short_header = 0;
        for (int i = 0; i < 4; i++)
            it->second->bytes_inside_long_header[i] = 0;
        it->second->bytes_inside_rest_header = 0;

        it->second->total = 0;
        it->second->bytes_inside_body = 0;
        it->second->bytes_inside_chunk_body = 0;
        it->second->chunktowrite = 0;
        it->second->body_size = 0;
        it->second->state_suspended = false;
        it->second->copy = true;
        it->second->utils = 0;

        it->second->bytes_inside_fd_body = 0;

        it->second->is_short_body = false;
        it->second->is_short_header = false;
        it->second->is_long_header = -1;
        it->second->currently[0] = true;
        for (int i = 1; i < 5; i++)
            it->second->currently[i] = false;

        memset (it->second->short_header, 0, 1024);
    }

    void free_node(int fd)
    {
        std::map<int, client_buffer*>::iterator it = tree.find(fd);

        delete[] (it->second->buffer);
        delete[] (it->second->short_header);

        delete(it->second);
    }

    int check_time ()
    {
        struct tm y2k2;
        memset(&y2k2, 0, sizeof(y2k2));
        double seconds;

        for (std::map<int, client_buffer*>::iterator it = tree.begin(); it != tree.end(); it++) 
        {
            y2k2.tm_hour = 0;   y2k2.tm_min = 0; y2k2.tm_sec = 0;
            y2k2.tm_year = 100; y2k2.tm_mon = 0; y2k2.tm_mday = 1;

            time(&it->second->timer);
            seconds = difftime(it->second->timer,mktime(&y2k2));
            
                it->second->last_EPOLL = seconds - it->second->secondsy2k1;

                if (it->second->last_EPOLL > it->second->last_EPOLL2)
                {
                    it->second->last_EPOLL2 = it->second->last_EPOLL;
                    if (it->second->last_EPOLL2 > 5)
                    {
                        int i = it->first;
                        free_node(it->first);
                        tree.erase(it);
                        return i;
                    }
                }
        }
        return 0;
    }

    bool check_suspended ( int file_descriptor )
    {
        std::map<int, client_buffer*>::iterator it = tree.find(file_descriptor);
        return it->second->state_suspended;
    }

    std::map<int, client_buffer*> get_tree ()
    {
        return tree;
    }

    void free_tree ()
    {
        
        for (std::map<int, client_buffer*>::iterator it = tree.begin(); it != tree.end(); it++) 
        {
            this->refresh_client(it->first);
            this->free_node(it->first);
        }
    }
    private :
    std::map<int, client_buffer*> tree;
};

#endif
