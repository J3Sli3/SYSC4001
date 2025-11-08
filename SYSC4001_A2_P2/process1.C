#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main() {
    int counter = 0;
    pid_t pid;

    printf("Process 1 (PID: %d) it begins now... \n", getpid());

    pid = fork();

    if (pid < 0){
        perror("Fork failed");
        exit(1);
    } else if (pid == 0){
        printf("Child PID: %d will exec process2 \n", getpid());
        sleep(1);

        execl("./process2", "process2", NULL);

        perror("exec fails");
        exit(1);
    } else {
        printf("Parent PID %d, Child PID: %d \n\n", getpid(), pid);

        while(1){
            if (counter % 3 == 0){
                printf("Process 1 = PID %d  Cycle: %d - %d is a multiple of 3\n", getpid(), counter, counter);
            } else {
                printf("Process 1 - PID: %d  Cycle: %d\n", getpid(), counter);
            }
            counter++;
            sleep(1);
        }
    }
    return 0;
}