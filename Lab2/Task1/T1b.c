#include <stdio.h>

int main(int argc, char** argv){
    // iarray, iarray+1, farray, farray+1, darray, darray+1, carray and carray+1 
    int iarray[3];
    float farray[3];
    double darray[3];
    char carray[3];
    
    printf("int       :%p\n",iarray);
    printf("int    + 1:%p\n",iarray+1);
    
    
    printf("float     :%p\n",farray);
    printf("float  + 1:%p\n",farray+1);
    
    printf("double    :%p\n",darray);
    printf("double + 1:%p\n",darray+1);
    
    printf("char      :%p\n",carray);
    printf("char   + 1:%p\n",carray+1);
}