#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <sys/mman.h>
#include <elf.h>
#include <fcntl.h>
#include <errno.h>


int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *,int), int arg);
const char *get_phdr_type_string(const Elf32_Phdr *phdr);
void print_phdr_task0(Elf32_Phdr *phdr, int index);
const char* get_phdr_flags_string(const Elf32_Phdr *phdr);
void print_phdr_readelfL(const Elf32_Phdr *phdr, int arg);//task1a
int get_protection_flags(const Elf32_Phdr* phdr);//task1b
void load_phdr(Elf32_Phdr *phdr, int fd);//task2b

size_t tableOffset = 0;
int main(int argc, char **argv) {
    
    /*char file_name[128];
    puts("Please enter the new file name");
    scanf("%s", file_name);
    */
    int fd = open(argv[1], O_RDWR);
    if (fd < 0) {
        puts("could not open the file");
        return 1;
    }
    struct stat st;
    fstat(fd, &st);

    void *map_start = mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    Elf32_Ehdr* ehdr = (Elf32_Ehdr*) map_start;
    tableOffset = ehdr->e_phoff;
    //foreach_phdr(map_start, print_phdr_task0, 0);
    //puts("Type\tOffset\tVirtAddr  PhysAddr  FileSiz  MemSiz  Flg  Align Protection flags\n");
    //foreach_phdr(map_start, print_phdr_readelfL, 0);
    
     
        
    foreach_phdr(map_start, load_phdr, fd);
    startup(argc-1, argv+1, (void *)(ehdr->e_entry));
    munmap(map_start, st.st_size);
    close(fd);
    return 0;
}



//@pre - file is opened 
//@pre - map returned void* map_start
int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *,int), int arg){
    Elf32_Ehdr* ehdr = (Elf32_Ehdr*) map_start;
    
    size_t tableOffset = ehdr->e_phoff;
    size_t numOfEntries = ehdr->e_phnum;
    //size_t entrySize = ehdr->e_phentsize;

    Elf32_Phdr* row = (Elf32_Phdr*)((char*)(map_start) + tableOffset);
    for(int i = 0; i < numOfEntries; i++){
        func(row + i, arg);
    }
    return 0;
    
}
void print_phdr_task0(Elf32_Phdr *phdr, int index) {
    printf("Program header number %d at address %p\n", index, phdr);
}
void print_phdr_readelfL(const Elf32_Phdr *phdr, int arg) {
    printf("%s\t%p\t%p\t%p\t0x%x\t0x%x\t%s\t0x%x\t%d\t%d\n", get_phdr_type_string(phdr), phdr->p_offset, 
    phdr->p_vaddr, phdr->p_paddr, phdr->p_filesz, phdr->p_memsz, get_phdr_flags_string(phdr), phdr->p_align, 
    get_protection_flags(phdr), get_map_visiability(phdr));
}
const char *get_phdr_flags_string(const Elf32_Phdr *phdr) {
    if (phdr->p_flags == PF_X) {
        return "E";
    }
    if (phdr->p_flags == PF_W) {
        return "W";
    }
    if (phdr->p_flags == PF_R) {
        return "R";
    }
    if (phdr->p_flags == PF_X + PF_W) {
        return "W E";
    }
    if (phdr->p_flags == PF_X + PF_R) {
        return "R E";
    }
    if (phdr->p_flags == PF_W + PF_R) {
        return "RW";
    }
    if (phdr->p_flags == PF_X + PF_W + PF_R) {
        return "R W E";
    }
    return "NONE";
}


const char *get_phdr_type_string(const Elf32_Phdr *phdr) {
    switch (phdr->p_type) {
        case PT_NULL:
            return "NULL";
        case PT_LOAD:
            return "LOAD";
        case PT_DYNAMIC:
            return "DYNAMIC";
        case PT_INTERP:
            return "INTERP";
        case PT_HIOS:
            return "HIOS";
        case PT_LOPROC:
            return "LOPROC";
        case PT_HIPROC:
            return "HIPROC";
        case PT_NOTE:
            return "NOTE";
        case PT_SHLIB:
            return "SHLIB";
        case PT_PHDR:
            return "PHDR";
        case PT_TLS:
            return "TLS";
        case PT_LOOS:
            return "LOOS";
        default:
            return "UNKNOWN";
    }
}
int get_protection_flags(const Elf32_Phdr* phdr) {
    int prot = 0;
    if (phdr->p_flags & PF_R) {
        prot |= PROT_READ;
    }
    if (phdr->p_flags & PF_W) {
        prot |= PROT_WRITE;
    }
    if (phdr->p_flags & PF_X) {
        prot |= PROT_EXEC;
    }
    return prot;
}
void load_phdr(Elf32_Phdr *phdr, int fd){

    if(phdr->p_type == PT_LOAD){
        print_phdr_readelfL(phdr, fd);
        void *start = mmap((void *)(phdr->p_vaddr&0xfffff000),
                                    phdr->p_memsz + (phdr->p_vaddr & 0xfff), 
                                    get_protection_flags(phdr),
                                    MAP_SHARED /*get_map_visiability(phdr)*/, 
                                    fd, 
                                    phdr->p_offset&0xfffff000 );
        if (start == MAP_FAILED) {
            perror("mmap failed");
            switch (errno) {
                case EACCES:
                  // The memory cannot be accessed. This could be because the file descriptor
                  // does not have the appropriate permissions or because the memory region
                  // is already in use.
                  perror("1");
                  break;
                case EINVAL:
                  // One or more of the arguments passed to the mmap function are invalid.
                  // This could be because the virtual address or the file size are invalid,
                  // or because the protection flags or map visibility flags are invalid.
                  perror("2");
                  break;
                case ENOMEM:
                  // There is not enough memory available to create the memory map.
                  perror("3");
                  break;
                default:
                  // An unknown error occurred.
                  break;
            }
        }
    }
}

int get_map_visiability(const Elf32_Phdr* phdr){
    int mapping = MAP_PRIVATE;
    if (phdr->p_flags & PF_W)
        mapping |= MAP_SHARED;

    return mapping;
}





