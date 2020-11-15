/// Задание 3

// Написать программу по схеме первого задания, но в процессе-предке выполнить системный вызов wait().
// Убедиться, что в этом случае индитификатор процесса-потомка на 1 больше идентификатора процесса-предка
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void)
{
	pid_t childPID1, childPID2;
	childPID1 = fork();
	
	if (childPID1 == -1)
	{
		perror("Can't fork\n");
		return 1;
	}
	else if (childPID1)
	{
		childPID2 = fork();
		if (childPID2 == -1)
		{
			perror("Can't fork\n");
			return 1;
		}
		else if (childPID2)
		{
			printf("Parent:\t\t\t\tPID=%d, PGRP=%d, CHILD1_PID=%d, CHILD2_PID=%d \n", 
			getpid(), getpgrp(), childPID1, childPID2);
			for (int i=0; i<2; i++)
			{
				int stat;
				pid_t child;
				child = wait(&stat);
				printf("%d finished, status=%d. Parent PID:%d\n", child, stat, getpid());
			}
		}
		else
		{
			printf("Child2:\t\t\t\tPID=%d, PGRP=%d, PARENT_PID=%d \n",
			getpid(), getpgrp(), getppid());
		}
	}
	else
	{
		printf("Child1:\t\t\t\tPID=%d, PGRP=%d, PARENT_PID=%d \n",
		getpid(), getpgrp(), getppid());	
	}
	
	return 0;	
}

