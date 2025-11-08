#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    int counter = 0;
    pid_t pid;
    int status;

    printf("Process 1 (PID: %d) it begins now... \n", getpid());

    pid = fork();

    if (pid < 0){
        perror("Fork failed");
        exit(1);
    } else if (pid == 0){
        printf("Child PID: %d will exec process2 \n", getpid());
        sleep(1);

        execl("./process2_part3", "process2_part3", NULL);

        perror("exec fails");
        exit(1);
    } else {
        //parent processes
        printf("Parent PID %d, Child PID: %d \n\n", getpid(), pid);
        printf("Process 1 is waiting for Process 2 to finish \n\n");

       //wait for the child to finish
       wait(&status);

       if (WIFEXITED(status)){
        printf("Process 2 finished with exit status: %d\n", WEXITSTATUS(status));
        printf("Process 1 is finishing as Process 2 has completed. \n");
       }
    }
    
    return 0;
}