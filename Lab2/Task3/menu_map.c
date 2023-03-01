#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define TRUE 1
struct fun_desc {
  char *name;
  char (*fun)(char);
};

 /* The function checks if c is a number between 0x20 and 0x7E*/
int inRange(char c){
  
  return c > 0x20 && c< 0x7E;
}
 
 /* Ignores c, reads and returns a character from stdin using fgetc. */
 char my_get(char c){
   return fgetc(stdin);
 
 }


/* If c is a number between 0x20 and 0x7E, cprt prints the character of ASCII value c followed by a new line. Otherwise, cprt prints the dot ('.') character. After printing, cprt returns the value of c unchanged. */
char cprt(char c){
  if(inRange(c))
    printf("%c\n",c);
  else
    printf("%c\n",'.');
  return c;  
}


/* Gets a char c and returns its encrypted form by adding 3 to its value. If c is not between 0x20 and 0x7E it is returned unchanged */
char encrypt(char c){

  if(inRange(c)){
        return c + 3;  
    }
    return c;
}


/* Gets a char c and returns its decrypted form by reducing 3 to its value. If c is not between 0x20 and 0x7E it is returned unchanged */
char decrypt(char c){
  if(inRange(c)){
      return c - 3;  
  }
  return c;
}



/* xprt prints the value of c in a hexadecimal representation followed by a new line, and returns c unchanged. */
char xprt(char c){
  printf("%X\n",c);
  return c;
}



 
 
 /* Gets a char c, and if the char is 'q' , ends the program with exit code 0. Otherwise returns c. */
 char quit(char c){
 
   if(c == 'q')
     exit(0);
   return c;
 
 }


char censor(char c) {
  if(c == '!')
    return '.';
  else
    return c;
}

 
char* map(char *array, int array_length, char (*f) (char)){
  char* mapped_array = (char*)(malloc(array_length*sizeof(char)));
  /* TODO: Complete during task 2.a */
  for(int i = 0; i < array_length; i++)
      mapped_array[i] = (*f)(array[i]);
  
  
  return mapped_array;
}

struct fun_desc menu[] = { { "Get string", my_get }, { "Print string", cprt },{ "Print hex", xprt },{ "Censor", censor },{ "Encrypt", encrypt },{ "Decrypt", decrypt },{ "Quit", quit }, { NULL, NULL } };


void presentMenu(){
  printf("\nPlease choose a function:\n");
  
  for(int i = 0; menu[i].name != NULL; i++)
      printf("%d)\t%s\n",i,menu[i].name);  

  printf("Option : ");
}
int chooseAction(){
  return fgetc(stdin) - '0';
}
int numberOfActions(){
  int count;
  for(count = 0;menu[count].name != NULL; count++);
  return count;
  
}
int leagalAction(int actionNumber, int menuLength){
  return (actionNumber >=0 && actionNumber < menuLength);

}
int main(int argc, char **argv){

  int menuLength = numberOfActions();
  int base_len = 5;
  char* carray = (char*)(malloc(5 * sizeof(char)));
  for(int i = 0; i <= base_len; i++) carray[i] = '\0';
  int actionNumber;
  while(TRUE){

    presentMenu();
    actionNumber = chooseAction();
    while(actionNumber < 0 || actionNumber == 10)
      actionNumber = chooseAction();
    printf("\n");
    if(!leagalAction(actionNumber, menuLength)) {printf("Not within bounds\n");exit(0);}
    printf("Within bounds\n");
    if(actionNumber == 6)break;
    chooseAction();
    char* carray2 = map(carray, base_len, menu[actionNumber].fun);
    strcpy(carray, carray2);
    free(carray2);
    printf("Done.\n");
    
  }
}
