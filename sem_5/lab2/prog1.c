/// Задание 1

// Написать программу, запускающую не мене двух новых процессов
// системным вызовом fork(). В предке вывести собственный идентификатор
// (функция getpid()), идентификатор группы ( функция getpgrp()) и
// идентификаторы потомков. В процессе-потомке вывести собственный
// идентификатор, идентификатор предка (функция getppid()) и идентификатор
// группы. Убедиться, что при завершении процесса-предка потомок, который
// продолжает выполняться, получает идентификатор предка (PPID), равный 1 или
// идентификатор процесса-посредника.

#include <stdio.h>
#include <unistd.h>

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
		}
		else
		{
			pid_t PPID = getppid();
			printf("Child2:\t\t\t\tPID=%d, PGRP=%d, PARENT_PID=%d \n",
			getpid(), getpgrp(), getppid());
			
			while(PPID == getppid()) {}
			
			printf("Child2 after parent exit:\tPID=%d, PGRP=%d, PARENT_PID=%d \n",
			getpid(), getpgrp(), getppid());
		}
	}
	else
	{
		pid_t PPID = getppid();
		printf("Child1:\t\t\t\tPID=%d, PGRP=%d, PARENT_PID=%d \n",
		getpid(), getpgrp(), getppid());
		
		while(PPID == getppid()) {}
		
		printf("Child1 after parent exit:\tPID=%d, PGRP=%d, PARENT_PID=%d \n",
		getpid(), getpgrp(), getppid());
	}
	
	return 0;	
}
