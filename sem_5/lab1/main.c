#include <stdio.h>
#include <unistd.h>

int main(void)
{
	int childPID = fork();
	if (childPID == -1)
	{
		perror("Can't fork\n");
		return 1;
	}
	else if (childPID == 0)
		while (1)
			printf("%d ", getpid());
	else
		while (1)
			printf("%d ", getpid());
	return 0;
}

