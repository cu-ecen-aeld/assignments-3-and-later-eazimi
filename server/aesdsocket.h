#include <stdio.h>
#include <string.h>
#include <unistd.h>

int create_socket();

int bind_addr(int sockfd, char* port);

int listen_conn(int sockfd);

void close_socket(int sock);