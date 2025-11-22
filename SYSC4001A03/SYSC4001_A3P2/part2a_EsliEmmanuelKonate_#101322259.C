/**
 * This is the TA Marking system without the semaphore
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <time.h>

#define SHM_KEY 1234
#define MAX_Q 5
#define MAX_EXAMS 25

typedef struct {
    int rubric_num[MAX_Q];
    char rubric_ans[MAX_Q];
    int student_num, q_status[MAX_Q]; // so 0 will be free, 1 will be marking and 2 will be done
    int exam_idx, total_exams, terminate;
} SharedData;

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

    // now we can load the rubric
    FILE *fp = fopen("rubric.txt", "r");
    for (i = 0; i < MAX_Q; i++) fscanf(fp, "%d, %c", &shm->rubric_num[i], &shm->rubric_ans[i]);
    fclose(fp);

    //now we set the exam list
    shm->total_exams = 21;
    shm->exam_idx = 0;
    //we will start with the first student
    shm->student_num = 1001;

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

                printf("[TA%d] Exam %04d - is Reviewing rubric\n", ta, stu);
                for (int q = 0; q < MAX_Q; q++){
                    delay(0.5, 1.0);
                    if (rand() % 5 == 0){
                        char old = shm->rubric_ans[q];
                        shm->rubric_ans[q] = old + 1;
                        printf("[TA%d] has Corrected Q%d: %c->%c\n", ta, q+1, old, shm->rubric_ans[q]);
                    }
                }
                printf("[TA%d] is Marking questions\n", ta);
                for (int q = 0; q< MAX_Q; q++){
                    if (shm->q_status[q] == 0){
                        shm->q_status[q] = 1;
                        printf("[TA%d] is Marking Q%d for %04d\n", ta, q+1, stu);
                        delay(1.0, 2.0);
                        shm->q_status[q] = 2;
                        printf("[TA%d] Done Q%d\n", ta, q+1);
                    }
                }

                //now we check if all is done then load next
                int done = 1;
                for (int q = 0; q < MAX_Q; q++) if (shm->q_status[q] != 2) done = 0;
                if (done){
                    shm->exam_idx++;
                    if (shm->exam_idx >= 20){
                        shm->student_num = 9999;
                    } else {
                        shm->student_num = 1001 + shm->exam_idx;
                        for (int q = 0; q < MAX_Q; q++){
                            shm->q_status[q] = 0;
                        }
                        printf("[SYSTEM] Next exam: %04d\n", shm->student_num);
                    }
                }
            }
            printf("[TA%d] is finishing\n", ta);
            shmdt(shm);
            exit(0);
        }
    }
    for (i = 0; i < num_tas; i++){
        wait(NULL);
    }
    printf("[MAIN] Done\n");
    shmdt(shm);
    shmctl(shmid, IPC_RMID, NULL);
    return 0;
}


