#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

//this struct is for shared memory
struct shared_data{
    int multiple; // this is the value of the multiple and it starts at 3
    int counter; // this is the shared counter
};

// wait/lock so P op

void sem_wait(int semid){
    struct sembuf sb;
    sb.sem_num = 0;
    sb.sem_op = -1; //here we will decrement so lock
    sb.sem_flg = 0;
    semop(semid, &sb, 1);
}

//this is signal/unlock so V op

void sem_signal(int semid){
    struct sembuf sb;
    sb.sem_num = 0;
    sb.sem_op = 1; //here it is 1 because it increment so unlock
    sb.sem_flg = 0;
    semop(semid, &sb, 1);
}

int main() {
     pid_t pid;
    int shmid, semid;
    struct shared_data *shm_ptr;
    //this is a key for shared memory
    key_t shm_key = 1234;
    //this is a key for semaphore
    key_t sem_key = 5678;
    int temp_counter;
    int temp_multiple;


    printf("Process 2 will start with exec() \n");
    printf("Process 2 PID %d \n", getpid());

    //create now a shared memory segment
    shmid = shmget(shm_key, sizeof(struct shared_data), 0666 );
    if (shmid < 0){
        perror("shmget has failed");
        exit(1);
    }

    // now we get the semaphore
    semid = semget(sem_key, 1, 0666);
    if (semid < 0){
        perror("semget has failed");
        exit(1);
    }

    //now we link to shared memory
    shm_ptr = (struct shared_data *)shmat(shmid, NULL, 0);
    if (shm_ptr == (struct shared_data *)-1){
        perror("shmat has failed");
        exit(1);
    }

    //now we wait until the counter is greater than 100
    printf("Process 2 is waiting for counter to go over 100 \n");
    while(1){
        //we lock before reading
        sem_wait(semid); 
        temp_counter = shm_ptr->counter;
        //now we unlock after reading
        sem_signal(semid); 
        if (temp_counter > 100){
            break;
        }
        sleep(1);
    }

    printf("Process 2 is now starting its execution, counter %d\n\n", temp_counter);

    //process 2 now shows the shared memory values with protection
    while(1){
        //we lock before going in the shared memory
        sem_wait(semid);

        if(shm_ptr->counter > 500){
            //now we unlock before the break
            sem_signal(semid);
            break;
        }
        temp_counter = shm_ptr -> counter;
        temp_multiple = shm_ptr-> multiple;

        printf("Process 2   PID %d    Shared Counter: %d, Multiple: %d\n", getpid(), temp_counter, temp_multiple);

        if (temp_counter % temp_multiple == 0){
            printf("Process 2 = PID %d is a multiple of %d\n", getpid(), temp_counter, temp_multiple);
        } 

        //now we unlock after going in the shared memory
        sem_signal(semid);

        sleep(1);
    }

    printf("Process 2 is finishing. Shared counter > 500\n");

    //now we unlink from the shared memory
    shmdt(shm_ptr);

    return 0;

}