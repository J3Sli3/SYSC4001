#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

//this struct is for shared memory
struct shared_data{
    int multiple; // this is the value of the multiple and it starts at 3
    int counter; // this is the shared counter
};

int main() {
    pid_t pid;
    int shmid;
    struct shared_data *shm_ptr;
    // this will be the key for shared memory
    key_t key = 1234; 
    int status;

    printf("Process 1 PID %d, starts now\n", getpid());

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

    //now we start the shared variables
    //multiple can be changed to any value
    shm_ptr->multiple = 3;
    shm_ptr->counter = 0;

    pid = fork();

    if (pid < 0){
        perror("Fork has failed");
        exit(1); 
    } else if (pid == 0){
        //this is the child process
        printf("Child PID: %d will now exec process2 \n", getpid());
        sleep(1);

        execl("./process2_part4", "process2_part4", NULL);

        perror("exec has failed");
        exit(1);
    } else {
        //this is the parent process
        printf("Parent PID %d, Child PID %d \n\n", getpid(), pid);

        //now the parent will keep on incrementing counter
        while (shm_ptr->counter <= 500){
            if (shm_ptr->counter % shm_ptr->multiple == 0){
                            printf("Process 1 = PID %d  Cycle: %d   %d is a multiple of %d\n", getpid(), shm_ptr->counter, shm_ptr->counter, shm_ptr->multiple);
            } else {
                printf("Process    PID: %d   Cycle: %d\n", getpid(), shm_ptr->counter);
            }
            shm_ptr->counter++;
            sleep(1);
        }
    

        //now wait for the child to finish
        wait(&status);

        //now we remove the shared memory
        shmdt(shm_ptr);
        shmctl(shmid, IPC_RMID, NULL);

        printf("Process 1 is finishing. Now we clean the shared memory. \n");

    }

    return 0;
    
}