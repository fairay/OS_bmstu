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

// LOCKMODE - чтение для пользователя, группы и остальных, запись для пользователя
#define LOCKFILE "/var/run/daemon.pid"
#define LOCKMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)


sigset_t mask; // Сигнальная маска

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
     Перенаправление потоков вывода на пустое устройство
    */
    int fd0, fd1, fd2;
    fd0 = open("/dev/null", O_RDWR);        // Открытие файла пустого устройства
    fd1 = dup(0);                           // Дублирование дескриптора файла fd0
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

// lockfile пробует установить совместную блокировку на файл
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

// already_running проверяет наличие другого запущенного демона
// с помощью блокировки LOCKFILE 
int already_running(void)
{
    // Открытие файла LOCKFILE
    // O_RDWR - режим чтения и записи
    // O_CREAT - создание файла, если он не существует
    int fd = open(LOCKFILE, O_RDWR | O_CREAT, LOCKMODE);

    // Файл не открылся
    if (fd < 0)
    {
        syslog(LOG_ERR, "%s open failed", LOCKFILE);
        exit(1);
    }

    // Попытка установить блокировку
    if (lockfile(fd) == -1)
    {
        /// Не повезло, не повезло

        // EACCES, EAGAIN - файл заблокирован другм процессом-демоном
        if (errno == EACCES || errno == EAGAIN)
        {
            close(fd);
            return 1;
        }

        // На случай непредвиденной ошибки
        syslog(LOG_ERR, "%s blocking failed", LOCKFILE);
        exit(1);
    }

    /// Повезло, повезло, файл не заблокирован другим процессом
    char buf[16];

    // Усечение размера файла до 0 байт
    ftruncate(fd, 0);
    // Печать PID в buf, а buf в LOCKFILE
    sprintf(buf, "%ld", (long)getpid());
    write(fd, buf, strlen(buf) + 1);

    return 0;
}

// Функция потока
// Обрабатывает сигналы поступающие демону
void* thr_fn(void* arg)
{
    int err, signo;

    for (;;)
    {
        // Ожидание сигнала заданного маской
        // Согласно маске обрабатываются любые сигналы
        err = sigwait(&mask, &signo);
        if (err)
        {
            // Ошибка при ожидании
            syslog(LOG_ERR, "sigwait call error");
            exit(1);
        }

        time_t ttime = time(NULL);
        switch (signo)
        {
        case SIGHUP:
            // Вывод логина и текущего времени в log
            syslog(LOG_INFO, "LOGIN: %s; TIME: %s", getlogin(), asctime(localtime(&ttime)));
            break;
        case SIGTERM:
            // Завершение демона
            syslog(LOG_INFO, "Got SIGTERM, exiting");
            exit(0);
        default:
            // Другие сигналы не обрабатываются 
            syslog(LOG_INFO, "Got unknown signal");
        }
    }
    return 0;
}



int main(int argc, char *argv[])
{
    // Вычленение из имени файла из его пути
    char* cmd = strrchr(argv[0], '/');
    if (cmd)
        cmd++;
    else
        cmd = argv[0];
    
    // Создание демона
    deamonize(cmd);

    // Проверка на наличие другого демона
    if (already_running())
    {
        syslog(LOG_ERR, "deamon is already running");
        exit(1);
    }

    int err;
    struct sigaction sa;

    sa.sa_handler = SIG_DFL;    // стандартное поведение SIGHUP
    sigemptyset(&sa.sa_mask);   // никакие сигналы не игнорируются 
    sa.sa_flags = 0;            // флаги не выставленны, стандартное поведение
    err = sigaction(SIGHUP, &sa, NULL); // вызов изменения поведения SIGHUP с SIG_IGN на SIG_DFL
    if (err == -1)
    {
        syslog(LOG_ERR, "It's impossible to recover SIG_DFL for SIGHUP");
        exit(1);
    }
    
    sigfillset(&mask); // ВСЕ сигналы входят в mask
    err = pthread_sigmask(SIG_BLOCK, &mask, NULL);  // Блокировка всех сигналов в данном поток
    if (err)
    {
        syslog(LOG_ERR, "SIG_BLOCK processing error");
        exit(1);
    }

    pthread_t tid;
    err = pthread_create(&tid, NULL, thr_fn, 0); // Создание потока для обработки сигналов
    if (err)
    {
        syslog(LOG_ERR, "It's imposible to create a thread");
        exit(1);
    }

    syslog(LOG_INFO, "Deamon is running");
    err = pthread_join(tid, NULL);      // Ожидание завершения потока 
    if (err)
    {
        syslog(LOG_ERR, "It's imposible to join the thread");
        exit(1);
    }

    return 0;
}
