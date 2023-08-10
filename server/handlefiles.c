#include "handlefiles.h"
#include <fcntl.h>
#include <sys/stat.h>

int close_file(int pfd)
{
    return close(pfd);
}

int clear_file(char *pathname)
{
    int pfd = open(pathname, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (pfd > -1)
    {
        close(pfd);
    }
    return pfd;
}