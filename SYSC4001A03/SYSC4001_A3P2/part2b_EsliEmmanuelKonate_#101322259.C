/**
 * This is the TA Marking system with the semaphores
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <time.h>

#define SHM_KEY 1234
#define SEM_KEY 5678
#define MAX_Q 5

//Semaphores
#define SEM_RUBRIC 0
#define SEM_EXAM 1
#define SEM_QUEST 2 


typedef struct {
    int rubric_num[MAX_Q];
    char rubric_ans[MAX_Q];
    int student_num, q_status[MAX_Q];
    int exam_idx, terminate;
} SharedData;

int semid;

void sem_wait_op(int n){
    struct sembuf op = {n, -1, 0};
    semop(semid, &op, 1);
}

void sem_signal_op(int n){
    struct sembuf op = {n, 1, 0};
    semop(semid, &op, 1);
}

void delay(float min, float max){
    usleep((int) ((min + ((float)rand()/RAND_MAX)*(max-min)) * 1000000));
}


int main(int argc, char *argv[]){
    if (argc != 2){
        printf("Usage: %s <num_TAs>\n", argv[0]);
        exit(1);
    }
    int num_tas = atoi(argv[1]);
    int i;

    int shmid = shmget(SHM_KEY, sizeof(SharedData), IPC_CREAT|0666);
    SharedData *shm = (SharedData *) shmat(shmid, NULL, 0);
    memset(shm, 0, sizeof(SharedData));

    //create the semaphores
    semid = semget(SEM_KEY, 3, IPC_CREAT|0666);
    for (i = 0; i < 3; i++){
        semctl(semid, i, SETVAL, 1);
    }

    // now we can load the rubric
    FILE *fp = fopen("rubric.txt", "r");
    for (i = 0; i < MAX_Q; i++) fscanf(fp, "%d, %c", &shm->rubric_num[i], &shm->rubric_ans[i]);
    fclose(fp);

    shm->student_num = 1001;
    shm->exam_idx = 0;
    

    printf("[Main] Starting %d TAs\n", num_tas);

    for (i = 0; i < num_tas; i++){
        if (fork() == 0){
            srand(time(NULL) ^ getpid());
            int ta = i + 1;

            while (!shm->terminate){
                int stu = shm->student_num;
                if (stu == 9999){
                    shm->terminate = 1;
                    break;
                }

                // now we review rubric with the mutex

                printf("[TA%d] Reviewing rubric for %04d\n", ta, stu);
                for (int q = 0; q < MAX_Q; q++){
                    delay(0.5, 1.0);
                    if (rand() % 5 == 0){
                        sem_wait_op(SEM_RUBRIC);
                        char old = shm -> rubric_ans[q];
                        shm->rubric_ans[q] = old + 1;
                        printf("[TA%d] has Corrected Q%d: %c->%c\n", ta, q+1, old, shm->rubric_ans[q]);
                        sem_signal_op(SEM_RUBRIC);
                    }
                }
                //mark the questions with the mutex
                while(!shm->terminate){
                    int found = -1;
                    sem_wait_op(SEM_QUEST);
                    for (int q = 0; q< MAX_Q; q++){
                        if (shm->q_status[q] == 0){
                            shm->q_status[q] = 1;
                            found = q;
                            break;
                        }
                    }
                    sem_signal_op(SEM_QUEST);
                    if (found == -1){
                        int done = 1;
                        for (int q = 0; q < MAX_Q; q++){
                            if (shm->q_status[q] != 2){
                            done = 0;
                            }
                        }
                        if (done){
                            sem_wait_op(SEM_EXAM);
                            if(shm->student_num == stu){
                                shm->exam_idx++;
                                shm->student_num = (shm->exam_idx >= 20) ? 9999 : 1001 + shm->exam_idx;
                                for (int q = 0; q < MAX_Q; q++){
                                    shm->q_status[q] = 0;
                                }
                                printf("[SYSTEM] Loaded exam %04d\n", shm->student_num);
                            }
                            sem_signal_op(SEM_EXAM);
                            break;
                        }
                        break;
                    }

                    printf("[TA%d] is Marking Q%d for %04d\n", ta, found+1, stu);
                    delay(1.0, 2.0);
                    sem_wait_op(SEM_QUEST);
                    shm->q_status[found] = 2;
                    sem_signal_op(SEM_QUEST);
                    printf("[TA%d] Done Q%d\n", ta, found+1);
                }
            }
            printf("[TA%d] is Terminating\n", ta);
            shmdt(shm);
            exit(0);
        }
    }
    for (i = 0; i < num_tas; i++){
        wait(NULL);
    }
    printf("[MAIN] is Done \n");
    shmdt(shm);
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);
    return 0;
}

