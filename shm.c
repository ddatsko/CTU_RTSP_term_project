#include "shm.h"


/*!
 * \brief Initialize the shared memory
 * \note Function using exit if the memory was not intiialized successfully
 * 
 * @return Pointer to the shared struct, NULL pointer in case of failure
 */
motor_driver_shared_memory_t* init_shm() {
    int fd;

    sem_shm_lock = semOpen("/complock", SEM_TYPE_MUTEX, SEM_EMPTY, SEM_Q_FIFO, OM_CREATE, NULL);

    int is_init=0;
    semTake(sem_shm_lock, WAIT_FOREVER);
 
    fd = shm_open("/counter", O_RDWR | O_CREAT | O_EXCL, S_IRUSR|S_IWUSR);
    
    /* Check if smbd already opened this */
    
    if (fd == -1){
    	if (errno != EEXIST) {
    		semGive(sem_shm_lock);
    		exit(ERROR_OPEN_FILE);
    	}
    	// Not first open
    	fd = shm_open("/counter", O_RDWR | O_CREAT, S_IRUSR|S_IWUSR);
		if (fd == -1) {
			semGive(sem_shm_lock);
			exit(ERROR_OPEN_FILE);
		}
		is_init = 1;
    }
    if (ftruncate (fd, sizeof(motor_driver_shared_memory_t)) == -1) {
		perror("ftruncate");
		semGive(sem_shm_lock);
		exit (ERROR_FTRUNC);
	}
    
    /* Map shared memory object in the process address space */
    motor_driver_shared_memory_t* dest = (motor_driver_shared_memory_t *)mmap(0, sizeof(motor_driver_shared_memory_t),
                          	  	  	  	  	  PROT_READ | PROT_WRITE,
                          	  	  	  	  	  MAP_SHARED, fd, 0);
    if (dest == (motor_driver_shared_memory_t *)MAP_FAILED) {
    	semGive(sem_shm_lock);
    	exit (ERROR_MAPPING);
    }
        
    /* close the file descriptor; the mapping is not impacted by this */
    close (fd);
    
   
    /* the first to open the structure initializes it */
    if (!is_init) {
    	dest->irq_count = 0;
    	dest->desired_position = 0;
    	dest->position = 0;
    	dest->pwm_duty = 0;    	
    }
    semGive(sem_shm_lock);

	return 0;
}
