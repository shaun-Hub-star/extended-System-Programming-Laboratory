build: encoder.o
    
encoder.o: encoder.c
	gcc -m32 -Wall -g -c -o enc encoder.c
clean:
	rm -f enc encoder.o