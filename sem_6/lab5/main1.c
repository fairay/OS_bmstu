#include <stdio.h>
#include <fcntl.h>

#define FNAME   "alph.txt"

int main()
{
  int fd = open(FNAME,O_RDONLY);
  if (fd == -1)
  {
    printf("Open failed\n");
    return -1;
  }

  FILE *fs1 = fdopen(fd,"r");
  char buff1[20];
  setvbuf(fs1,buff1,_IOFBF,20);

  FILE *fs2 = fdopen(fd,"r");
  char buff2[20];
  setvbuf(fs2,buff2,_IOFBF,20);
  
  int flag1 = 1, flag2 = 1;
  while(flag1 == 1 || flag2 == 1)
  {
    char c;

    flag1 = fscanf(fs1,"%c",&c);
    if (flag1 == 1)
      fprintf(stdout,"%c",c);
    
    flag2 = fscanf(fs2,"%c",&c);
    if (flag2 == 1)
      fprintf(stdout,"%c",c); 
  }

  printf("\n");

  return 0;
}
