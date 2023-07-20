#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int create_socket()
{
    return socket(AF_INET, SOCK_STREAM, 0);
}

void close_socket(int sock)
{
    close(sock);
}
