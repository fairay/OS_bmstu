#include "main_header.h"

int rand_dt()
{
    return 1 + rand() % 3;
}

void prod_func(size_t b_size, int isem_descry, char* buf, int my_n)
{
    srand(time(NULL) + (my_n+1)*100);
    struct sembuf pre_sem[2] = { {EMPT_SEMN, -1, SEM_UNDO},
                                 {BIN_SEMN, -1, SEM_UNDO} };
    struct sembuf post_sem[2] = { {FULL_SEMN, 1, SEM_UNDO},
                                  {BIN_SEMN, 1, SEM_UNDO} };

    sleep(rand_dt());
    while (1)
    {
        if (semop(isem_descry, pre_sem, 2) == -1)
        {
            perror("semop");
            exit(1);
        }

        buf[buf[2]] = buf[0];
        printf("Producer №%d wrote:\t %c\n", my_n+1, buf[0]);
        if (++buf[2] >= b_size)   
            buf[2] = 3;
        if (++buf[0] > 'Z')
            buf[0] = 'A';

        if (semop(isem_descry, post_sem, 2) == -1)
        {
            perror("semop");
            exit(1);
        }

        sleep(rand_dt());
    }
    
}

void cons_func(size_t b_size, int isem_descry, char* buf, int my_n)
{
    srand(time(NULL) - (my_n+1)*100);
    struct sembuf pre_sem[2] = { {FULL_SEMN, -1, SEM_UNDO},
                                 {BIN_SEMN, -1, SEM_UNDO} };
    struct sembuf post_sem[2] = { {EMPT_SEMN, 1, SEM_UNDO},
                                  {BIN_SEMN, 1, SEM_UNDO} };
    char cur_letter;
    sleep(rand_dt());
    while (1)
    {
        if (semop(isem_descry, pre_sem, 2) == -1)
        {
            perror("semop");
            exit(1);
        }

        cur_letter = buf[buf[1]];
        printf("Consumer №%d read:\t\t %c\n", my_n+1, cur_letter);
        if (++buf[1] >= b_size)   
            buf[1] = 3;

        if (semop(isem_descry, post_sem, 2) == -1)
        {
            perror("semop");
            exit(1);
        }

        sleep(rand_dt());
    }
    
}


int main(void)
{
    int perms = S_IRWXU | S_IRWXO | S_IRWXG;

    int isem_descry = semget(IPC_PRIVATE, 3, IPC_CREAT | perms );
    if (isem_descry == -1) 
    { 
        perror("semget"); 
        return 1;
    }
    int full_ctl = semctl(isem_descry, FULL_SEMN, SETVAL, 0);
    int empt_ctl = semctl(isem_descry, EMPT_SEMN, SETVAL, QUEUE_SIZE);
    int bin_ctl = semctl(isem_descry, BIN_SEMN, SETVAL, 1);
    if (full_ctl == -1 || empt_ctl == -1 || bin_ctl == -1)
    {
        perror("semctl");
        return 1;
    }

    int mem_id = shmget(IPC_PRIVATE, (BUF_SIZE)*sizeof(char), IPC_CREAT | perms);
    if (mem_id == -1)
    {
        perror("shmget");
        return 1;
    }   
    char* addr = shmat(mem_id, 0, 0);
    if (addr == (char*)(-1))
    {
        perror("shmat");
        return 1;
    }

    addr[0] = 'A';
    addr[1] = (char)3;
    addr[2] = (char)3;

    printf("> Start of simulation\n");
    for (size_t i=0; i<PROC_N; i++)
    {
        pid_t prod_pid = fork();
        switch (prod_pid)
        {
        case -1:
            perror("fork");
            return 1;
        case 0:
            prod_func(BUF_SIZE, isem_descry, addr, i);
            return 0;
        default:
            printf("> Producer created\n");
            break;
        }

        pid_t cons_pid = fork();
        switch (cons_pid)
        {
        case -1:
            perror("fork");
            return 1;
        case 0:
            cons_func(BUF_SIZE, isem_descry, addr, i);
            return 0;
        default:
            printf("> Consumer created\n");
            break;
        }
    }

    int status, pid;
    for (size_t i=0; i<PROC_N*2; i++)
    {
        pid = wait(&status);
        if (pid == -1)
        {
            perror("wait");
            return 1;
        }
    }


    if (semctl(isem_descry, 0, IPC_RMID, 0) == -1)
    {
        perror("semctl");
        return 1;
    }
    if (shmctl(mem_id, IPC_RMID, NULL) == -1)
    {
        perror("shmctl");
        return 1;
    }
    if (shmdt(addr) == -1)
    {
        perror("shmdt");
        return 1;
    }

    return 0;
}