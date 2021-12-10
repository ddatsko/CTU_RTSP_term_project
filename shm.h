#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <semLib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>

// error codes
#define WRONG_COMPANY_NAME 2
#define ERROR_FTRUNC 3
#define ERROR_MAPPING 4
#define ERROR_OPEN_FILE 6


typedef struct {
	uint32_t irq_count;
	int32_t position;
	int32_t desired_position;
	int32_t pwm_duty;
} motor_driver_shared_memory_t;

motor_driver_shared_memory_t* init_shm();

SEM_ID sem_shm_lock;
