#include <stdio.h>
#include <string.h>
#include <errno.h> 
#include "aesdsocket.h"

int main(int argc, char **argv)
{
    int sock = create_socket();
    if(sock == -1)
    {
        printf("error in opening socket: %s", strerror(errno));
        return 1;
    }

    close_socket(sock);

    return 0;
}