#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h> 
#include "aesdsocket.h"

#define PORT 9000

int main(int argc, char **argv)
{
    int sockfd = create_socket();
    if(sockfd == -1)
    {
        fprintf(stderr, "socket error: %s", strerror(errno));
        return -1;
    }

    char port[5];
    memset(port, 0, sizeof port);
    sprintf(port, "%d", PORT);
    int rc_bind = bind_addr(sockfd, port);
    if(rc_bind == -1)
    {
        fprintf(stderr, "bind_addr error: %s", strerror(errno));
        return -1;
    }

    int rc_listen = listen_conn(sockfd);
    if(rc_listen == -1)
    {
        fprintf(stderr, "listen_conn error: %s", strerror(errno));
        return -1;
    }

    close_socket(sockfd);

    return 0;
}