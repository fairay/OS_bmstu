/// Задание 5

// В программу с программным каналом включить собственный обработчик сигнала. Использовать сигнал для 
// изменения хода выполнения программы.

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MSG_SIZE	21

int fd2[2];
void send_msg(int sig_n)
{
	write(fd2[1], "Parent signal msg", MSG_SIZE);
	printf("Parent message sent\n");
	kill(0, SIGUSR1);
}
void get_msg(int sig_n)
{
	char res[MSG_SIZE];
	read(fd2[0], res, MSG_SIZE);
	printf("Child 1 got parent message: %s\n", res);
}

int main(void)
{
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
		close(fd[0]);
		close(fd2[1]);	

		printf("Child 1:\t\t\tPID=%d, PGRP=%d, PARENT_PID=%d \n", getpid(), getpgrp(), getppid());
		write(fd[1], "Data from child №1", MSG_SIZE);
		printf("Child 1 sent message\n");
		
		signal(SIGUSR1, get_msg);
		signal(SIGINT, SIG_IGN);
		sleep(3);
		
		close(fd[1]);
		close(fd[0]);
		return 0;
	}
	
	
   	childPID2 = fork();
	if (childPID2 == 0)
	{
		close(fd[0]);
		close(fd2[0]);	
		close(fd2[1]);	
		
		printf("Child 2:\t\t\tPID=%d, PGRP=%d, PARENT_PID=%d \n", getpid(), getpgrp(), getppid());
		write(fd[1], "Data from child №2", MSG_SIZE);
		printf("Child 2 sent message\n");
		
		close(fd[1]);
		return 0;
	}
	else if (childPID2 == -1)
	{
		perror("Can't fork\n");
		return -1;
	}
	
	close(fd[1]);
	close(fd2[0]);
	
	printf("Parent:\t\t\t\tPID=%d, PGRP=%d, CHILD1_PID=%d, CHILD2_PID=%d \n", getpid(), getpgrp(), childPID1, childPID2);
	
	signal(SIGUSR1, SIG_IGN);
	signal(SIGALRM, send_msg);
 	alarm(2);

	printf("Parent is waiting\n");
	int status, pid;
	for (int i=0; i<2; i++)
		pid = wait(&status);
	printf("All child process are done\n");
	
	char res1[MSG_SIZE], res2[MSG_SIZE];
	read(fd[0], res1, MSG_SIZE);
	printf("First message: %s\n", res1);
	read(fd[0], res2, MSG_SIZE);
	printf("Second message: %s\n", res2);
	
	close(fd[0]);
	close(fd2[1]);
	return 0;	
}

