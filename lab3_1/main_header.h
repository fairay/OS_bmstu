#ifndef MAIN_H
#define MAIN_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define FULL_SEMN   0
#define EMPT_SEMN   1
#define BIN_SEMN    2

#define QUEUE_SIZE  5
#define BUF_SIZE    2 + QUEUE_SIZE
#define PROC_N      3

#endif // MAIN_H