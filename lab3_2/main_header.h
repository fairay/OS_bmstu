#ifndef _MAIN_H
#define _MAIN_H

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

#define ACT_W_SEMN   0
#define ACT_R_SEMN   1
#define WAI_W_SEMN   2
#define WAI_R_SEMN   3


#define WRITER_N    2
#define READER_N    5

#endif // _MAIN_H