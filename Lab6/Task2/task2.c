#include <stdlib.h>
#include <linux/limits.h>
#include <stdio.h>
#include <unistd.h>
#include "LineParser.h"
#include <string.h>
#include <wait.h>
#include <sys/types.h>
#include <sys/wait.h>
#define TERMINATED -1
#define RUNNING 1
#define SUSPENDED 0
#define UN_INITIALIZED -999
typedef struct process
{
    cmdLine *cmd;         /* the parsed command line*/
    pid_t pid;            /* the process id that is running the command*/
    int status;           /* status of the process: RUNNING/SUSPENDED/TERMINATED */
    struct process *next; /* next process in chain */
} process;

/*
typedef struct cmdLine
{
    char * const arguments[MAX_ARGUMENTS];
    int argCount;
    char const *inputRedirect;
    char const *outputRedirect;
    char blocking;
    int idx;
    struct cmdLine* next;
} cmdLine;
*/
void execute(cmdLine *pCmdLine);
void addProcess(process **process_list, cmdLine *cmd, pid_t pid);
void printProcessList(process **process_list);
void appendCmdLineToHead(cmdLine *head_new_cmd, cmdLine *cmd);
void printProcess(process *process, unsigned int index);
char *strClone(const char *source);
void freeProcessList(process *process_list);
void updateProcessList(process **process_list);
void freeProcess(process *process);
const char *getProcessStatus(int status);
void deleteTerminatedProcesses(process** processList);

process **process_list;

int main(int argc, char **argv)
{
    process_list = malloc(sizeof(process *));
    (*process_list) = malloc(sizeof(process));
    (*process_list)->status = UN_INITIALIZED;
    pid_t pid;
    cmdLine *parsedInput;
    int debug = 0;

    char userInputCmd[2048];
    char newDirCD[PATH_MAX];
    char cwd[PATH_MAX];
    FILE *input = stdin;
    // display the current working direcctory the path length is at most PATH_MAX
    if (argc >= 2 && strncmp(argv[1], "-d", 2) == 0)
        debug = 1;

    while (1)
    {

        if (getcwd(cwd, sizeof(cwd)) == NULL)
            perror("could not get cwd");
        else
            printf("My cwd is: %s\n", cwd);

        // create a buffer of length 2048

        // read user input using fgets
        if (fgets(userInputCmd, sizeof(userInputCmd), input) == NULL)
            perror("could not get an input");
        if (strcmp(userInputCmd, "quit\n") == 0)
            break;

        // parse the input using parseCmdLines() returns cmdLine*
        parsedInput = parseCmdLines(userInputCmd);

        if (strcmp(parsedInput->arguments[0], "cd") == 0)
        {

            strcpy(newDirCD, parsedInput->arguments[1]);

            if (chdir(newDirCD) != 0 || parsedInput->argCount < 2)
                fprintf(stderr, "%s\n", "Failed to perform cd");
            continue;
        }
        
        if (strcmp(parsedInput->arguments[0], "procs") == 0)
        {
            printProcessList(process_list);
            continue;
        }
        if (strcmp(parsedInput->arguments[0], "suspend") == 0) 
        {
            kill(atoi(parsedInput-> arguments[1]), SIGTSTP);
            continue;
        }

        if (strcmp(parsedInput-> arguments[0] ,"wake")== 0)
        {
            kill(atoi(parsedInput->arguments[1]), SIGCONT);
            continue;
        }
        if (strcmp(parsedInput-> arguments[0] ,"kill")== 0)
        {
            kill(atoi(parsedInput->arguments[1]), SIGINT);
            continue;
        }

        // call execute
        if (!(pid = fork()))
        {
            execute(parsedInput);
        }
    
        addProcess(process_list, parsedInput, pid);
        
        if (debug)
            fprintf(stderr, "pid: %ld\ncommand: %s\n", (long)pid, userInputCmd);

        if (parsedInput->blocking)
            waitpid(pid, NULL, 0);

        freeCmdLines(parsedInput);
    }
    return 0;
}
void execute(cmdLine *pCmdLine)
{

    int argc = pCmdLine->argCount;
    char *const *arguments = pCmdLine->arguments;
    if (argc < 1)
        exit(1);
    // printf("%s argc: %d\n", arguments[i], argc);
   

    execvp(arguments[0], arguments);
    puts("ERROR");
    
    exit(1);
}

void addProcess(process **_process_list, cmdLine *cmd, pid_t pid)
{
    process *p = malloc(sizeof(process));
    process *pointerToProcess_list = (*_process_list);

    cmdLine *head_new_cmd = malloc(sizeof(cmdLine));
    head_new_cmd->idx = UN_INITIALIZED;

    cmdLine *pointerToHeadNewCmd = head_new_cmd;
    cmdLine *pointerToCmd = cmd;
    while (pointerToCmd != NULL)
    {
        appendCmdLineToHead(pointerToHeadNewCmd, pointerToCmd);
        pointerToHeadNewCmd = pointerToHeadNewCmd->next;
        pointerToCmd = pointerToCmd->next;
    }
    p->cmd = head_new_cmd;
    p->pid = pid;
    p->next = NULL;
    p->status = RUNNING;

    if ((*_process_list) == NULL || (*_process_list)->status == UN_INITIALIZED)
    {
        (*_process_list) = p;
    }
    else
    {
        pointerToProcess_list = (*_process_list);
        while (pointerToProcess_list->next != NULL)
            pointerToProcess_list = pointerToProcess_list->next;
        pointerToProcess_list->next = p;
    }
}
char *strClone(const char *source)
{
    char *clone = (char *)malloc(strlen(source) + 1);
    strcpy(clone, source);
    return clone;
}
void appendCmdLineToHead(cmdLine *head_new_cmd, cmdLine *cmd)
{
    cmdLine *_cmd;
    if (head_new_cmd->idx == UN_INITIALIZED)
    {
        for (int i = 0; i < cmd->argCount; i++)
        {
            //(char**)(_cmd->arguments[i]) = strClone(cmd->arguments[i]);
            ((char **)head_new_cmd->arguments)[i] = strClone(cmd->arguments[i]);
            printf("%s\n", (head_new_cmd->arguments)[i]);
        }
        head_new_cmd->argCount = cmd->argCount;
        head_new_cmd->inputRedirect = cmd->inputRedirect;
        head_new_cmd->outputRedirect = cmd->outputRedirect;
        head_new_cmd->blocking = cmd->blocking;
        head_new_cmd->idx = cmd->idx;
        head_new_cmd->next = NULL;
    }
    else
    {
        puts("maybe the bug is here");
        _cmd = malloc(sizeof(cmdLine));

        for (int i = 0; i < cmd->argCount; i++)
        {
            //(char**)(_cmd->arguments[i]) = strClone(cmd->arguments[i]);
            ((char **)_cmd->arguments)[i] = strClone(cmd->arguments[i]);
            printf("%s\n", (_cmd->arguments)[i]);
        }

        _cmd->argCount = cmd->argCount;
        _cmd->inputRedirect = cmd->inputRedirect;
        _cmd->outputRedirect = cmd->outputRedirect;
        _cmd->blocking = cmd->blocking;
        _cmd->idx = cmd->idx;
        _cmd->next = NULL;

        head_new_cmd->next = _cmd;
    }
}
void printProcessList(process **_process_list)
{
    if ((*_process_list) == NULL || (*_process_list)->status == UN_INITIALIZED)
    {
        puts("process list is empty");
        return;
    }
    updateProcessList(_process_list);
    process *pointerToProcess_list = (*_process_list);
    unsigned int index = 1;
    while (pointerToProcess_list != NULL)
    {
        printProcess(pointerToProcess_list, index++);
        pointerToProcess_list = pointerToProcess_list->next;
    }
    deleteTerminatedProcesses(_process_list);
}

void printProcess(process *process, unsigned int index)
{
    puts("INDEX\tPID\tSTATUS\t\tCMD");
    printf("%u\t%d\t%s\t%s\n", index, process->pid, getProcessStatus(process->status), (process->cmd->arguments)[0]);
}
const char *getProcessStatus(int status)
{
    if (status == RUNNING)
        return "RUNNING";
    if (status == TERMINATED)
        return "TERMINATED";
    if (status == SUSPENDED)
        return "SUSPENDED";
    else
        return "BUG IN STATUS";
}

void freeProcessList(process *process_list)
{
    if (process_list == NULL)
        return;
    freeProcessList(process_list->next);
    freeProcess(process_list);
}
void freeProcess(process *process)
{
    freeCmdLines(process->cmd);
    process->next = NULL;
    process->pid = 0;
    process->status = 0;
}
void updateProcessList(process **process_list)
{
    process *processPointer = (*process_list);
    int status;
    pid_t pid;
    int code;
    while (processPointer != NULL)
    {
        pid = processPointer->pid;
        code = waitpid(pid, &status, WNOHANG | WUNTRACED | WCONTINUED);

        if (code == -1)
        {
            processPointer->status = TERMINATED;
        }
        else if (code == 0){}
        else if (WIFEXITED(status) || WIFSIGNALED(status))
        {
            processPointer->status = TERMINATED;
        }
        else if (WIFSTOPPED(status))
        {
            processPointer->status = SUSPENDED;
        }
        else if (WIFCONTINUED(status))
        {
            processPointer->status = RUNNING;
        }
        else
        {
            exit(1);
        }
        // update the process iteslf
        printf("%d\n", processPointer->status);

        processPointer = processPointer->next;
    }
}

void deleteTerminatedProcesses(process** processList)
{
    process *temp = *processList, *prev;

    if (temp != NULL && temp->status == TERMINATED) {
        *processList = temp->next;
        freeProcess(temp);
        temp = *processList;
    }
    
    while(1)
    {
        puts("inside the delete terminate processes");
        while (temp != NULL && temp->status != TERMINATED) 
        {
            prev = temp;
            temp = temp->next;
        }
        if(temp == NULL) break;
        
        prev->next = temp->next;
        freeProcess(temp);
        temp = prev->next;
    }
 
}


