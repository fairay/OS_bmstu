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

// Функция daemonize создаёт демона
void deamonize(const char* cmd)
{
    /// 6 шагов к созданию демона

    /* 1.
     Вызов umask для сброса режима создания файлов в 0
     Биты в маске режима создания могут препятствовать
     созданию файлов демоном.
    */
    umask(0);
    
    /* 2.
     Вызов fork, завершение родительского процесса
     На то есть 2 причины:
     * Если процесс был запущен как команда оболочки, 
       то будет считаться, что команда выполнена
     * Новый процесс будет в группе родителя, но точно
       не будет лидером этой группы (т.к. будет иметь другой PID),
       что позволяет вызвать setsid.
    */
    pid_t pid = fork();
    if (pid < 0)
        perror("%s: fork call error \n");
    else if (pid != 0)
    {
        printf("DAEMON PID: %d\n", pid);
        exit(0);
    }

    /* 3.
     Создание новой сессии.
     Новый процесс:
      * лидер сессии и группы
      * не имеет управляющего терминала
     Ошибка в setdis возникает только когда процесс - лидер группы,
     (что исключено после fork())
    */
    setsid();

    // Игнорирование сигнала SIGHUP
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGHUP, &sa, NULL) == -1)
        perror("It's impossible to ignore SIGHUP \n");

    /* 4.
     Назначение корневого каталога текущим (отмонтирование текущей ФС).
     На случай, если демон был запущен из монтированной файловой системы.
    */
    if (chdir("/") < 0)
        perror("It's imposible to change work directory to / \n");
    
    /* 5.
     * Получение максимального возможного номера файлового дискриптора
     * В случае, если текущего ограничения нет, установить его на 1024 (максимум ОС)
     * Закрытие всех потенциальных файловых дескрипторов 
    */
    struct rlimit rl;
    if (getrlimit(RLIMIT_NOFILE, &rl) == -1)
        perror("It's impossible to get max number of file descriptor \n");
    if (rl.rlim_cur == RLIM_INFINITY)
        rl.rlim_max = 1024;
    for (int i = 0; i<rl.rlim_max; i++)
        close(i);
    
    /* 6.
     Перенаправление потоков вывода на null устройство
    */
    int fd0, fd1, fd2;
    fd0 = open("/dev/null", O_RDWR);
    fd1 = dup(0);
    fd2 = dup(0);
    
    if (fd0 != 0 || fd1 != 1 || fd2 != 2)
    {
        syslog(LOG_ERR, "error file descriptors %d %d %d", fd0, fd1, fd2);
        exit(1);
    }

    // Инициализация файла журнала
    // LOG_CONS - вывод в консоль в случае ошибки при логировании
    // LOG_DAEMON - процесс-демон => логируется в daemon.log
    openlog(cmd, LOG_CONS, LOG_DAEMON);
}

int lockfile(int fd)
{
    struct flock fl;
    fl.l_type = F_WRLCK;            // тип блокировки - чтение/запись
    fl.l_start = 0;                 // смещение от блокировки = 0
    fl.l_whence = SEEK_SET;         // блокировка с начала файла
    fl.l_len = 0;                   // блокируются все байты файла

    // установка блокировки
    // в случае неудачи (ЗАНЯТО!) fcntl вернёт -1
    return fcntl(fd, F_SETLK, &fl); 

}

int already_running(void)
{
    int fd;
    char buf[16];

    fd = open(LOCKFILE, O_RDWR | O_CREAT, LOCKMODE);

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