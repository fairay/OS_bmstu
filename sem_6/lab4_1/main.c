#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>

#define BUF_SIZE 0x300

void null_str(char *str, int size)
{
    for (int i=0; i<size; i++)
        str[i] = '\0';
}

void read_link(const char* name)
{
    char file_name[BUF_SIZE];
    sprintf(file_name, "%s.txt", name);
    FILE *fout = fopen(file_name, "w");

    char file_path[BUF_SIZE], buf[BUF_SIZE];
    sprintf(file_path, "/proc/self/%s", name);
    null_str(buf, BUF_SIZE);
    readlink(file_path, buf, BUF_SIZE);
    
    fprintf(fout, "%s", buf);
    fclose(fout);
}

void read_fd()
{
    DIR *dp = opendir("/proc/self/fd");
    FILE *fout = fopen("fd.txt", "w");
    char file_path[BUF_SIZE], buf[BUF_SIZE];

    struct dirent *dirp;
    while ((dirp = readdir(dp)) != NULL) 
    {
        if ((strcmp(dirp->d_name, ".") != 0) &&
            (strcmp(dirp->d_name, "..") != 0)) 
        {
            sprintf(file_path, "/proc/self/fd/%s", dirp->d_name);

            readlink(file_path, buf, BUF_SIZE);
            fprintf(fout, "%s\n", buf);
        }
    }

    closedir(dp);
    fclose(fout);
}

void read_file(const char* name)
{
    char file_name[BUF_SIZE];
    sprintf(file_name, "%s.txt", name);
    FILE *fout = fopen(file_name, "w");

    char file_path[BUF_SIZE], buf[BUF_SIZE];
    sprintf(file_path, "/proc/self/%s", name);
    FILE *f = fopen(file_path, "r");

    int len, i;
    while ((len=fread(buf, 1, BUF_SIZE, f)) != 0)
    {
        for (i=0; i<len; i++)
            if (buf[i] == 0)
                buf[i] =10;
        buf[len] = 0;
        fprintf(fout, "%s", buf);
    }

    fclose(f);
    fclose(fout);
}

int main (int argc, char* argv)
{
    char buf[BUF_SIZE];
    int len, i;

    read_fd();

    read_link("exe");  
    read_link("root");
    read_link("cwd");  
    
    read_file("cmdline");
    read_file("environ");
    read_file("maps");
    read_file("mem");
    read_file("stat");
    read_file("statm");

    /*
    FILE *f = fopen("/proc/self/maps", "r");
    FILE *fout = fopen("out.txt", "w");

    while ((len=fread(buf, 1, BUF_SIZE, f)) != 0)
    {
        for (i=0; i<len; i++)
            if (buf[i] == 0)
                buf[i]=10;
        buf[len] = 0;
        printf("%s", buf);        
        fprintf(fout, "%s", buf);
    }

    fclose(f);
    fclose(fout);
    */

    return 0;
}