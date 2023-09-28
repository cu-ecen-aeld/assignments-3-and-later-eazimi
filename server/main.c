#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <time.h>
#include "aesdsocket.h"
#include "queue.h"

#define SLEEP_SECS 10
#define PORT 9000
#define BUFF_SIZE 1024
#define PCKT_SIZE 20 * 1024
#define FILE_PATH "/var/tmp/aesdsocketdata"

bool loop = true;

#define CHECK_EXIT_CONDITION(rc, func_name) \
    do                                      \
    {                                       \
        if ((rc) == -1)                     \
        {                                   \
            exit(EXIT_FAILURE);             \
        }                                   \
    } while (0)

static void signal_handler(int signal_number)
{
    if ((signal_number == SIGINT) || (signal_number == SIGTERM))
    {
        syslog(LOG_INFO, "Caught signal, exiting");
        loop = false;
    }
}

struct thread_info
{
    pthread_mutex_t *mutex;
    int pfd;
    int connfd;
};

struct timer_thread_info
{
    pthread_mutex_t *mutex;
    int pfd;
};

static void *thread_start(void *arg)
{
    struct thread_info *tinfo = (struct thread_info *)arg;
    pthread_mutex_lock(tinfo->mutex);
    while (true)
    {
        /// receive data - write to file
        char recv_buff[BUFF_SIZE + 1];
        memset((void *)recv_buff, 0, BUFF_SIZE + 1);
        int rc_recvdata = recv_data(tinfo->connfd, recv_buff, BUFF_SIZE);
        CHECK_EXIT_CONDITION(rc_recvdata, "recv_data");
        int rc_writefile = write(tinfo->pfd, (const void *)recv_buff, rc_recvdata);
        CHECK_EXIT_CONDITION(rc_writefile, "write_file");
        char *pch = strstr(recv_buff, "\n");
        if (pch != NULL)
            break;
    }

    int data_size = lseek(tinfo->pfd, 0L, SEEK_CUR);
    char send_buff[BUFF_SIZE];
    lseek(tinfo->pfd, 0L, SEEK_SET);
    do
    {
        int rc_readfile = read(tinfo->pfd, send_buff, BUFF_SIZE);
        CHECK_EXIT_CONDITION(rc_readfile, "read_file");
        int rc_senddata = send_data(tinfo->connfd, send_buff, rc_readfile);
        CHECK_EXIT_CONDITION(rc_senddata, "send_data");
        data_size -= rc_readfile;
        memset(send_buff, 0, BUFF_SIZE);
    } while (data_size > 0);
    pthread_mutex_unlock(tinfo->mutex);
    return arg;
}

static void *timer_thread_start(void* arg)
{
    struct timer_thread_info *ttinfo = (struct timer_thread_info *)arg;
    while(loop)
    {
        time_t rawTime;
        struct tm *info;
        char buffer[80];
        time(&rawTime);

        info = localtime(&rawTime);
        strftime(buffer, 80, "%a, %d %b %Y %T %z", info);

        int buffer_len = strlen(buffer);
        buffer[buffer_len] = '\n';
        buffer[buffer_len + 1] = '\0';

        char msg[256];
        sprintf(msg, "timestamp:%s", buffer);

        pthread_mutex_lock(ttinfo->mutex);
        ssize_t rc_write = write(ttinfo->pfd, (const void *)msg, strlen(msg));
        pthread_mutex_unlock(ttinfo->mutex);

        CHECK_EXIT_CONDITION(rc_write, "write");
        
        sleep(SLEEP_SECS);
    }
    return arg;
}

void _daemon()
{
    // PID: Process ID
    // SID: Session ID
    pid_t pid, sid;
    pid = fork(); // Fork off the parent process
    if (pid < 0)
    {
        exit(EXIT_FAILURE);
    }
    if (pid > 0)
    {
        exit(EXIT_SUCCESS);
    }
    // Create a SID for child
    sid = setsid();
    if (sid < 0)
    {
        // FAIL
        exit(EXIT_FAILURE);
    }
    if ((chdir("/")) < 0)
    {
        // FAIL
        exit(EXIT_FAILURE);
    }
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

typedef struct slist_data_s slist_data_t;
struct slist_data_s
{
    pthread_t tid;
    SLIST_ENTRY(slist_data_s) entries;
};

int main(int argc, char **argv)
{
    slist_data_t *datap = NULL;
    SLIST_HEAD(slisthead, slist_data_s) head;
    SLIST_INIT(&head);

    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);

    /// create socket
    int sockfd = create_socket();
    CHECK_EXIT_CONDITION(sockfd, "create_socket");

    char port[5];
    memset(port, 0, sizeof port);
    sprintf(port, "%d", PORT);
    int rc_bind = bind_addr(sockfd, port);
    CHECK_EXIT_CONDITION(rc_bind, "bind_addr");

    openlog("server_log", LOG_PID, LOG_USER);

    struct sigaction new_action;
    memset((void *)&new_action, 0, sizeof(struct sigaction));
    new_action.sa_handler = signal_handler;
    if ((sigaction(SIGTERM, &new_action, NULL) != 0) || (sigaction(SIGINT, &new_action, NULL) != 0))
    {
        return 0;
    }

    if (argc == 2 && strcmp(argv[1], "-d") == 0)
    {
        _daemon();
    }

    int rc_listen = listen_conn(sockfd);
    CHECK_EXIT_CONDITION(rc_listen, "listen_conn");

    int pfd = open(FILE_PATH, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    CHECK_EXIT_CONDITION(pfd, "open_file");

    struct timer_thread_info *ttinfo = (struct timer_thread_info *)malloc(sizeof(struct timer_thread_info));
    if (ttinfo == NULL)
    {
        syslog(LOG_ERR, "Could not allocate memory for a timer_thread_info object");
        return 0;
    }
    ttinfo->mutex = &mutex;
    ttinfo->pfd = pfd;

    pthread_t ttid;
    int s = pthread_create(&ttid, NULL, timer_thread_start, (void *)ttinfo);
    if (s != 0)
    {
        syslog(LOG_ERR, "pthread_create() error [timer thread]: %s", strerror(errno));
        free(ttinfo);
        return 0;
    }

    while (loop)
    {
        struct sockaddr addr_cli;
        int connfd = accept_conn(sockfd, &addr_cli);
        if (connfd == -1)
        {
            continue;
        }        

        char str_ipcli[BUFF_SIZE];
        get_ipcli(&addr_cli, str_ipcli);
        syslog(LOG_INFO, "Accepted connection from %s", str_ipcli);

        struct thread_info *tinfo = (struct thread_info *)malloc(sizeof(struct thread_info));
        if(tinfo == NULL)
        {
            syslog(LOG_ERR, "Could not allocate memory for a thread_info object");
            continue;
        }
        tinfo->mutex = &mutex;
        tinfo->pfd = pfd;
        tinfo->connfd = connfd;

        pthread_t tid;
        int s = pthread_create(&tid, NULL, thread_start, (void *)tinfo);
        if (s != 0)
        {
            syslog(LOG_ERR, "pthread_create() error: %s", strerror(errno));
            free(tinfo);
            continue;
        }
        datap = (slist_data_t *)malloc(sizeof(slist_data_t));
        datap->tid = tid;
        SLIST_INSERT_HEAD(&head, datap, entries);      
    }

    /// shutdown
    int rc_join;
    void *res = NULL;
    
    SLIST_FOREACH(datap, &head, entries)
    {
        rc_join = pthread_join(datap->tid, &res);
        free(res);
        if(rc_join != 0)
        {
            syslog(LOG_ERR, "pthread_join() error: %s", strerror(errno));
            continue;
        }
        syslog(LOG_ERR, "joined with thread %d\n", (int)datap->tid);
    }

    while(!SLIST_EMPTY(&head))
    {
        datap = SLIST_FIRST(&head);
        SLIST_REMOVE_HEAD(&head, entries);
        free(datap);
    }

    pthread_join(ttid, &res);
    free(res);
    pthread_mutex_destroy(&mutex);
    shutdown(sockfd, SHUT_RDWR);
    close(pfd);
    remove(FILE_PATH);
    closelog();

    return 0;
}