#include "webserv.hpp"
#include <fcntl.h>
#include <readline/readline.h>

void	handler(int signal)
{
	if (signal == SIGINT)
    {
		throw std::runtime_error ("signal == SIGINT");
    }
	if (signal == SIGQUIT)
    {
		throw std::runtime_error ("signal == SIGQUIT");
    }
}


struct Epoll_struct
{
    Epoll_struct(Cluster cluster)
    {
        number_server_max = cluster.get_servers().size();
        ret = 0;
        event_wait = new struct epoll_event* [number_server_max];
        for (int i = 0 ; i < number_server_max; i++)
            event_wait[i] = new struct epoll_event [32];
        port = new int [number_server_max];
        epoll_fd = new int [number_server_max];
        socket_server = new int [number_server_max];
        number_server = 0;
        fd = 0;
        socket_client = -1;
        memset(&sig, 0, sizeof(struct sigaction));
        sig.sa_handler = &handler;
	    sigaction(SIGINT, &sig, 0);
	    sigaction(SIGQUIT, &sig, 0);
    }

    ~Epoll_struct()
    {
        for (int i = 0 ; i < number_server_max; i++)
            delete [] event_wait[i];
        delete [] event_wait;
        delete [] port;
        delete [] epoll_fd;
        delete [] socket_server;
    }
    
    int                         number_server_max;
    int                         nb_fd_ready;
    int                         ret;
    struct epoll_event          **event_wait;
    int                         *port;
    int                         number_server;
    int                         *epoll_fd;
    Static_response             rsp_hdl;
    int                         fd;
    int                         *socket_server;
    struct sigaction	        sig;
    int                         socket_client;
};

class Epoll_class 
{
    public :

    void launch_server (Epoll_struct *epoll_struct, socket_class *instance, Cluster cluster)
    {
        /* DECLARATION SOCKET & STRUCTURE SERVEUR */
        SOCKADDR_IN                 struct_server[epoll_struct->number_server_max];
        socklen_t recsize =         sizeof(struct_server);

        /* DECLARATION SOCKET & STRUCTURE CLIENT */
        SOCKADDR_IN                 struct_client[epoll_struct->number_server_max];
        socklen_t crecsize =        sizeof(struct_client);


        /*ASSIGNATION DES PORTS*/
        for (int i = 0; i < epoll_struct->number_server_max; i++)
        {
            if ((epoll_struct->port[i] = cluster.get_servers()[i].get_port()) < 0)
                throw std::runtime_error ("ports");
        }

        /* CREATION FILE DESCRIPTOR EPOLL */
        for (int i = 0; i < epoll_struct->number_server_max; i++)
        {
            if ((epoll_struct->epoll_fd[i] = epoll_create(1)) < 0)
                throw std::runtime_error ("epoll_create");
        }

        /* CREATION DU SOCKET */
        for (int i = 0; i < epoll_struct->number_server_max; i++)
        {
            if ((epoll_struct->socket_server[i] = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) < 0)
                throw std::runtime_error ("socket_server");
        }

        /* ASSIGNATION STRUCTURE FD SERVERS */

        for (int i = 0; i < epoll_struct->number_server_max; i++)
        {
            struct_server[i].sin_addr.s_addr =         htonl(INADDR_ANY);  
            struct_server[i].sin_family =              AF_INET;            
            struct_server[i].sin_port =                htons(epoll_struct->port[i]);
        }

        for (int i = 0; i < epoll_struct->number_server_max; i++)
        {
            int on = 1;
            if (setsockopt(epoll_struct->socket_server[i], SOL_SOCKET,  SO_REUSEADDR | SO_REUSEPORT, &on, sizeof(int)) == -1)
                throw std::runtime_error ("set socket operation");
        }

        /*AFFECTATION D'UN NOM A UNE SOCKET*/
        //Bind permet de lier un port a une socket
        for (int i = 0; i < epoll_struct->number_server_max; i++)
        {
            epoll_struct->ret = bind(epoll_struct->socket_server[i], (SOCKADDR*)&struct_server[i], recsize);
            if (epoll_struct->ret == -1)
                throw std::runtime_error ("epoll_create");
        }

        /*ECOUTE DES SOCKET SERVEUR*/
        //Devinet une soquette passive qui va ecouter 
        for (int i = 0; i < epoll_struct->number_server_max; i++)
        {
            epoll_struct->ret = listen(epoll_struct->socket_server[i], 32);
            if (epoll_struct->ret == -1)
                throw std::runtime_error ("listen");
        }

        /*AJOUT DES FD SERVEUR AU POOL DE FD A MONITORER*/

        for (int i = 0; i < epoll_struct->number_server_max; i++)
        {
            struct epoll_event event_server;
            memset(&event_server, 0, sizeof(event_server));

            event_server.events  = EPOLLIN;
            event_server.data.fd = epoll_struct->socket_server[i];
            epoll_struct->ret = epoll_ctl(epoll_struct->epoll_fd[i], EPOLL_CTL_ADD, epoll_struct->socket_server[i], &event_server);
        }

        /* BOUCLE DE N-SERVEUR NON BLOQUANTE*/

        int i = 0;
        Static_response   rsp_hdl;

        while (42)
        {
                

                epoll_struct->nb_fd_ready = epoll_wait(epoll_struct->epoll_fd[epoll_struct->number_server], epoll_struct->event_wait[epoll_struct->number_server], 32, 0 );
                if (epoll_struct->nb_fd_ready)
                {
                    for (int i = 0; i < epoll_struct->nb_fd_ready; i++) 
                    {
                        int fd = epoll_struct->event_wait[epoll_struct->number_server][i].data.fd;

                        if (epoll_struct->event_wait[epoll_struct->number_server][i].events == EPOLLIN) //SI LE FD TRIGGER EPOLL POUR UNE LECTURE
                        {
                            
                            if (fd == epoll_struct->socket_server[epoll_struct->number_server]) 
                            {
                                
                                epoll_struct->socket_client = accept(epoll_struct->socket_server[epoll_struct->number_server], (struct sockaddr*)&struct_client[epoll_struct->number_server], &crecsize);

                                int flags = fcntl(epoll_struct->socket_client, F_GETFL, 0);
                                fcntl(epoll_struct->socket_client, F_SETFL, flags | O_NONBLOCK);

                                struct epoll_event event;
                                memset(&event, 0, sizeof(event));

                                event.events  = EPOLLIN;         
                                event.data.fd = epoll_struct->socket_client;
                                epoll_ctl(epoll_struct->epoll_fd[epoll_struct->number_server], EPOLL_CTL_ADD, epoll_struct->socket_client, &event);

                                instance->add_client(epoll_struct->socket_client);
                                epoll_struct->rsp_hdl.add_client(epoll_struct->socket_client);
                                fd = epoll_struct->socket_client;
                            }
                            try 
                            {
                                if (!instance->traitement(fd))
                                {
                                    epoll_ctl(epoll_struct->epoll_fd[epoll_struct->number_server], EPOLL_CTL_DEL, fd, 0);
                                    epoll_struct->rsp_hdl.delete_client(fd);
                                    close (fd);
                                }
                            }
                            catch (BadRequest& mce) {}

                            if (!instance->check_suspended(fd))
                            {
                            
                                std::map<int, client_buffer*> it = instance->get_tree();
                                Request_parser  req_pars(it.find(fd)->second, fd);
                                try
                                {
                                    req_pars.parse_request();
                                }
                                catch(const std::exception& e)
                                {}
                                 
                                epoll_struct->rsp_hdl.setRequestParser(req_pars);


                                

                                std::vector<std::string> host_ = split2(req_pars.getRequestHostname(), ':');
                                int port;
                                if (host_.size() == 1)
                                    port = 13879;
                                else
                                {
                                    const char* port_ = host_[1].c_str();
                                    port = std::atoi(port_);
                                }

                                Request request(req_pars.getRequestMethod(), req_pars.getRequestHostname(), req_pars.getRequestUri(), host_[0], port);
                                AlgoServer algo1(cluster, request);
                                Server     server_choosen;
                                server_choosen = algo1.run();
                                epoll_struct->rsp_hdl.set_error_pages(server_choosen.get_error_page());
                                epoll_struct->rsp_hdl.set_server_root(server_choosen.get_root());
                                AlgoLocation algo2(server_choosen, request);
                                Location location_chosen = algo2.run();
                                epoll_struct->rsp_hdl.set_status(location_chosen.get_status());
                                while (location_chosen.get_redirection().size())
                                {   
                                    request.set_path(location_chosen.get_redirection());
                                    algo2.set_request(request);
                                    location_chosen = algo2.run();
                                }   
                                epoll_struct->rsp_hdl.set_autoIndex(location_chosen.get_autoIndex());
                                
                                

                                if (location_chosen.get_client_body_size() == 0)
                                    location_chosen.set_client_max_body_size(server_choosen.get_client_body_size());

                                epoll_struct->rsp_hdl.set_client_max_body_size(location_chosen.get_client_body_size());
                               

                                std::string title = location_chosen.get_title();
                                std::string::size_type pos = title.find_last_of('.');
                                if (pos != std::string::npos)
                                {
                                    std::string extension = title.substr(pos);
                                    req_pars.set_extension(extension);
                                }




                                handle_request_execution(req_pars, location_chosen, epoll_struct->rsp_hdl, fd);
                                struct epoll_event event;
                                memset(&event, 0, sizeof(event));
                                event.events = EPOLLOUT;         
                                event.data.fd = fd;
                                epoll_ctl(epoll_struct->epoll_fd[epoll_struct->number_server], EPOLL_CTL_MOD, fd, &event);
                            }
                        }

                        if (epoll_struct->event_wait[epoll_struct->number_server][i].events == EPOLLOUT) //SI LE FD TRIGGER EPOLL POUR UNE ECRITURE
                        {
                            epoll_struct->rsp_hdl.send_response();
    
                            struct epoll_event event;
                            memset(&event, 0, sizeof(event));

                            event.events  = EPOLLIN;
                            event.data.fd = fd;
                            epoll_ctl(epoll_struct->epoll_fd[epoll_struct->number_server], EPOLL_CTL_MOD, fd, &event);

                            instance->refresh_client(fd);


                        }
                    }
                }
                if ((i = instance->check_time ()) > 0)
                {
                    epoll_ctl(epoll_struct->epoll_fd[epoll_struct->number_server], EPOLL_CTL_DEL, i, 0);
                    epoll_struct->rsp_hdl.delete_client(i);
                    close (i);
                }

                epoll_struct->number_server++;
                if (epoll_struct->number_server == epoll_struct->number_server_max)
                    epoll_struct->number_server = 0;
        }
    }

   
};  