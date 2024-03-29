#include <stdio.h>
#include <stdlib.h>
#include <errno.h> 
#include <unistd.h> 
#include <limits.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h> 

#define FTW_F 	1 
#define FTW_D 	2 
#define FTW_DNR 3 
#define FTW_NS 	4

typedef int info_print_t(const char *, int);

void print_indent(int n)
{
	for (int i=0; i<n; i++)
			printf("    ");
}

static int dopath(const char *filename, int depth, info_print_t *func)
{
	struct stat statbuf;
	struct dirent * dirp;
	DIR *dp;
	int ret = 0;

	print_indent(depth);

	if (lstat(filename, &statbuf) < 0) 
		return(func(filename, FTW_NS));

	if (S_ISDIR(statbuf.st_mode) == 0) 
		return(func(filename, FTW_F));

	func(filename, FTW_D);
    
	if ((dp = opendir(filename)) == NULL)
		return(func(filename, FTW_DNR));
    
	chdir(filename);
	print_indent(depth); printf("╚═══╤\n");

	while ((dirp = readdir(dp)) != NULL && ret == 0)
	{
		if (strcmp(dirp->d_name, ".") != 0 && strcmp(dirp->d_name, "..") != 0)
			ret = dopath(dirp->d_name, depth + 1, func);
	}
    
	print_indent(depth+1); 	printf("└\n");
	chdir("..");

	if (closedir(dp) < 0)
	{
		perror("Error: catalog is not closing");
		ret = -1;
	}

	return ret;    
}

static info_print_t print_info;
static int print_info(const char *pathame,  int type)
{
	switch(type)
	{
		case FTW_F: 
			printf( "│ %s\n", pathame);
			break;
		case FTW_D: 
			printf( "║ %s/\n", pathame);
			break;
		case FTW_DNR:
			perror("No acsess for catalog\n");
			return -1;
		case FTW_NS:
			perror("stat function error\n");
			return -1;
		default: 
			perror("unknown file type"); 
	}
	return 0;
}


int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		perror("ERROR: wrong number of arguments\n");
		return -1;
	}

	printf("%s\n", argv[0]);
	int ret = dopath(argv[1], 0, print_info);
	exit(ret);
}
