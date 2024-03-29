/// Задание 3

// Потомок вызывает exec(), предок ждёт его завершения. (>= 2 потомка)

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
				if (child == -1)
				{
					printf("Wait returned with error\n");
					return 1;
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
		}
		else
		{
			printf("Child2:\t\t\t\tPID=%d, PGRP=%d, PARENT_PID=%d \n",
			getpid(), getpgrp(), getppid());
			int stat = execl("demo2.o", " ", NULL);
			if (stat == -1)
			{
				printf("Execl error\n");
				return 1;
			}
		}
	}
	else
	{
		printf("Child1:\t\t\t\tPID=%d, PGRP=%d, PARENT_PID=%d \n",
		getpid(), getpgrp(), getppid());
		int stat = execl("demo1.o", " ", NULL);
		if (stat == -1)
		{
			printf("Execl error\n");
			return 1;
		}
	}
	
	return 0;	
}

