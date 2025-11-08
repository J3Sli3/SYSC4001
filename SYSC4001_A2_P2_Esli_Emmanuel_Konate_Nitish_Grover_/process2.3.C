#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    int counter = 0;

    printf("Process 2 starts with exec() \n");
    printf("Process 2 PID %d \n\n", getpid());

    // now we change here so that it stops at -500
    while (counter > -500){
        if (counter % 3 == 0){
            printf("Process 2 = PID %d, Cycle: %d   %d is a multiple of 3\n", getpid(), counter, counter);
        } else {
            printf("Process 2    PID %d, Cycle: %d\n", getpid(), counter);
        } 
        counter--;
        sleep(1);
    }
    printf("Process 2 has reached %d, it is now terminating \n", counter); 
    return 0;
}