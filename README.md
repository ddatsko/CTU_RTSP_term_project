# CTU_RTSP_term_project
## TODO: add description here

### Tasks structure 
![Structure drawio](https://user-images.githubusercontent.com/35429810/145201530-005b390d-ce5f-4674-87a6-57b0b9ad7077.png)

## UDP communication package structure
| Type | Bytes | Description |
| ---- | ----- | ----------- | 
| uint32 | 4 | Motor desired absolute position |

## Tasks workflow:
### Driver
#### If in listener mode
* When interrupt comes - update the counter and store it to the shared memory
#### If in writer mode
##### in the interrupt handler
* In the interupt, give the semaphore (maybe only if the movement direction and desired movement direction are different)
##### In the main loop
* wait fot the semaphore to be released
* check for the desired value and real motor position
* manage pwm signals to make motor be moved to the proper direction or to be stopped

### UDP communication
#### On reader board loop
* Wait for some time (e.g. 20 ms)
* Read the current motor absolute position from the shared memory
* Send the motor position through sockets to another device
#### On motor controller board loop
* Wait for the packet from the second board
* Write the updated data to the shared memory
* Give the semaphore to make the driver "see" the changes

 ### Web server
 ## TODO: Document this too
