#include <stdlib.h>
#include <linux/limits.h>
#include <stdio.h>
#include <unistd.h>
#include "LineParser.h"
#include <string.h>
#include <wait.h>
void execute(cmdLine *pCmdLine);

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



int main(int argc, char** argv){
  int pid;
  int debug = 0;
  cmdLine *parsedInput;
  char userInputCmd[2048];
  char newDirCD[PATH_MAX];
  char cwd[PATH_MAX];
  FILE* input = stdin;
  //display the current working direcctory the path length is at most PATH_MAX
  if(argc>= 2 && strncmp(argv[1], "-d", 2) == 0)
    debug = 1;
  
  while(1){
  
    if (getcwd(cwd, sizeof(cwd)) == NULL)
        perror("could not get cwd");
      else
        printf("My cwd is: %s\n", cwd);
    
    //create a buffer of length 2048
    
    
    
    //read user input using fgets
    if(fgets(userInputCmd, sizeof(userInputCmd), input) == NULL)
        perror("could not get an input");
    if(strcmp(userInputCmd, "quit\n") == 0)
      break;
    

    
    
    //parse the input using parseCmdLines() returns cmdLine*
    parsedInput = parseCmdLines(userInputCmd);
    

    if (strcmp(parsedInput->arguments[0],"cd") == 0) {
        
        strcpy(newDirCD,parsedInput->arguments[1]);
        if(chdir(newDirCD)!=0 || parsedInput->argCount < 2)
            fprintf(stderr,"%s\n","Failed to perform cd");
    
    }
    
    //call execute
    if(!(pid = fork()))
      execute(parsedInput);
    
    if(debug)
      fprintf(stderr,"pid: %d\ncommand: %s\n", pid, userInputCmd);
    
    if(parsedInput -> blocking) 
      waitpid(pid,NULL,0);
    
    freeCmdLines(parsedInput);    

  
  }
  
  
  return 0;
}
void execute(cmdLine *pCmdLine){
  int i = 0;
  int argc = pCmdLine->argCount;
  char * const *arguments = pCmdLine->arguments;
  for(i = 0; i < argc; i++){
    
    execvp(arguments[i], arguments);
    puts("ERROR");
    
    exit(1);
  }
}


























