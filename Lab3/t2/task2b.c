#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define MIN(a,b) (((a)<(b))?(a):(b))
typedef struct virus {
    unsigned short SigSize;
    char virusName[16];
    unsigned char* sig;
} virus;

typedef struct link {
    struct link *nextVirus;
    virus *vir;
}link;

struct fun_desc {
    char *name;
    void (*fun)();
};

/************************Function Declarations************************/
virus* readVirus();
void printVirus(virus* virus, FILE* output);
void list_print();
link* list_append(link* virus_list, virus* data);
void list_free();
void load_signatures();
void present_menu();
int leagalAction(int actionNumber, int menuLength);
int numberOfActions();
void detect_virus_help(char* buffer, unsigned int size, link *virus_list);
void detect_virus();
void fix_file();
void kill_virus(FILE* fileName, int signitureOffset, int signitureSize);
void quit();
/*********************End Of Function Declarations**********************/

/****************************Global Variables***************************/
link* linked_list = NULL;
FILE* signature_file;
FILE* infected_file;
char buffer[10000];

struct fun_desc menu[] = {  { "Load signatures", load_signatures }, 
                            { "Print signatures", list_print },
                            { "Detect viruses", detect_virus },
                            { "Fix file", fix_file },
                            { "Quit", quit }, 
                            { NULL, NULL } };
/****************************End Of Global Variables***************************/

int main(int argc, char** argv){
    int menuLength = numberOfActions();
    int actionNumber;
    signature_file = fopen(argc == 2 ? argv[1] : "signatures-L","rb");
    printf("Enter the suspected file name that you would like to check: ");
    char fileName[100];
    memset(fileName, '\0', 100);
    fgets(fileName, 100, stdin);
    for(int i = 0; i < 100; i++){
        if(fileName[i] == '\n')
            fileName[i] = '\0';
    }
    infected_file = fopen(fileName, "r+");
    
    while(1){
        present_menu();
        actionNumber = fgetc(stdin) - '0';
        while(actionNumber <= 0 || actionNumber == 10)
            actionNumber = fgetc(stdin) - '0';
        //sscanf(buffer, "%d", &actionNumber);
        printf("\n");
        if(!leagalAction(actionNumber, menuLength)) {
            printf("Not within bounds\n");
            quit(stdout);
        }
        printf("Within bounds\n");
        menu[actionNumber - 1].fun(signature_file);
    }
    fclose(signature_file);
    fclose(infected_file);

    return 0;
}
virus* readVirus(){
    unsigned short length;
    int success;
    virus* vir = malloc(sizeof(virus));
    memset(vir->virusName,'\0',16);
    success = fread(vir, 18, 1, signature_file);
    if(success == 0){
        free(vir);
        return NULL;
    }
    length = vir->SigSize;

    vir->sig = calloc(length, sizeof(unsigned char));
    fread(vir->sig, length, 1, signature_file);
    return vir;
}
void printVirus(virus* v, FILE* output){
    fprintf(output, "Virus name: %s\nVisus size: %hu\n",
                (*v).virusName, (*v).SigSize);
    for(int i = 0; i < v->SigSize; i++){
        if(i % 21 == 0)
            fprintf(output, "\n");
        fprintf(output, "%02X ", v->sig[i]);
    }
    fprintf(output, "\n\n");
}
void list_print(){
    
    //print virus in every link
    link* pointer;
    if(linked_list == NULL)
        return;
    pointer = linked_list;
    while(pointer != NULL){
        printVirus(pointer->vir, stdout);
        pointer = pointer->nextVirus;
    }
}
link* list_append(link* virus_list, virus* data){
    /* append at the begining of the virus list*/
    // link* new_link = (link*)malloc(sizeof(link));
    // new_link->vir = data;
    // if(virus_list == NULL)
    //  return new_link;
    // new_link->nextVirus = virus_list;
    // return new_link;

    /* append at the end of the virus list*/
    link* new_link = (link*)malloc(sizeof(link));
    link* pointer = virus_list;
    new_link->vir = data;
    new_link->nextVirus = NULL;
    if(virus_list == NULL)
        return new_link;

    while(pointer->nextVirus != NULL){
        pointer = pointer->nextVirus;
    }
    pointer->nextVirus = new_link;
    return virus_list;
}

void list_free(){
    link* current = linked_list;
    link* prev;
    while(current->nextVirus != NULL){
        prev = NULL;
        prev = current;
        current = current->nextVirus;
        free(prev->vir->sig);
        free(prev->vir);
        free(prev);
    }
    free(current);
}
void load_signatures(){
    //FILE* signature_file = fopen("signatures-L","rb");
    virus* v;
    fseek(signature_file, 4, SEEK_SET);
    while((v = readVirus(signature_file)) != NULL)
        linked_list = list_append(linked_list, v);
           
    
}
void present_menu(){
  printf("\nPlease choose a function:\n");
  
  for(int i = 0; menu[i].name != NULL; i++)
      printf("%d)\t%s\n",i+1,menu[i].name);  

  printf("Option : ");
}

int numberOfActions(){
    int count;
    for(count = 0;menu[count].name != NULL; count++);
    return count;
  
}
int leagalAction(int actionNumber, int menuLength){
    return (actionNumber >=1 && actionNumber <= menuLength);
}

void quit(){
    printf("quiting...\n\n");
    list_free(linked_list);
    fclose(infected_file);
    fclose(signature_file);
    exit(0);
}
void detect_virus_help(char* buffer, unsigned int size, link *virus_list){

    link* current = virus_list;
    unsigned short sigLength;
    int index = 0;
    while(current != NULL){
        sigLength = current -> vir ->SigSize;
        while(index < size - sigLength + 1){
            if(memcmp(&buffer[index], current->vir->sig, sigLength) == 0)
            {
                printf("Virus detected:\nStarting byte location:\t%d\nVirus Name:\t%s\nSignature length:\t%hu\n\n\n",index, current->vir->virusName,sigLength);
            }
            index++;

        }
        index = 0;
        current = current->nextVirus;
    }
}

void detect_virus(){
    fseek(infected_file, 0, SEEK_END);
    unsigned int infected_length = ftell(infected_file);
    fseek(infected_file, 0, SEEK_SET);
    
    unsigned int file_size = MIN(10000, infected_length);
    memset(buffer,'\0',10000);
    fread(buffer, file_size, 1, infected_file);
    detect_virus_help(buffer, file_size, linked_list);
}
void kill_virus(FILE* suspected, int signitureOffset, int signitureSize) {
    fseek(suspected, signitureOffset, SEEK_SET);
    char buffer[signitureSize];
    memset(buffer, ';',signitureSize);
    fwrite(buffer, signitureSize, 1, suspected);
}
void fix_file(FILE* f){
    f = stdout;
    
    int signitureOffset;
    printf("Enter the signiture offset byte: ");
    scanf("%d",&signitureOffset);
    getchar();

    int signitureSize;
    printf("Enter the signeture size of the suspected virus: ");
    scanf("%d",&signitureSize);
    getchar();
    kill_virus(infected_file, signitureOffset, signitureSize);
    
}
