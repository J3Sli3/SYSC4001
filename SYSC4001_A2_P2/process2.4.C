#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

//this struct is for shared memory
struct shared_data{
    int multiple; // this is the value of the multiple and it starts at 3
    int counter; // this is the shared counter
};

int main() {
    int shmid;
    struct shared_data *shm_ptr;
    // this will be the key for shared memory
    key_t key = 1234; 
    int local_counter;


    printf("Process 2 will start with exec() \n");
    printf("Process 2 PID %d \n", getpid());

    //create now a shared memory segment
    shmid = shmget(key, sizeof(struct shared_data), IPC_CREAT | 0666);
    if (shmid < 0){
        perror("shmget has failed");
        exit(1);
    }

    //now we attach shared memory
    shm_ptr = (struct shared_data *)shmat(shmid, NULL, 0);
    if (shm_ptr == (struct shared_data *)-1){
        perror("shmat has failed");
        exit(1);
    }

    //now wait until counter is greater than 100
    printf("Process 2 is waiting for counter to surpass 100 \n");
    while(shm_ptr->counter <= 100){
        sleep(1);
    }

    printf("Process 2 starts executing, counter %d\n\n", shm_ptr->counter);

    //process 2 now reads from a shared counter and shows
    local_counter = shm_ptr->counter;

    while(shm_ptr->counter <= 500){
        printf("Process 2  PID: %d shared counter: %d, Multiple: %d\n", getpid(), shm_ptr->counter, shm_ptr->multiple);

        if (shm_ptr->counter % shm_ptr->multiple == 0){
            printf("Process 2 = PID %d   %d is a multiple of %d\n", getpid(), shm_ptr->counter, shm_ptr->multiple);
        }
        
        sleep(1);
    }

    printf("Process 2 is finishing. Shared counter > 500\n");

    //now we remove the shared memory
    shmdt(shm_ptr);

    return 0;

}