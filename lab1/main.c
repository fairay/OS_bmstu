#include <syslog.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>

#include <sys/resource.h>
#include <sys/stat.h> 
#include <sys/file.h>

#define LOCKFILE "/var/run/daemon.pid"
#define LOCKMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)


sigset_t mask;

void deamonize(const char* cmd)
{
    umask(0);

    pid_t pid = fork();
    if (pid < 0)
        perror("%s: fork call error \n");
    else if (pid != 0)
    {
        printf("DAEMON PID: %d\n", pid);
        exit(0);
    }
    setsid();

    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGHUP, &sa, NULL) == -1)
        perror("It's impossible to ignore SIGHUP \n");

    if (chdir("/") < 0)
        perror("It's imposible to change work directory to / \n");

    struct rlimit rl;
    if (getrlimit(RLIMIT_NOFILE, &rl) == -1)
        perror("It's impossible to get max number of file descriptor \n");    
    if (rl.rlim_cur == RLIM_INFINITY)
        rl.rlim_max = 1024;
    for (int i = 0; i<rl.rlim_max; i++)
        close(i);
    
    int fd0, fd1, fd2;
    fd0 = open("/dev/null", O_RDWR);
    fd1 = dup(0);
    fd2 = dup(0);
    
    openlog(cmd, LOG_CONS, LOG_DAEMON);
    
    if (fd0 != 0 || fd1 != 1 || fd2 != 2)
    {
        syslog(LOG_ERR, "error file descriptors %d %d %d", fd0, fd1, fd2);
        exit(1);
    }
}

int lockfile(int fd)
{
    struct flock fl;
    fl.l_type = F_WRLCK;
    fl.l_start = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len = 0;
    return fcntl(fd, F_SETLK, &fl);
}

int already_running(void)
{
    int fd = open(LOCKFILE, O_RDWR | O_CREAT, LOCKMODE);

    if (fd < 0)
    {
        syslog(LOG_ERR, "%s open failed", LOCKFILE);
        exit(1);
    }

    if (lockfile(fd) == -1)
    {
        if (errno == EACCES || errno == EAGAIN)
        {
            close(fd);
            return 1;
        }
        syslog(LOG_ERR, "%s blocking failed", LOCKFILE);
        exit(1);
    }

    char buf[16];
    ftruncate(fd, 0);
    sprintf(buf, "%ld", (long)getpid());
    write(fd, buf, strlen(buf) + 1);

    return 0;
}


void* thr_fn(void* arg)
{
    int err, signo;

    for (;;)
    {
        err = sigwait(&mask, &signo);
        if (err)
        {
            syslog(LOG_ERR, "sigwait call error");
            exit(1);
        }

        time_t ttime = time(NULL);
        switch (signo)
        {
        case SIGHUP:
            syslog(LOG_INFO, "LOGIN: %s; TIME: %s", getlogin(), asctime(localtime(&ttime)));
            break;
        case SIGTERM:
            syslog(LOG_INFO, "Got SIGTERM, exiting");
            exit(0);
        default:
            syslog(LOG_INFO, "Got unknown signal");
        }
    }
    return 0;
}


int main(int argc, char *argv[])
{
    int err;
    pthread_t tid;
    char *cmd;
    struct sigaction sa;

    cmd = strrchr(argv[0], '/');
    if (cmd)
        cmd++;
    else
        cmd = argv[0];
    
    deamonize(cmd);

    if (already_running())
    {
        syslog(LOG_ERR, "deamon is already running");
        exit(1);
    }

    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGHUP, &sa, NULL) == -1)
    {
        syslog(LOG_ERR, "It's impossible to recover SIG_DFL for SIGHUP");
        exit(1);
    }
    
    sigfillset(&mask);
    err = pthread_sigmask(SIG_BLOCK, &mask, NULL);
    if (err)
    {
        syslog(LOG_ERR, "SIG_BLOCK processing error");
        exit(1);
    }

    err = pthread_create(&tid, NULL, thr_fn, 0);
    if (err)
    {
        syslog(LOG_ERR, "It's imposible to create a thread");
        exit(1);
    }

    syslog(LOG_INFO, "Deamon is running");
    err = pthread_join(tid, NULL);
    if (err)
    {
        syslog(LOG_ERR, "It's imposible to join the thread");
        exit(1);
    }

    syslog(LOG_INFO, "Deamon stoped");
    return 0;
}