#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>

#define FNAME   "alph.txt"


void* thread_f (void *data)
{
    int fd2 = open(FNAME,O_RDONLY);
    if (fd2 == -1)
        return (void*)-1;

    char c2;
    int flag2 = 1;
    while(flag2)
    {
        flag2 = (read(fd2,&c2,1) == 1);
        if (flag2)  write(1,&c2,1);
    }

    return (void*)0;
}

int main()
{
    char c1;
    
    pthread_t tid;
    int err = pthread_create(&tid, NULL, thread_f, NULL);
    if (err)
    {
        printf("It's imposible to create a thread");
        return -1;
    }

    int fd1 = open(FNAME,O_RDONLY);
    if (fd1 == -1)
    {
        printf("Open failed\n");
        return -1;
    }

    int flag1 = 1;
    while(flag1)
    {
        flag1 = (read(fd1,&c1,1) == 1);
        if (flag1)  write(1,&c1,1);
    }

    int thread_code;
    err = pthread_join(tid, (void**)(&thread_code));
    if (err)
    {
        printf("It's imposible to join the thread");
        return -1;
    }
    if (thread_code == -1)
    {
        printf("Open failed (in thread)\n");
        return -1;
    }

    return 0;
}