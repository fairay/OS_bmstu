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
		printf("----");
}

static int dopath(const char *filename, int depth, info_print_t *func)
{
	int ret = 0;

	print_indent(depth);

	// структура с информацией о файле
	// тип, режим доступа, номер индексного узла...
	struct stat statbuf; 
	// lstat - заполняет statbuf информацией о файле filename
	if (lstat(filename, &statbuf) < 0) 
		return(func(filename, FTW_NS)); // в случае неудачи - вывод сообщения и выход

	// проверка является ли файл каталогом с помощью макроса
	// st_mode - тип файла + права доступа
	if (S_ISDIR(statbuf.st_mode) == 0) 
		return(func(filename, FTW_F)); // не каталог => печать имени файла, выход

	func(filename, FTW_D);

	DIR *dp; // структура с информацией о каталоге
	dp = opendir(filename);		// открытие потока каталога
	if (!dp)
		return(func(filename, FTW_DNR));
    
	chdir(filename); 	// измененеие текущего рабочего каталога на filename
	print_indent(depth); printf("╘═══╤\n");

	struct dirent* dirp; // хранит № индексного узла + строка имени файла
	// readdir - возвращает указатель на очередную запись 
	// из структуры dp (поток каталога) или NULL по прочтению всех записей
	while ((dirp = readdir(dp)) && ret == 0)
	{
		// файлы с именем "." ".." не обрабатываются
		if (strcmp(dirp->d_name, ".") != 0 && strcmp(dirp->d_name, "..") != 0)
			ret = dopath(dirp->d_name, depth + 1, func);
			// запуск dopath для очередного файла из рассматриваемого каталога
	}

	print_indent(depth+1);	printf("└\n");
	chdir("..");		// измененеие текущего рабочего каталога на родительский

	if (closedir(dp) < 0)		// закрытие потока каталога
		perror("Error: catalog is not closing");

	return(ret);    
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
			printf( "║ %s\n", pathame);
			break;
		case FTW_DNR:
			perror("No acsess for catalog\n");
		case FTW_NS:
			perror("stat function error\n");
		default: 
			perror("unknown file type"); 
	}
	return 0;
}


int main(int argc, char* argv[])
{
	if (argc != 2)
		perror("ERROR: wrong number of arguments\n");

	printf("%s\n", argv[0]);
	int ret = dopath(argv[1], 0, print_info);
	exit(ret);
}
