/// Задание 5

// В программу с программным каналом включить собственный обработчик сигнала. Использовать сигнал для 
// изменения хода выполнения программы.

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MSG_SIZE	21
typedef void (*sighandler_t)(int);

int fd2[2];
int send_flag = 0;

void send_msg(int sig_n)
{
	send_flag = 1;
	if (getpid() == getpgrp()) 
	{
		write(fd2[1], "Parent signal msg", MSG_SIZE);
		printf("Parent message sent\n");	
	}
}
int get_msg()
{
	char res[MSG_SIZE];
	ssize_t code = read(fd2[0], res, MSG_SIZE);
	if (code == -1)
		printf("Read error\n");
	else
		printf("Child 1 got parent message: %s\n", res);
	return code == -1;
}

int main(void)
{
	sighandler_t sg = signal(SIGINT, send_msg);
	if (sg == SIG_ERR)
    {
        perror("Can\'t create signal\n");
		return -1;
    }
	
	int fd[2];
	pid_t childPID1, childPID2;
	
	int pipe_code = pipe(fd);
    if (pipe_code < 0)
    {
        perror("Can\'t create pipe\n");
		return -1;
    }
    pipe_code = pipe(fd2);
    if (pipe_code < 0)
    {
        perror("Can\'t create pipe\n");
		return -1;
    }
    
    
   	childPID1 = fork();
	if (childPID1 == -1)
	{
		perror("Can't fork\n");
		return -1;
	}
	else if (!childPID1)
	{
		printf("Child 1:\t\t\tPID=%d, PGRP=%d, PARENT_PID=%d \n", getpid(), getpgrp(), getppid());

		ssize_t code = write(fd[1], "Data from child №1", MSG_SIZE);
		if (code == -1)
		{
			printf("Write error\n");
			return -1;
		}
		else
			printf("Child 1 sent message\n");

		sleep(4);
		
		if (send_flag) 
			if (get_msg())	return -1;
		else
			printf("\nChild 1 did not get message\n");
		
		if (close(fd[1]) || close(fd[0]) || close(fd2[0]) || close(fd2[1]))
		{
			printf("Close error\n");
			return -1;
		}
		else
			return 0;
	}
	
	
   	childPID2 = fork();
	if (childPID2 == 0)
	{		
		printf("Child 2:\t\t\tPID=%d, PGRP=%d, PARENT_PID=%d \n", getpid(), getpgrp(), getppid());
		
		ssize_t code = write(fd[1], "Data from child №2", MSG_SIZE);
		if (code == -1)
		{
			printf("Write error\n");
			return -1;
		}
		else
			printf("Child 2 sent message\n");
	
		if (close(fd[1]) || close(fd[0]) || close(fd2[0]) || close(fd2[1]))
		{
			printf("Close error\n");
			return -1;
		}
		else
			return 0;
			
	}
	else if (childPID2 == -1)
	{
		perror("Can't fork\n");
		return -1;
	}
	
	printf("Parent:\t\t\t\tPID=%d, PGRP=%d, CHILD1_PID=%d, CHILD2_PID=%d \n", getpid(), getpgrp(), childPID1, childPID2);

	printf("Parent is waiting\n");
	int stat, child;
	for (int i=0; i<2; i++)
	{
		child = wait(&stat);
		if (child == -1)
		{
			printf("Wait returned with error\n");
			return -1;
		}
		else
		{
			printf("%d finished, status=%d. Parent PID:%d\t\t\t", child, stat, getpid());
			if (WIFEXITED(stat))
				printf("Child exited with code %d\n", WEXITSTATUS(stat));
			else 
				printf("Child terminated abnormally\n");
		}
	}
	printf("All child process are done\n");
	
	char res1[MSG_SIZE], res2[MSG_SIZE];
	ssize_t code1 = read(fd[0], res1, MSG_SIZE);
	ssize_t code2 = read(fd[0], res2, MSG_SIZE);
	if (code1 == -1 || code1 == -1)
	{
		printf("Read error\n");
		return -1;
	}
	printf("First message: %s\n", res1);
	printf("Second message: %s\n", res2);
	
	if (close(fd[1]) || close(fd[0]) || close(fd2[0]) || close(fd2[1]))
	{
		printf("Close error\n");
		return -1;
	}
	else
		return 0;

}

