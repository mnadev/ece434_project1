#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]){
    printf("Process A is now running\n");
    int status;
    int value = 2;
    //A fork B
    int pid = fork();
    if (pid == 0){//child process B
        //let B sleep for 2 s
        sleep(2);
        value = value + 2;
        printf("Process B is now running\n");
        //B fork D
        int pid2 = fork();
        if (pid2 == 0){//child process D
            //let D sleep for 6 s
            sleep(6);
            value = value + 6;
            printf("Process D is now running\n");
            printf("Process D is now terminating\n");
            printf("Process D exits with return value %d\n", value);
        }else{
            printf("Process B, pid=%d, forked process D, pid=%d\n", getpid(), pid2);
            printf("Process B is waitting for child D to terminate\n");
            waitpid(pid2,&status,0);
            printf("Process B is now running again\n");
            printf("Process B is now terminating\n");
            printf("Process B exits with return value %d\n", value);
        }
    }else{//parent process A
        printf("Process A, pid=%d, forked process B, pid=%d\n", getpid(), pid);
        //A fork C
        int pid1 = fork();
        if (pid1 == 0){//child process C
            //let C sleep for 4 s
            sleep(4);
            value = value + 4;
            printf("Process C is now running\n");
            printf("Process C is now terminating\n");
            printf("Process C exits with return value %d\n", value);
        }else{//parent process A
            printf("Process A, pid=%d, forked process C, pid=%d\n", getpid(), pid1);
            printf("Process A is waitting for children B and C to terminate\n");
            waitpid(pid,&status,0);
            waitpid(pid1,&status,0);
            printf("Process A is now running again\n");
            printf("Process A is now terminating\n");
            printf("Process A exits with return value %d\n", value);
        }
    }
    return value;
}
