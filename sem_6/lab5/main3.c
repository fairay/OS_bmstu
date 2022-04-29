#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h> 

#define FNAME   "out.txt"

int main()
{
    struct stat statbuf;

    FILE* fd1 = fopen(FNAME, "w");
    if (!fd1)
    {
        printf("Fopen failed\n");
        return -1;
    }
    stat(FNAME, &statbuf);
    printf("%ld, %lu\n", statbuf.st_size, statbuf.st_ino);

    FILE* fd2 = fopen(FNAME, "w");
    if (!fd2)
    {
        printf("Fopen failed\n");
        fclose(fd1);
        return -1;
    }
    stat(FNAME, &statbuf);
    printf("%ld, %lu\n", statbuf.st_size, statbuf.st_ino);

    for (int i=0; i<26; i++)
    {
        if (i % 2)
            fprintf(fd2, "%c", 'a' + i);
        else
            fprintf(fd1, "%c", 'a' + i);
    }

    fclose(fd2);
    stat(FNAME, &statbuf);
    printf("%ld, %lu\n", statbuf.st_size, statbuf.st_ino);

    fclose(fd1);
    stat(FNAME, &statbuf);
    printf("%ld, %lu\n", statbuf.st_size, statbuf.st_ino);

    return 0;
}