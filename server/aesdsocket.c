#include "aesdsocket.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <errno.h>

struct addrinfo* addrinfo = NULL;

int create_socket()
{
    return socket(AF_INET, SOCK_STREAM, 0);
}

int bind_addr(int sockfd, char* port)
{
    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int rc_getaddrinfo = getaddrinfo(NULL, port, &hints, &addrinfo); 
    if(rc_getaddrinfo != 0)
    {
        fprintf(stderr, "getaddrinfo error: %s\n", strerror(errno));
        return rc_getaddrinfo;
    }

    return bind(sockfd, addrinfo->ai_addr, sizeof(struct addrinfo));
}

void close_socket(int sock)
{
    free(addrinfo);
    close(sock);
}
