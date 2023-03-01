#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define STDIN 0
#define STDOUT 1
#define STDERR 2

int main(int argc, char**argv){
    
    int debug = 0;
    if(argc == 2 && strcmp(argv[1], "-d") == 0)
        debug = 1;

    int fd[2];
    //fd[0] read from
    //fd[1] write to
    if(pipe(fd) == -1){
        return 1;
    }
    if(debug)
        fprintf(stderr, "(parent process>forking...)\n");
    int child1 = fork();
    if(child1 != 0 && debug)
        fprintf(stderr, "(parent process>created process with id: %d)\n", child1);
    if(child1 == 0){
        close(STDOUT);
        
        if(debug)
            fprintf(stderr, "(child1>redirecting stdout to the write end of the pipe...)\n");
        dup(fd[1]);

        close(fd[1]);
        char* arguments[] = {"ls","-l", NULL};
        
        if(debug){
            fprintf(stderr, "(child1>going to execute cmd: ls -l\n");
            /*int i = 0;
            while(arguments[i] != NULL)
                fprintf(stderr, "%s ", arguments[i++]);
            fprintf(stderr, ")\n");*/
        }
        
        execvp(arguments[0], arguments);
        puts("error!");
        exit(1);
    }
    
    if(debug)
        fprintf(stderr, "(parent process> closing the write end of the pipe...)\n");
    close(fd[1]);
    
    
    
    if(debug)
        fprintf(stderr, "(parent process>forking...)\n");
    int child2 = fork();
    
    if(child2 != 0 && debug)
        fprintf(stderr, "(parent process>created process with id: %d)\n", child2);
    
    
    if(child2 == 0){
        close(STDIN);
        if(debug)
            fprintf(stderr, "(child2>redirecting stdin to the read end of the pipe ...)\n");
            
        dup(fd[0]);
        close(fd[0]);
        char *arguments[] = {"tail", "-n", "2", NULL};
        
        if(debug){
            fprintf(stderr, "(child2>going to execute cmd: tail -n 2\n");
            /*int i = 0;
            while(arguments[i] != NULL)
                fprintf(stderr, "%s ", arguments[i++]);
            fprintf(stderr, ")\n");*/
        }
        
        execvp(arguments[0], arguments);
        puts("error!");
        exit(1);
    }
    
    if(debug)
        fprintf(stderr, "(parent process> closing the read end of the pipe...)\n");
    
    close(fd[0]);
    
    if(debug)
        fprintf(stderr, "(parent_process>waiting for child processes to terminate ...)\n");
    waitpid(child1, NULL, 0);
    waitpid(child2, NULL, 0);
    
    if(debug)
        fprintf(stderr, "(parent_process>exiting...)\n");
    return 0;
    
}