#include "shm.h"


int init_shm(int is_srv) {
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
    if (ftruncate (fd, sizeof(struct company_registry)) == -1) {
		perror("ftruncate");
		semGive(sem_shm_lock);
		exit (ERROR_FTRUNC);
	}
    
    /* Map shared memory object in the process address space */
    ptr = (struct company_registry *)mmap(0, sizeof(struct company_registry),
                          	  	  	  	  	  PROT_READ | PROT_WRITE,
                          	  	  	  	  	  MAP_SHARED, fd, 0);
    if (ptr == (struct company_registry *)MAP_FAILED) {
    	semGive(sem_shm_lock);
    	exit (ERROR_MAPPING);
    }
        
    
    /* close the file descriptor; the mapping is not impacted by this */
    close (fd);
    
   
    /* the fist company should zero the memory this way: */
    if (!is_init) {
    	memset(ptr, 0, sizeof(struct company_registry));
    	for (int i = 0; i < 50; ++i) {
    		(&ptr->companies[i])->is_empty = 1;
    	}
    	
    }
    semGive(sem_shm_lock);
    
    if (is_srv) {
    	
    }
	return 0;
}
