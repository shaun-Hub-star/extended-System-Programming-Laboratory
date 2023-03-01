#include <stdlib.h>
#include <linux/limits.h>
#include <stdio.h>
#include <unistd.h>
#include "LineParser.h"
#include <string.h>
#include <wait.h>
#include <fcntl.h>
#define STDIN  0
#define STDOUT 1
#define STDERR 2
#define HISTLEN 4

typedef struct cmdHistory
{
    int idx;
    char userInputCmd[2048];		
    cmdLine* parsedInput;	 
} cmdHistory;

void execute(cmdLine *pCmdLine);

void addCmdHistory(cmdHistory *cmdHistoryArr[], char userInputCmd[], cmdLine* parsedInput);
void addCmdHistoryS(cmdHistory *cmdHistoryArr[], cmdHistory *cmdHistoryElement);
void printHistory(cmdHistory *cmdHistoryArr[], int numberOfHistory);
void freeCmdHistory(cmdHistory *cmdHistory);
cmdLine* copyCmdLine( cmdLine* toCopy);
cmdHistory* getLastCmdHistory(cmdHistory *cmdHistoryArr[]);
cmdHistory* getIdxCmdHistory(cmdHistory *cmdHistoryArr[], int idx);
char *strClone(const char *source);
int fd[2];
int closeFD;
int pipeMode = 0;
int debug = 0;

int cmdHistoryArrIndex = 0;

int main(int argc, char** argv){

    cmdHistory *cmdHistoryArr[HISTLEN];
    int child1, child2;
    int doubleExclamationMark;
    int exclamationMark;
    int history;
    cmdLine *parsedInput;
    char userInputCmd[2048];
    char newDirCD[PATH_MAX];
    char cwd[PATH_MAX];
    FILE* input = stdin;
    //display the current working direcctory the path length is at most PATH_MAX
    if(argc>= 2 && strncmp(argv[1], "-d", 2) == 0)
        debug = 1;
    
    while(1){
        doubleExclamationMark = 0;
        exclamationMark = 0;
        history = 0;
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
        
        if(strcmp(parsedInput->arguments[0], "!!") == 0)
            doubleExclamationMark = 1;
        if(strcmp(parsedInput->arguments[0], "!") == 0)
            exclamationMark = 1; 
        if(strncmp(parsedInput->arguments[0], "history", 7) == 0)
            history = 1;
        
        if(!doubleExclamationMark && !exclamationMark)
            addCmdHistory(cmdHistoryArr, userInputCmd, parsedInput);
        
        
        int numberOfHistory;
        
        if(doubleExclamationMark){
            cmdHistory* cmdH = getLastCmdHistory(cmdHistoryArr);
            addCmdHistoryS(cmdHistoryArr, cmdH);
            strcpy(userInputCmd, cmdH -> userInputCmd);
            parsedInput = cmdH -> parsedInput;
            if(strncmp(parsedInput->arguments[0], "history", 7) == 0)
                history = 1;
        }
        if(exclamationMark && parsedInput->argCount >= 2){
            fprintf(stderr,"exclamationMark \n");
            int index = atoi(parsedInput->arguments[1]);
            cmdHistory* cmdH = getIdxCmdHistory(cmdHistoryArr, index);
            addCmdHistoryS(cmdHistoryArr, cmdH);
            strcpy(userInputCmd, cmdH -> userInputCmd);
            parsedInput = cmdH -> parsedInput;
            if(strncmp(parsedInput->arguments[0], "history", 7) == 0)
                history = 1;
        }
        
        if(history){
            numberOfHistory = atoi(parsedInput->arguments[1]);
            if(numberOfHistory == 0){
                fprintf(stderr,"error converting to number or cannot show 0 history\n");
                continue;
            }
            if(cmdHistoryArrIndex - numberOfHistory < 0){
                fprintf(stderr,"does not have %d commands to show\n", numberOfHistory);
                continue;
            }
            printHistory(cmdHistoryArr, numberOfHistory);
            continue;
        }
        
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
void addCmdHistory(cmdHistory *cmdHistoryArr[], char userInputCmd[], cmdLine* parsedInput){
    cmdHistory * newHistory = malloc(sizeof(cmdHistory));
    char* cloneUserInputCmd = (char*)malloc(strlen(userInputCmd) + 1);
    strcpy(cloneUserInputCmd, userInputCmd);
    strcpy(newHistory -> userInputCmd, cloneUserInputCmd);
    newHistory -> parsedInput = parseCmdLines(userInputCmd);//copyCmdLine(parsedInput);
    newHistory -> idx = cmdHistoryArrIndex;
    //cmdHistory* currentElementInTheArray = cmdHistoryArr[cmdHistoryArrIndex % HISTLEN];
    //if(currentElementInTheArray != NULL)
        //freeCmdHistory(currentElementInTheArray);
    cmdHistoryArr[cmdHistoryArrIndex % HISTLEN] = newHistory;
    cmdHistoryArrIndex += 1;
}

void addCmdHistoryS(cmdHistory *cmdHistoryArr[], cmdHistory *cmdHistoryElement){
    char userInputCmd[2048];
    strcpy(userInputCmd,cmdHistoryElement->userInputCmd);
    cmdLine* parsedInput = parseCmdLines(userInputCmd);//copyCmdLine(cmdHistoryElement->parsedInput);
    addCmdHistory(cmdHistoryArr, userInputCmd, parsedInput);
    //cmdHistoryArrIndex -= 1;

}
void printHistory(cmdHistory *cmdHistoryArr[], int numberOfHistory){
    int fromIndex = (cmdHistoryArrIndex - numberOfHistory) % HISTLEN;
    for(int i = fromIndex; i < fromIndex + numberOfHistory; i++){
        printf("index: %d\tcomand: %s\n", cmdHistoryArr[i % HISTLEN] -> idx, cmdHistoryArr[i % HISTLEN] -> userInputCmd);  
    }

}
cmdHistory* getLastCmdHistory(cmdHistory *cmdHistoryArr[]){
    return cmdHistoryArr[(cmdHistoryArrIndex-1) % HISTLEN];
}
cmdHistory* getIdxCmdHistory(cmdHistory *cmdHistoryArr[], int idx){
    for(int i = 0; i < HISTLEN; i++){
        if(cmdHistoryArr[i] != NULL && cmdHistoryArr[i]->idx == idx)
            return cmdHistoryArr[i];
    }
    return NULL;

}
void freeCmdHistory(cmdHistory *cmdH){
    //freeCmdLines(cmdH->parsedInput);
    free(cmdH->userInputCmd);
    cmdH->idx = 0;
}



cmdLine* copyCmdLine(cmdLine* toCopy){
    cmdLine * newCmdLine = malloc(sizeof(cmdLine));

    
    for(int i = 0;i < toCopy->argCount; i++)
        ((char**)newCmdLine->arguments)[i] = strClone(toCopy->arguments[i]);
        //replaceCmdArg(newCmdLine, i, toCopy->arguments[i]);
    newCmdLine->argCount = toCopy->argCount;
    if(toCopy->inputRedirect != NULL)
        strcpy((char*)(newCmdLine->inputRedirect), strClone(toCopy->inputRedirect));
    if(toCopy->outputRedirect != NULL)
        strcpy((char*)(newCmdLine->outputRedirect), strClone(toCopy->inputRedirect));
    newCmdLine->blocking = toCopy->blocking;
    newCmdLine->idx = toCopy->idx;
    if(toCopy->next != NULL){
        newCmdLine->next = malloc(sizeof(cmdLine));
        for(int i = 0;i < toCopy->next->argCount; i++)
            replaceCmdArg(newCmdLine->next, i, strClone(toCopy->next->arguments[i]));
            
        newCmdLine->next->argCount = toCopy->argCount;
        if(toCopy->inputRedirect != NULL)
            strcpy((char*)(newCmdLine->inputRedirect), strClone(toCopy->inputRedirect));
        if(toCopy->outputRedirect != NULL)
            strcpy((char*)(newCmdLine->outputRedirect), strClone(toCopy->inputRedirect));
        newCmdLine->next->blocking = toCopy->next->blocking;
        newCmdLine->next->idx = toCopy->next->idx;
    }
    return newCmdLine;

}
char *strClone(const char *source)
{
    char* clone = (char*)malloc(strlen(source) + 1);
    strcpy(clone, source);
    return clone;
}

