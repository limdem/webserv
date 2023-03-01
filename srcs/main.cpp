#include <fcntl.h>
#include "../includes/Epoll_class.hpp"
#include <signal.h>


int main(int argc, char **argv)
{

    if (argc != 2)
        return 1;
    
    if (!check_file(argv[1]))
        return 1;

    Cluster             cluster;
    cluster.put_in_vector(argv[1]);
    if (!cluster.check_point_virgule())
    {
        std::cerr << "Error : missing ; ";
        return(1);
    }
    if (!cluster.config())
        return (1);

    socket_class        instance;
    Epoll_struct        epoll_struct(cluster);
    
    try {
        Epoll_class         epoll_class; 

        epoll_class.launch_server(&epoll_struct, &instance, cluster);
    }

    catch( const std::exception & e ) {
        std::cerr << e.what() << std::endl;
        
        if (epoll_struct.socket_client != -1)
            close (epoll_struct.socket_client);
        for (int i = 0; i < epoll_struct.number_server_max; i++)
            close (epoll_struct.socket_server[i]);
        for (int i = 0; i < epoll_struct.number_server_max; i++)
            close (epoll_struct.epoll_fd[i]);
        instance.free_tree();
    }
}