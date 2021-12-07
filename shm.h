#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <semLib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

// error codes
#define WRONG_COMPANY_NAME 2
#define ERROR_FTRUNC 3
#define ERROR_MAPPING 4
#define ERROR_OPEN_FILE 6

SEM_ID sem_shm_lock;
