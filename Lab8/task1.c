#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

typedef struct {
    int debug_mode;
    char file_name[128]; 
    int unit_size; 
    unsigned char mem_buf[10000]; 
    size_t mem_count;
    int display_flag;
    /*Any additional fields you deem necessary */
} state;

typedef struct
{
    char *name;
    void (*func)(state* s);
} menu_option;

// Declare function prototypes for menu options
void debugMode(state* s);
void setFileName(state* s);
void setUnitSize(state* s);
void loadIntoMemory(state* s);
void toggleDisplayMode(state* s);
void memoryDisplay(state* s);
void saveIntoFile(state* s);
void memoryModify(state* s);
void quit(state* s);



int main(int argc, char* argv[])
{
    state* s = malloc(sizeof(state));
    s->debug_mode = 0;
    s->unit_size = 1;
    s->display_flag = 0;
    // Declare an array of menu options
    menu_option options[] =
    {
        {"Toggle Debug Mode", debugMode},
        {"Set file name", setFileName},
        {"Set unit size", setUnitSize},
        {"Load into memory", loadIntoMemory},
        {"Toggle display mode", toggleDisplayMode},
        {"Memory display", memoryDisplay},
        {"Save into file", saveIntoFile},
        {"Memory modify", memoryModify},
        {"Quit", quit}
    };
    int num_options = sizeof(options) / sizeof(options[0]);

    int choice;

    while (1)
    {
        // Display menu
        printf("Menu:\n");
        for (int i = 0; i < num_options; i++)
        {
            printf("%d. %s\n", i + 1, options[i].name);
        }
        printf("Enter your choice: ");
        scanf("%d", &choice);

        // Check if the choice is valid
        if (choice < 1 || choice > num_options)
        {
            printf("Invalid choice. Please try again.\n");
            continue;
        }
        //if(strcmp(options[choise - 1].name, "Quit") == 0)
        //    break;

        // Call the function for the selected menu option
        (*options[choice - 1].func)(s);
    }

    return 0;
}

// Function definitions for menu options
void debugMode(state* s){
    s->debug_mode = 1;
}
void setFileName(state* s){

    puts("Please enter the new file name");
    scanf("%s", s->file_name);
    if(s->debug_mode)
        printf("DEBUG: file name set to %s\n", s->file_name);
}
void setUnitSize(state* s){
    int unit_size;
    puts("Please enter unit size (1, 2 or 4)");
    scanf("%d", &unit_size);
    if(unit_size != 1 && unit_size != 2 && unit_size != 4){
        printf("ERROR: unit size may be of the values 1, 2 or 4, Entered: %d\n", unit_size);
        return;
    }
    s->unit_size = unit_size;
    if(s->debug_mode)
        printf("DEBUG: set unit size to %d\n", s->unit_size);
}
void loadIntoMemory(state* s){
    if(strcmp(s->file_name, "") == 0){
        puts("The file name is empty");
        return;
    }
    FILE* fp = fopen(s->file_name, "rb");
    if(fp == NULL){
        puts("ERROR: could not open the file");
        return;
    }
    int location;
    int length;
    puts("Please enter <location> in hexadecimal");
    scanf("%x", &location);//potential bug

    puts("Please enter the <length>");
    scanf("%d", &length);
    
    if(s->debug_mode)
        printf("The location is: %X\nThe length is: %d\nThe file name is: %s\n", location, length, s->file_name);
    
    if (fseek(fp, location, SEEK_SET) != 0){
        printf("Error seeking in file.\n");
        return;
    }
    size_t bytes_read = fread(s->mem_buf, s->unit_size, length, fp);
    if (bytes_read < length){
        if (feof(fp))
            printf("Reached end of file.\n");
        else
            memset(s->mem_buf, 0, sizeof(s->mem_buf));
        return;
    }
    s->mem_count = bytes_read;
    printf("Loaded %d units into memory.\n", bytes_read);
    fclose(fp);
}    
void toggleDisplayMode(state* s){
    if(!s->display_flag){
        puts("Display flag now on, hexadecimal representation");
        s->display_flag = 1;
    }else{
        puts("Display flag now off, decimal representation");
        s->display_flag = 0;
    }
}
void memoryDisplay(state* s){
    
    FILE* fp = fopen(s->file_name, "rb");
    char buffer[10000];
    //memset(buffer, 0, 10000);
    int addr;
    int length;
    size_t bytes_read;
    puts("Please enter <address> in hexadecimal");
    scanf("%x", &addr);//potential bug

    puts("Please enter the <length>");
    scanf("%d", &length);
    if(addr != 0){
        if (fseek(fp, addr, SEEK_SET) != 0){
            printf("Error seeking in file.\n");
            return;
        }
        bytes_read = fread(buffer, s->unit_size, length, fp);
        if (bytes_read < length){
            if (feof(fp))
                printf("Reached end of file.\n");
            else
                memset(s->mem_buf, 0, sizeof(s->mem_buf));
            return;
        }
    } 
    if(s->display_flag)
        puts("Hexadecimal\n==========");
    else
        puts("Decimal\n=======");
    
    
    if(s->unit_size == 1){
        for(int i = 0; i < length*s->unit_size; i++){
            if(addr == 0){
                if(s->display_flag)
                    printf("%2hX\n", s->mem_buf[i]);
                else
                    printf("%u\n", s->mem_buf[i]);
            }
            else{
                if(s->display_flag)
                    printf("%X\n", buffer[i]);
                else
                    printf("%u\n", buffer[i]);
            }
                
        }
    }else if(s->unit_size == 2){    
        for(int i = 0; i < length*s->unit_size; i+=2){
            if(addr == 0){
                if(s->display_flag)
                    printf("%X\n", s->mem_buf[i] + (s->mem_buf[i+1]<<8));
                else
                    printf("%u\n", s->mem_buf[i] + (s->mem_buf[i+1]<<8));
            }
            else{
                if(s->display_flag)
                    printf("%X\n", buffer[i] + (s->mem_buf[i+1]<<8));
                else
                    printf("%u\n", buffer[i] + (buffer[i+1]<<8));
            }
                
        }
     }else{   
        for(int i = 0; i < length*s->unit_size; i+=4){
            if(addr == 0){
                
                if(s->display_flag)
                    printf("%X\n", s->mem_buf[i] + (s->mem_buf[i+1]<<8) + (s->mem_buf[i+2]<<16) + (s->mem_buf[i+3]<<24));
                else
                    printf("%u\n", s->mem_buf[i] + (s->mem_buf[i+1]<<8) + (s->mem_buf[i+2]<<16) + (s->mem_buf[i+3]<<24));
            }
            else{
                if(s->display_flag)
                    printf("%X\n", buffer[i] + (buffer[i+1]<<8) + (buffer[i+2]<<16) + (buffer[i+3]<<24));
                else
                    printf("%u\n", buffer[i] + (buffer[i+1]<<8) + (buffer[i+2]<<16) + (buffer[i+3]<<24));
            }
                
        }
     }
    
    
    fclose(fp);

}
void saveIntoFile(state* s){
    intptr_t sourceAddr;
    int targetLoc;
    int length;
    unsigned char *memoryAddress;
    
    FILE* fp = fopen(s->file_name, "rb+");
    if(fp == NULL){
        puts("could not open the file for reading");
        return;
    }
    
    puts("Please enter <source-address> in hexadecimal");
    fflush(stdin);
    scanf("%x", &sourceAddr);
    puts("Please enter the <target-location>");
    fflush(stdin);
    scanf("%x", &targetLoc);
    fflush(stdin);
    puts("Please enter the <length>");
    scanf("%d", &length);
    
    if(s->debug_mode)
        printf("DEBUG: <source-address>:%X <target-location>: %d <length>: %d\n", sourceAddr, targetLoc, length);
    
    if(s->mem_buf != NULL && sourceAddr == 0)
        memoryAddress = s->mem_buf;
    else
        memoryAddress = (unsigned char *)sourceAddr;
    if(s->debug_mode)
        printf("DEBUG: <memory address>:%p\n", memoryAddress);
    
    fseek(fp, 0, SEEK_END);
    
    // Get the current position of the file position indicator
    long file_size = ftell(fp);
    printf("File length is: %ld\n", file_size);
    
    fseek(fp, targetLoc, SEEK_SET);
    
    if(targetLoc + length >= file_size){
        puts("cannot write to the file because the text overflows the file");
        return;
    }
    unsigned char buffer[5];
    for(int i = 0; i < length*s->unit_size; i+=s->unit_size){
        for(int j = 0; j < s->unit_size; j++)
            buffer[j] = *(memoryAddress + i +j);
        
        fwrite(buffer, sizeof(unsigned char), s->unit_size, fp);
    }
    fclose(fp);
    
    
}
void memoryModify(state* s){
    int targetLoc;
    int val;

    
    puts("Please enter <location> in hexadecimal");

    scanf("%x", &targetLoc);
    puts("Please enter <val> in hexadecimal");
    
    scanf("%x", &val);
    if(s->debug_mode)
        printf("DEBUG: <location>:%X <val>: %d\n", targetLoc, val);

    unsigned char writeBuffer[] = {val, val>>8, val>>16, val>>24};
    if(targetLoc + s->unit_size >= s->mem_count){
        puts("cannot write to this location because it overflows");
        return;
    }
    for(int i = 0; i < s->unit_size; i++)
        s->mem_buf[i+targetLoc] = writeBuffer[i];
    
    
    
}
void quit(state* s){
    if(s-> debug_mode)
        puts("quitting");
    free(s);
    exit(0);
}