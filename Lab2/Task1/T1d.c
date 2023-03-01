#include <stdio.h>

int main(int argc, char** argv){
  int iarray[] = {1,2,3};
  char carray[] = {'a','b','c'};
  int* iarrayPtr;
  char* carrayPtr;
  //print:
  
  iarrayPtr = iarray;
  carrayPtr = carray;
  for(int i = 0;i<3;i++){
    printf("iarray[%d] = %d\n",i,*iarrayPtr++);
  }
   printf("%c",'\n');
  for(int i = 0;i<3;i++){
    printf("carray[%d] = %c\n",i,*carrayPtr++);
  }
  int* p;
  printf("un initialized pointer p located -> %p\n",p);
    
}