#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <sys/mman.h>
#include <elf.h>
#include <fcntl.h>

typedef struct {
    int debug_mode;
    char file_name[128]; 
    int currentfd;
    Elf32_Ehdr* ehdr;
    void* start;
    size_t file_size;
    /*Any additional fields you deem necessary */
} state;

typedef struct
{
    char *name;
    void (*func)(state* s);
} menu_option;

// Declare function prototypes for menu options
void debugMode(state* s);
void examineElfFile(state* s);
void printSectionNames(state* s);
void printSymbols(state* s);
void relocationTable(state* s);
void quit(state* s);
char* printTpye(Elf32_Shdr* sectionHeaderPointer);
char *r_type_to_string(Elf32_Word r_type);
const char* get_st_info_string(Elf32_Sym *sym);
int main(int argc, char* argv[])
{
    state* s = malloc(sizeof(state));
    s->debug_mode = 0;
    
    // Declare an array of menu options
    menu_option options[] =
    {
        {"Toggle Debug Mode", debugMode},
        {"Examine ELF File", examineElfFile},
        {"Print Section Names", printSectionNames},
        {"Print Symbols", printSymbols},
        {"Relocation Tables", relocationTable},
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
void examineElfFile(state* s){

    puts("Please enter the new file name");
    scanf("%s", s->file_name);
    if(s->debug_mode)
        printf("DEBUG: file name set to %s\n", s->file_name);
        
    int fd = open(s->file_name, O_RDONLY);//open for reading and writing
    if(fd < 0){
        puts("failed to open the file");
        return;
    
    }
     s->currentfd = fd;  

    struct stat sb;
    fstat(fd, &sb);
    size_t size = sb.st_size;
    s->file_size = size;
    
    // Map the file into memory
    void* addr = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) {
        puts("failed to mmap");
        close(fd);
        return;
    }
    s->start = addr;
    // Cast the mapped memory to an Elf32_Ehdr pointer
    Elf32_Ehdr* ehdr = (Elf32_Ehdr*) addr;
    s->ehdr = ehdr;
    char* bytes = (char*) addr;
    char magicNumbers[] = {bytes[0], bytes[1], bytes[2], bytes[3]};
    printf("%s %d\n", magicNumbers, strlen(magicNumbers));
    if (memcmp(ehdr->e_ident, ELFMAG, SELFMAG) != 0) {
        puts("The file is not ELF therefore it is not supported");
        munmap(addr, size);
        close(fd);
        s->currentfd = -1;
        return;
    
    }
        
    printf("The magic numbers are: %X %X %X %X\n", magicNumbers[0], magicNumbers[1], magicNumbers[2], magicNumbers[3]);
    //2.
    printf("Data encoding scheme: ");
    switch (ehdr->e_ident[EI_DATA]) {
        case ELFDATA2LSB:
            printf("2's complement, little endian\n");
            break;
        case ELFDATA2MSB:
            printf("2's complement, big endian\n");
            break;
        default:
            printf("Unknown\n");
            break;
  }
    //3.
  // Print the entry point in hexadecimal
    printf("Entry point: 0x%x\n", ehdr->e_entry);

  // Print the file offset of the section header table
    printf("Section header table offset: %u\n", ehdr->e_shoff);

  // Print the number of section header entries
    printf("Number of section header entries: %u\n", ehdr->e_shnum);
//6.
  // Print the size of each section header entry
    printf("Size of each section header entry: %u\n", ehdr->e_shentsize);
    //7
    printf("The file offset in which the program header table resides: %u\n", ehdr->e_phoff);
    //8
    
    printf("The number of program header entries: %u\n", ehdr->e_phnum);
    //9
    
    printf("The size of each program header entry: %u\n", ehdr->e_ehsize);
}

void printSectionNames(state* s){
    
    
    size_t sectionHeaderTableOffset = s->ehdr->e_shoff;
    size_t numberOfEntriesInSectionHeader = s->ehdr->e_shnum;
    size_t sectionHeaderEntrySize = s->ehdr->e_shentsize;
    
    // Get the offset of the string table
    size_t stringTableIndex = s->ehdr->e_shstrndx;
    Elf32_Shdr *shstrhdr = (Elf32_Shdr*)(s->start + sectionHeaderTableOffset + stringTableIndex * sectionHeaderEntrySize);
    size_t stringTableOffset = shstrhdr->sh_offset;
     for (size_t i = 0; i < numberOfEntriesInSectionHeader; i++) {
    // Get the section header
        Elf32_Shdr *currentSectionHeader = (Elf32_Shdr*)(s->start + sectionHeaderTableOffset + i * sectionHeaderEntrySize);

    // Get the index of the name in the string table
        size_t name_index = currentSectionHeader->sh_name;

    // Look up the name in the string table
        char *name = (char*)(s->start + stringTableOffset + name_index);
        printf("[%d]\t%s\t\t%s\t\t%x\t\t%x\t\t%x\n",i, name, printTpye(currentSectionHeader), 
        currentSectionHeader->sh_addr,currentSectionHeader->sh_offset, currentSectionHeader->sh_size);
    }
    

}
void printSymbols(state* s){
    size_t sectionHeaderTableOffset = s->ehdr->e_shoff;
    size_t numberOfEntriesInSectionHeader = s->ehdr->e_shnum;
    size_t sectionHeaderEntrySize = s->ehdr->e_shentsize;
    
    Elf32_Shdr *symbolTablePointer = NULL;
    Elf32_Shdr *stringTablePointer = NULL;
     for (size_t i = 0; i < numberOfEntriesInSectionHeader; i++) {
    // Get the section header
        Elf32_Shdr *currentSectionHeader = (Elf32_Shdr*)(s->start + sectionHeaderTableOffset + i * sectionHeaderEntrySize);

        if(currentSectionHeader->sh_type == SHT_SYMTAB){
            symbolTablePointer = currentSectionHeader;
        }else if(currentSectionHeader->sh_type == SHT_STRTAB){
            stringTablePointer = currentSectionHeader;
        }
        
    }
    size_t symbolTableOffset = symbolTablePointer->sh_offset;
    size_t symtab_size = symbolTablePointer->sh_size;
    Elf32_Sym *symtab = (Elf32_Sym*) (s->start + symbolTableOffset);
    size_t num_symbols = symtab_size / sizeof(Elf32_Sym);
    
     // Get the offset and size of the string table
    size_t strtab_off = stringTablePointer->sh_offset;
    //size_t strtab_size = strtab_shdr->sh_size;
    char *strtab = (char*) (s->start + strtab_off);

    for (size_t j = 0; j < num_symbols; j++) {

        Elf32_Sym *sym = symtab + j;

        const char *name = strtab + sym->st_name;
        if(sym->st_value == 0)
            printf("[%d]0000000\t\t%x\t\t%s\t\t%x\t\t%s\n", j, sym->st_shndx, get_st_info_string(sym), sym->st_size, name);
        else
            printf("[%d]%x\t\t%x\t\t%s\t\t%x\t\t%s\n", j, sym->st_value, sym->st_shndx, get_st_info_string(sym), sym->st_size, name);
    }
    


}
void relocationTable(state* s){

    size_t sectionHeaderTableOffset = s->ehdr->e_shoff;
    size_t numberOfEntriesInSectionHeader = s->ehdr->e_shnum;
    size_t sectionHeaderEntrySize = s->ehdr->e_shentsize;
    
    Elf32_Shdr *symbolTablePointer = NULL;
    Elf32_Shdr *stringTablePointer = NULL;
    Elf32_Shdr *relTablePointer = NULL;
    
    size_t stringTableIndex = s->ehdr->e_shstrndx;
    Elf32_Shdr *shstrhdr = (Elf32_Shdr*)(s->start + sectionHeaderTableOffset + stringTableIndex * sectionHeaderEntrySize);
    size_t stringTableOffset = shstrhdr->sh_offset;
     for (size_t i = 0; i < numberOfEntriesInSectionHeader; i++) {
    // Get the section header
        Elf32_Shdr *currentSectionHeader = (Elf32_Shdr*)(s->start + sectionHeaderTableOffset + i * sectionHeaderEntrySize);
        char *tableName = (s->start + stringTableOffset+currentSectionHeader->sh_name);
        if(currentSectionHeader->sh_type == SHT_DYNSYM){
            symbolTablePointer = currentSectionHeader;
        }else if(strlen(tableName) == 7 && tableName[6] == 'r'){ 
            stringTablePointer = currentSectionHeader;
        
        }else if(currentSectionHeader->sh_type == SHT_REL){
            relTablePointer = currentSectionHeader;
        }
        
    }
     if(stringTablePointer == NULL){
        puts("error");
        return;
    }

   
    size_t symbolTableOffset = symbolTablePointer->sh_offset;
   // size_t symtab_size = symbolTablePointer->sh_size;
    Elf32_Sym *symtab = (Elf32_Sym*) (s->start + symbolTableOffset);
    //size_t num_symbols = symtab_size / sizeof(Elf32_Sym);
    
    size_t relTableOffset = relTablePointer->sh_offset;
    Elf32_Rel *reltab = (Elf32_Rel*) (s->start + relTableOffset);
    size_t reltab_size = relTablePointer->sh_size;
    size_t num_rel_lines = reltab_size / sizeof(Elf32_Rel);
     
     // Get the offset and size of the string table
    size_t strtab_off = stringTablePointer->sh_offset;
    //size_t strtab_size = strtab_shdr->sh_size;
    char *strtab = (char*) (s->start + strtab_off);
    
    for(size_t i = 0; i < num_rel_lines; i++){
        Elf32_Rel *line_rel = &reltab[i];
        Elf32_Addr reloc_offset = line_rel->r_offset;

        Elf32_Word sym_index = ELF32_R_SYM(line_rel->r_info);
        Elf32_Sym *sym = &symtab[sym_index];
        const char *name = strtab + sym->st_name;
        if(sym->st_value == 0)
            printf("%x\t%x\t%s\t00000000\t%s\n", reloc_offset, line_rel->r_info, r_type_to_string(ELF32_R_TYPE(line_rel->r_info)), name);
        else
            printf("%x\t%x\t%s\t%x\t%s\n", reloc_offset, line_rel->r_info, r_type_to_string(ELF32_R_TYPE(line_rel->r_info)), sym->st_value, name);
    
    }

}
 
void quit(state* s){
    if(s->currentfd > 0)
        close(s->currentfd);
    exit(0);
}

char* printTpye(Elf32_Shdr* sectionHeaderPointer){

    switch (sectionHeaderPointer->sh_type) {
        case SHT_NULL: return "SHT_NULL";
        case SHT_PROGBITS: return "SHT_PROGBITS";
        case SHT_SYMTAB: return "SHT_SYMTAB";
        case SHT_STRTAB: return "SHT_STRTAB";
        case SHT_RELA: return "SHT_RELA";
        case SHT_HASH: return "SHT_HASH";
        case SHT_DYNAMIC: return "SHT_DYNAMIC";
        case SHT_NOTE: return "SHT_NOTE";
        case SHT_NOBITS: return "SHT_NOBITS";
        case SHT_REL: return "SHT_REL";
        case SHT_SHLIB: return "SHT_SHLIB";
        case SHT_DYNSYM: return "SHT_DYNSYM";
        case SHT_INIT_ARRAY: return "SHT_INIT_ARRAY";
        case SHT_FINI_ARRAY: return "SHT_FINI_ARRAY";
        case SHT_PREINIT_ARRAY: return "SHT_PREINIT_ARRAY";
        case SHT_GROUP: return "SHT_GROUP";
        case SHT_SYMTAB_SHNDX: return "SHT_SYMTAB_SHNDX";
        case SHT_NUM: return "SHT_NUM";
        case SHT_LOOS: return "SHT_LOOS";
        case SHT_GNU_ATTRIBUTES: return "SHT_GNU_ATTRIBUTES";
        case SHT_GNU_HASH: return "SHT_GNU_HASH";
        case SHT_GNU_LIBLIST: return "SHT_GNU_LIBLIST";
        case SHT_CHECKSUM: return "SHT_CHECKSUM";
        case SHT_SUNW_move: return "SHT_SUNW_move";
        case SHT_SUNW_COMDAT: return "SHT_SUNW_COMDAT";
        case SHT_SUNW_syminfo: return "SHT_SUNW_syminfo";
        case SHT_GNU_verdef: return "SHT_GNU_verdef";
        case SHT_GNU_verneed: return "SHT_GNU_verneed";
        case SHT_GNU_versym: return "SHT_GNU_versym";
        case SHT_LOPROC: return "SHT_LOPROC";
        case SHT_HIPROC: return "SHT_HIPROC";
        case SHT_LOUSER: return "SHT_LOUSER";
        case SHT_HIUSER: return "SHT_HIUSER";
        default: return "Unknown";
    }

}
const char* get_st_info_string(Elf32_Sym *sym) {

    unsigned char st_info = sym->st_info;
    unsigned char st_type = ELF32_ST_TYPE(st_info);
    unsigned char st_bind = ELF32_ST_BIND(st_info);
    

    const char *type_str = "UNKNOWN";
    switch (st_type) {
        case STT_SECTION: type_str = "SECTION"; break;
        case STT_FILE: type_str = "FILE"; break;
        case STT_COMMON: type_str = "COMMON"; break;
        case STT_TLS: type_str = "TLS"; break;
        case STT_NUM: type_str = "NUM"; break;
        case STT_NOTYPE: type_str = "NOTYPE"; break;
        case STT_OBJECT: type_str = "OBJECT"; break;
        case STT_FUNC: type_str = "FUNC"; break;
      
    }
    
    const char *bind_str = "UNKNOWN";
    switch (st_bind) {
        case STB_WEAK: bind_str = "WEAK"; break;
        case STB_NUM: bind_str = "NUM"; break;
        case STB_LOCAL: bind_str = "LOCAL"; break;
        case STB_GLOBAL: bind_str = "GLOBAL"; break;

    }
    static char str[32];
    size_t len = 0;
    while (type_str[len] != '\0') {
        str[len] = type_str[len];
        len++;
    }
    str[len++] = '\t';
    size_t i = 0;
    while (bind_str[i] != '\0') {
        str[len + i] = bind_str[i];
        i++;
    }
    str[len + i] = '\0';
    return str;
}
char *r_type_to_string(Elf32_Word r_type){
    // Convert the relocation type to a string
    char *r_type_str;
    switch (r_type)
    {
        case R_386_GOT32:
            r_type_str = "R_386_GOT32";
            break;
        case R_386_PLT32:
            r_type_str = "R_386_PLT32";
            break;
        case R_386_TLS_GOTIE:
            r_type_str = "R_386_TLS_GOTIE";
            break;
        case R_386_TLS_LE:
            r_type_str = "R_386_TLS_LE";
            break;
        case R_386_TLS_GD:
            r_type_str = "R_386_TLS_GD";
            break;
        case R_386_TLS_LDM:
            r_type_str = "R_386_TLS_LDM";
            break;
        case R_386_16:
            r_type_str = "R_386_16";
            break;
        case R_386_NONE:
            r_type_str = "R_386_NONE";
            break;
        case R_386_32:
            r_type_str = "R_386_32";
            break;
        case R_386_COPY:
            r_type_str = "R_386_COPY";
            break;
        case R_386_GLOB_DAT:
            r_type_str = "R_386_GLOB_DAT";
            break;
        case R_386_JMP_SLOT:
            r_type_str = "R_386_JMP_SLOT";
            break;
        case R_386_RELATIVE:
            r_type_str = "R_386_RELATIVE";
            break;
        case R_386_GOTOFF:
            r_type_str = "R_386_GOTOFF";
            break;
        case R_386_GOTPC:
            r_type_str = "R_386_GOTPC";
            break;
        case R_386_32PLT:
            r_type_str = "R_386_32PLT";
            break;
        case R_386_TLS_TPOFF:
            r_type_str = "R_386_TLS_TPOFF";
            break;
        case R_386_TLS_IE:
            r_type_str = "R_386_TLS_IE";
            break;
        case R_386_PC32:
            r_type_str = "R_386_PC32";
            break;
        case R_386_PC16:
            r_type_str = "R_386_PC16";
            break;
        default:
            return NULL;
    }
    return r_type_str;
}



