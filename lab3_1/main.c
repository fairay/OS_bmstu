#include "main_header.h"

struct sembuf sem_arr[2] = {{0, 1, SEM_UNDO},
                            {1, 1, SEM_UNDO}};

int main(void)
{
    int perms = S_IRWXU | S_IRWXO | S_IRWXG; //S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    int isem_descry = semget(IPC_PRIVATE, 3, IPC_CREAT | perms );

    if (isem_descry == -1) 
    { 
        perror("semget"); 
        return 1;
    }

    int full_ctl = semctl(isem_descry, FULL_SEMN, SETVAL, 0);
    int empt_ctl = semctl(isem_descry, EMPT_SEMN, SETVAL, BUF_SIZE);
    int bin_ctl = semctl(isem_descry, BIN_SEMN, SETVAL, 1);

    if (full_ctl == -1 || empt_ctl == -1 || bin_ctl == -1)
    {
        perror("semctl");
        return 1;
    }

    if ( semop(isem_descry, sem_arr, 2) == -1) 
    { 
        perror("semop");
        return 1;
    }
    return 0;
}