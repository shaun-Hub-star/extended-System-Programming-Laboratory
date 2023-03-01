#include <stdlib.h>
#include <linux/limits.h>
#include <stdio.h>
#include <unistd.h>
#include "LineParser.h"
#include <string.h>
#include <wait.h>
#include <fcntl.h>
#define STDIN 0
#define STDOUT 1
#define STDERR 2

void execute(cmdLine *pCmdLine);

int fd[2];
int closeFD;
int pipeMode = 0;
int debug = 0;
int main(int argc, char** argv){


    
    
    
    int child1, child2;
    
    cmdLine *parsedInput;
    char userInputCmd[2048];
    char newDirCD[PATH_MAX];
    char cwd[PATH_MAX];
    FILE* input = stdin;
    //display the current working direcctory the path length is at most PATH_MAX
    if(argc>= 2 && strncmp(argv[1], "-d", 2) == 0)
        debug = 1;
    
    while(1){
        if(pipe(fd) == -1){
            return 1;
        }
        closeFD = STDOUT;
        pipeMode = 0;
        if (getcwd(cwd, sizeof(cwd)) == NULL)
            perror("could not get cwd");
        else
            printf("\nMy cwd is: %s\n", cwd);
        
        //create a buffer of length 2048
        
        
        
        //read user input using fgets
        if(fgets(userInputCmd, sizeof(userInputCmd), input) == NULL)
            perror("could not get an input");
        if(strcmp(userInputCmd, "quit\n") == 0)
            break;
        
        
        //parse the input using parseCmdLines() returns cmdLine*
        parsedInput = parseCmdLines(userInputCmd);
        if(parsedInput->next != NULL)
            pipeMode = 1;
        
        if (strcmp(parsedInput->arguments[0],"cd") == 0) {
            
            strcpy(newDirCD,parsedInput->arguments[1]);
            if(chdir(newDirCD)!=0 || parsedInput->argCount < 2)
                fprintf(stderr,"%s\n","Failed to perform cd");
        
        }
        
        if(debug)
            fprintf(stderr, "(parent process>forking...)\n");
            
        child1 = fork();
        if(child1 != 0 && debug)
            fprintf(stderr, "(parent process>created process with id: %d)\n", child1);
        if(child1 == 0)
            execute(parsedInput);
        if(pipeMode){ 
            if(debug)
                fprintf(stderr, "(parent process> closing the write end of the pipe...)\n");
            close(fd[1]);
        
        
            if(debug)
                fprintf(stderr, "(parent process>forking...)\n");
            child2 = fork();
        
            if(child2 != 0 && debug)
                fprintf(stderr, "(parent process>created process with id: %d)\n", child2);
            closeFD = STDIN;
            if(child2 == 0)
                execute(parsedInput->next);
            if(debug)
                fprintf(stderr, "(parent process> closing the read end of the pipe...)\n");
        
            close(fd[0]);
        }
        
        
        
        
        
        if(!pipeMode){
            if(debug)
                fprintf(stderr,"pid: %d\ncommand: %s\n", child1, userInputCmd);
            if(parsedInput -> blocking) 
                waitpid(child1,NULL,0);
        }else{
        
            if(debug)
                fprintf(stderr, "(parent_process>waiting for child processes to terminate ...)\n");
            waitpid(child1, NULL, 0);
            waitpid(child2, NULL, 0);
        }   
        freeCmdLines(parsedInput);    
    
    }
    
    if(debug)
        fprintf(stderr, "(parent_process>exiting...)\n");
    return 0;
}
void execute(cmdLine *pCmdLine){
    //fd[1] write to the pipe
    //fd[0] read from a pipe
    
    if(pCmdLine->inputRedirect != NULL){
        if(debug)
            fprintf(stderr, "in inputRedirect\n");
        close(STDIN);
        int inFD = open(pCmdLine->inputRedirect, O_RDONLY);
        dup(inFD);
    }                           
    if(pCmdLine->outputRedirect != NULL){
        if(debug)
            fprintf(stderr, "in outputRedirect\n");
        close(STDOUT);
        int outFD = open(pCmdLine->outputRedirect,O_WRONLY);
        dup(outFD);
    }
    if(pipeMode){
        close(closeFD);
        if(debug)
            fprintf(stderr, "(child>redirecting stdout/stdin(%d) to the write end of the pipe...)\n", closeFD);
        dup(fd[closeFD]);
        close(fd[closeFD]);
    }

    char * const *arguments = pCmdLine->arguments;
    
    if(debug)
        fprintf(stderr, "(child>going to execute cmd %s)\n", arguments[0]);
    
    
    execvp(arguments[0], arguments);
    fprintf(stderr, "could not perform command");
    if(pCmdLine->inputRedirect != NULL){
        close(STDIN);
    }
    if(pCmdLine->outputRedirect != NULL){
        close(STDOUT);
    }
    exit(1);
    
}




