#include <stdio.h>
#include <fcntl.h>
#include <pthread.h>

#define FNAME   "alph.txt"

void* thread_f(void *data)
{
    int fd = *((int*)data);
    FILE *fs2 = fdopen(fd,"r");
    char buff2[20];
    setvbuf(fs2,buff2,_IOFBF,20);

    int flag = 1;
    while (flag == 1)
    {
        char c; 
        flag = fscanf(fs2,"%c",&c);
        if (flag == 1)
            fprintf(stdout,"%c",c);
    }
}

int main()
{
    int fd = open(FNAME,O_RDONLY);
    if (fd == -1)
    {
        printf("Open failed\n");
        return -1;
    }

    pthread_t tid;
    int err = pthread_create(&tid, NULL, thread_f, (void*)(&fd));
    if (err)
    {
        printf("It's imposible to create a thread");
        return -1;
    }

    FILE *fs1 = fdopen(fd,"r");
    char buff1[20];
    setvbuf(fs1,buff1,_IOFBF,20);

    int flag1 = 1;
    while(flag1 == 1)
    {
        char c;

        flag1 = fscanf(fs1,"%c",&c);
        if (flag1 == 1)
            fprintf(stdout,"%c",c);
    }

    err = pthread_join(tid, NULL);
    if (err)
    {
        printf("It's imposible to join the thread");
        return -1;
    }

    printf("\n");

    return 0;
}
