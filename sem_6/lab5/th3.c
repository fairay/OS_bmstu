#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h> 
#include <pthread.h>

#define FNAME   "out.txt"

void* thread_f(void *data)
{
    struct stat statbuf;
    FILE* fd2 = fopen(FNAME, "w");
    if (!fd2)
    {
        printf("Fopen failed\n");
        return (void*)-1;
    }
    stat(FNAME, &statbuf);
    printf("%ld, %lu\n", statbuf.st_size, statbuf.st_ino);


    for (int i=1; i<26; i+=2)
        fprintf(fd2, "%c", 'a' + i);
    
    fclose(fd2);
    stat(FNAME, &statbuf);
    printf("%ld, %lu\n", statbuf.st_size, statbuf.st_ino);

    return (void*)0;
}

int main()
{
    pthread_t tid;
    int err = pthread_create(&tid, NULL, thread_f, NULL);
    if (err)
    {
        printf("It's imposible to create a thread");
        return -1;
    }

    ///
    struct stat statbuf;
    FILE* fd1 = fopen(FNAME, "w");
    if (!fd1)
    {
        printf("Fopen failed\n");
        return -1;
    }
    stat(FNAME, &statbuf);
    printf("%ld, %lu\n", statbuf.st_size, statbuf.st_ino);

    for (int i=0; i<26; i+=2)
        fprintf(fd1, "%c", 'a' + i);
    
    fclose(fd1);
    stat(FNAME, &statbuf);
    printf("%ld, %lu\n", statbuf.st_size, statbuf.st_ino);


    ///
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