#include "main_header.h"

int rand_dt()
{
    return 1 + rand() % 2;
}

void start_read(int isem_descry)
{
    static struct sembuf dir_sem[3] = {
        {ACT_W_SEMN, 0, SEM_UNDO | IPC_NOWAIT},
        {WAI_W_SEMN, 0, SEM_UNDO | IPC_NOWAIT},
        {ACT_R_SEMN, 1, SEM_UNDO}};
    static struct sembuf wait_sem[1] = {
        {WAI_R_SEMN, 1, SEM_UNDO} };
    static struct sembuf awake_sem[4] = {
        {ACT_W_SEMN, 0, SEM_UNDO},
        {CAN_R_SEMN,-1, SEM_UNDO},
        {ACT_R_SEMN, 1, SEM_UNDO},
        {WAI_R_SEMN,-1, SEM_UNDO} };
    
    if (semop(isem_descry, dir_sem, 3) == -1)
    {
        if (semop(isem_descry, wait_sem, 1) == -1)
        {
            perror("semop");
            exit(1);
        }
        if (semctl(isem_descry, CAN_R_SEMN, SETVAL, 0) == -1)
        {
            perror("semctl");
            exit(1);
        }
        if (semop(isem_descry, awake_sem, 4) == -1)
        {
            perror("semop");
            exit(1);
        }
        if (semctl(isem_descry, CAN_R_SEMN, SETVAL, 1) == -1)
        {
            perror("semctl");
            exit(1);
        }
    }

    // if (semctl(isem_descry, CAN_R_SEMN, SETVAL, 0) == -1)
    //     {
    //         perror("semctl");
    //         exit(1);
    //     }
}
void stop_read(int isem_descry)
{
    static struct sembuf act_sem[1] = {
        {ACT_R_SEMN,-1, SEM_UNDO}};

    if (semop(isem_descry, act_sem, 1) == -1)
    {
        perror("semop");
        exit(1);
    }
    
    int active_read_n = semctl(isem_descry, ACT_R_SEMN, GETVAL, 0);
    if (active_read_n == -1)
    {
        perror("semctl");
        exit(1);
    }
    else if (active_read_n == 0)
        if (semctl(isem_descry, CAN_W_SEMN, SETVAL, 1) == -1)
        {
            perror("semctl");
            exit(1);
        }
}
void start_write(int isem_descry)
{
    static struct sembuf dir_sem[3] = {
        {ACT_R_SEMN, 0, SEM_UNDO | IPC_NOWAIT},
        {ACT_W_SEMN, 0, SEM_UNDO | IPC_NOWAIT},
        {ACT_W_SEMN, 1, SEM_UNDO}};
    static struct sembuf wait_sem[1] = {
        {WAI_W_SEMN, 1, SEM_UNDO} };
    static struct sembuf awake_sem[4] = {
        {ACT_W_SEMN, 0, SEM_UNDO},
        {CAN_W_SEMN,-1, SEM_UNDO},
        {ACT_W_SEMN, 1, SEM_UNDO},
        {WAI_W_SEMN,-1, SEM_UNDO} };
    
    if (semop(isem_descry, dir_sem, 3) == -1)
    {
        if (semop(isem_descry, wait_sem, 1) == -1)
        {
            perror("semop");
            exit(1);
        }
        if (semctl(isem_descry, CAN_W_SEMN, SETVAL, 0) == -1)
        {
            perror("semctl");
            exit(1);
        }
        if (semop(isem_descry, awake_sem, 4) == -1)
        {
            perror("semop");
            exit(1);
        }
    }
}
void stop_write(int isem_descry)
{
    static struct sembuf act_sem[1] = {
        {ACT_W_SEMN,-1, SEM_UNDO}};
            
    int active_read_n = semctl(isem_descry, WAI_R_SEMN, GETVAL, 0);
    if (active_read_n == -1)
    {
        perror("semctl");
        exit(1);
    }
    else if (active_read_n)
    {
        if (semctl(isem_descry, CAN_R_SEMN, SETVAL, 1) == -1)
        {
            perror("semctl");
            exit(1);
        }
    }
    else
    {
        if (semctl(isem_descry, CAN_W_SEMN, SETVAL, 1) == -1)
        {
            perror("semctl");
            exit(1);
        }
    }

    if (semop(isem_descry, act_sem, 1) == -1)
    {
        perror("semop");
        exit(1);
    }
}

void read_func(int isem_descry, char* buf, int my_n)
{
    srand(time(NULL) + my_n*100);
    while (1)
    {
        start_read(isem_descry);

        printf("Reader №%d get:\t\t %c\n", my_n, *buf);

        stop_read(isem_descry);
        sleep(rand_dt());
    }
}

void write_func(int isem_descry, char* buf, int my_n)
{
    srand(time(NULL) + my_n*10);
    while (1)
    {
        start_write(isem_descry);

        *buf = (char)(rand() % ('z' - 'a') + 'a');
        printf("Writer №%d send:\t %c\n", my_n, *buf);

        stop_write(isem_descry);
        sleep(rand_dt());
    }
}

int main(void)
{
    int perms = S_IRWXU | S_IRWXO | S_IRWXG;

    int isem_descry = semget(IPC_PRIVATE, SEMN, IPC_CREAT | perms );
    if (isem_descry == -1) 
    { 
        perror("semget"); 
        return 1;
    }
    for (size_t i=0; i<SEMN; i++)
        if (semctl(isem_descry, i, SETVAL, 0) == -1)
        {
            perror("semctl");
            return 1;
        }


    int mem_id = shmget(IPC_PRIVATE, sizeof(char), IPC_CREAT | perms);
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
    *addr = '!';

    printf("> Start of simulation\n");
    for (size_t i=0; i<WRITER_N; i++)
    {
        pid_t prod_pid = fork();
        switch (prod_pid)
        {
        case -1:
            perror("fork");
            return 1;
        case 0:
            write_func(isem_descry, addr, i);
            return 0;
        default:
            printf("> Writer created\n");
            break;
        }
    }
    for (size_t i=0; i<READER_N; i++)
    {
        pid_t prod_pid = fork();
        switch (prod_pid)
        {
        case -1:
            perror("fork");
            return 1;
        case 0:
            read_func(isem_descry, addr, i);
            return 0;
        default:
            printf("> Reader created\n");
            break;
        }
    }
    

    int status, pid;
    for (size_t i=0; i<WRITER_N+READER_N; i++)
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