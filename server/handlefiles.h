#include <unistd.h>

ssize_t read_file(int pfd, void *buffer, size_t nbytes);

int close_file(int pfd);

int clear_file(char *pathname);

