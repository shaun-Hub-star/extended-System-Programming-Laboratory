# include <stdio.h>
# include <stdlib.h>
//for strcmp linux man
#include <string.h>
#include <ctype.h>
#define INPUT "-i"
#define DEBUG "-D"
#define OUTPUT "-o"
enum Action {Addition = 1, Subtraction = -1, NoAction = 0};

char toUpper(char letter);
int isLowerCase(char* ascii);
int isDebugMode(int argc, char ** argv);
int isInputFile(int argc, char ** argv);
int isOutputFile(int argc, char ** argv);
void printArg(FILE* err, int argc, char ** argv);
enum Action getAction(int argc, char ** argv);
const char* getActionEncryption(int argc, char ** argv);
const char* getPath(int argc, char ** argv);
const char* getGenAction(char* infix, int argc, char** argv);
const char* getOutputPath(int argc, char ** argv);
int isFlag(char* flag, int argc, char** argv);

int main(int argc, char ** argv){
	
	
	FILE* in = stdin;
	FILE* out = stdout;
	FILE* err = stderr;
	FILE* input;
	FILE* output;
	FILE* readFrom = in;
	FILE* outputTo = out;
	enum Action act;
	int length;
	const char* number;
	const char* path;
	const char* outputPath;
	char c;
	int number_idx = 0;
	int isInputFileFlag = 0;
	int isOutputFileFlag = 0;
	
	if(isOutputFile(argc,argv)){
			outputPath = getOutputPath(argc,argv);
			output = fopen(outputPath,"w");
			isOutputFileFlag = 1;
			outputTo = output;
	}
	
	act = getAction(argc, argv);
	if(act != NoAction){
		number = getActionEncryption(argc, argv);
		length = strlen(number);
		if(isInputFile(argc,argv)){
			path = getPath(argc,argv);
			input = fopen(path,"r");
			isInputFileFlag = 1;
			readFrom = input;
		}
		while( (c = fgetc(readFrom)) != EOF){
			if(c != '\n'){
				fprintf(outputTo,"%c",(number[(number_idx%length)] - 48) * act+c);
				number_idx++;
			}
			else
				number_idx = 0;
		}
	}
	
	if(isDebugMode(argc, argv)){
		printArg(err, argc, argv);
		while( (c = fgetc(in)) != EOF){
			if(c != '\n'){
				if(isLowerCase(&c)){
					fprintf(err, "%X %X\n", c, toUpper(c));
					fputc(toUpper(c),outputTo);
				}
				else{
					fprintf(err, "%X %X\n", c, c);
					fputc(c,outputTo);
				}
			}
		}
		fputc('\n',outputTo);
	}
	else{
		while( (c = fgetc(in)) != EOF){
			if(isLowerCase(&c))
				fputc(toUpper(c),outputTo);
			else
				fputc(c,outputTo);
		}
		fputc('\n',outputTo);
	}
	if(isInputFileFlag){
		fclose(input);
		free((void *)path);
	}
	if(isOutputFileFlag){
		fclose(output);
		free((void *)outputPath);
	}
	if(act != NoAction)
		free((void *)number);
	
	return 0;
}
char toUpper(char letter){
	return letter - 32;
}
int isLowerCase(char* ascii){
	return *ascii >= 'a' && *ascii <= 'z';
}
int isFlag(char* flag, int argc, char** argv){
	for(int i = 1; i < argc; i++){
		if(strncmp(argv[i],flag,2) == 0) return 1;
	}
	return 0;
}
int isDebugMode(int argc, char ** argv){
	return isFlag(DEBUG,argc,argv);
}
int isInputFile(int argc, char ** argv){
	return isFlag(INPUT,argc,argv);
}
int isOutputFile(int argc, char ** argv){
	return isFlag(OUTPUT,argc,argv);
}
void printArg(FILE* err, int argc, char ** argv){
	for(int i = 1; i < argc; i++){
		if(i == argc - 1)
			fprintf(err,"%s\n",argv[i]);
		else
			fprintf(err,"%s ",argv[i]);
	}
}
enum Action getAction(int argc, char ** argv){
	char* argument;
	for(int i = 1; i < argc; i++){
		argument = argv[i];
		if( (argument[0] == '+' || argument[0] == '-') 
			&& 
					argument[1] == 'e'){
				
			if(argument[0] == '+')
				return Addition;
			else 
				return Subtraction;
		}
	}
	return NoAction;
}
const char* getActionEncryption(int argc, char ** argv){
	char* argument;
	int j = 2;
	char *path = malloc (sizeof (char) * 100);
	for(int i = 1; i < argc; i++){
		argument = argv[i];
		if((argument[0] == '+' || argument[0] == '-') 
			&& argument[1] == 'e'){
				while(argument[j] != '\0'){
					path[j-2] = argument[j];
					j++;
				}
				path[j] = '\0';
				return path;
		}			
	}
	return path;		
}	
const char* getPath(int argc, char ** argv){
	return getGenAction(INPUT, argc, argv);
}		
const char* getOutputPath(int argc, char ** argv){
	return getGenAction(OUTPUT, argc, argv);
}	
const char* getGenAction(char* infix, int argc, char** argv){
	char* argument;
	int j = 2;
	char *path = malloc (sizeof (char) * 100);
	for(int i = 1; i < argc; i++){
		argument = argv[i];
		if(strlen(argument) >= 2 && strncmp(argument,infix,2) == 0){
				while(argument[j] != '\0'){
					path[j-2] = argument[j];
					j++;
				}
				path[j] = '\0';
				return path;
		}			
	}
	return path;

}	

