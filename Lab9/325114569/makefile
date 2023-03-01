all: task1 task2 task3 task4

task1: task1.o
	gcc -m32 -Wall -g -o task1 task1.o

task1.o: task1.c
	gcc -m32 -Wall -g -c -o task1.o task1.c
	
task2: task2.o
	gcc -m32 -Wall -g -o task2 task2.o

task2.o: task2.c
	gcc -m32 -Wall -g -c -o task2.o task2.c
	
task3: task3.o
	gcc -m32 -Wall -g -o task3 task3.o

task3.o: task3.c
	gcc -m32 -Wall -g -c -o task3.o task3.c
	
task4: task4.o
	gcc -m32 -Wall -g -o task4 task4.o

task4.o: task4.c
	gcc -m32 -Wall -g -c -o task4.o task4.c


.PHONY: clean

clean: 
	rm -f *.o task1
