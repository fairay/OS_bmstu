#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#define FNAME   "alph.txt"

int main()
{
    char c1, c2;    

    int fd1 = open(FNAME,O_RDONLY);
    int fd2 = open(FNAME,O_RDONLY);
    
    if (fd1 == -1 || fd2 == -1)
    {
        printf("Open failed\n");
        return -1;
    }

    int flag1 = 1, flag2 = 1;
    while(flag1 && flag2)
    {
        flag1 = (read(fd1,&c1,1) == 1);
        flag2 = (read(fd2,&c2,1) == 1);
        if (flag1)  write(1,&c1,1);
        if (flag2)  write(1,&c2,1);
    }

    return 0;
}