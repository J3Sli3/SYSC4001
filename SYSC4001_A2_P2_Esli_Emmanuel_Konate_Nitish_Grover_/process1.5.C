#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>

//this struct is for shared memory
struct shared_data{
    int multiple; // this is the value of the multiple and it starts at 3
    int counter; // this is the shared counter
};

// this is a union for semaphore op
union semaphoreunion {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
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
    union semaphoreunion sem_arg;
    int status;
    
    printf("Process 1 PID %d it starts now \n", getpid());

    //now we create shared memory segment
    shmid = shmget(shm_key, sizeof(struct shared_data), IPC_CREAT | 0666);
    if (shmid < 0){
        perror("shmget has failed");
        exit(1);
    }

    //now we create semaphore
    semid = semget(sem_key, 1, IPC_CREAT | 0666);
    if (semid < 0){
        perror("semget has failed");
        exit(1);
    }

    //now we start semphare as 1 (unlocked)
    sem_arg.val = 1;
    if (semctl(semid, 0, SETVAL, sem_arg) < 0){
        perror("semctl failed");
        exit(1);
    }

    //now we link to shared memory
    shm_ptr = (struct shared_data *)shmat(shmid, NULL, 0);
    if (shm_ptr == (struct shared_data *)-1){
        perror("shmat has failed");
        exit(1);
    }

    // now we start the shared variables but protected
    sem_wait(semid);
    //here it can be changed to any value
    shm_ptr->multiple = 3;
    shm_ptr->counter = 0;
    sem_signal(semid);

    pid = fork();

    if (pid < 0){
        perror("The Fork has failed");
        exit(1);
    } else if (pid == 0){
        //now this is the child process
        printf("Child PID: %d will exec process2 \n", getpid());
        sleep(1);

        execl("./process2_part5", "process2_part5", NULL);

        perror("exec has failed");
        exit(1);
    } else {
        //now this is the parent process
        printf("Parent PID %d, Child PID: %d \n\n", getpid(), pid);

        //the parent keeps on incrementing the counter with the protected semaphore
        while(1){
            //now we lock before going in the shared memory
            sem_wait(semid);

            if (shm_ptr->counter > 500){
                //now we unlock before the break
                sem_signal(semid);
                break;
            }

            if (shm_ptr->counter % shm_ptr->multiple == 0){
                printf("Process 1 = PID %d Cycle: %d   %d is a multiple of %d\n", getpid(), shm_ptr->counter, shm_ptr->counter, shm_ptr->multiple);
            } else {
                printf("Process 1   PID: %d  Cycle: %d\n", getpid(), shm_ptr->counter);
            }
            shm_ptr->counter++;
            //now we unlock before going in the shared memory
            sem_signal(semid);
            sleep(1);
        }

        //wait for child to finish
        wait(&status);

        //now we clean up
        shmdt(shm_ptr);
        shmctl(shmid, IPC_RMID, NULL);
        semctl(semid, 0, IPC_RMID);

        printf("Process 1 is finishing. Shared memory and semaphore is now cleaned up.\n");
    }
    return 0;
}