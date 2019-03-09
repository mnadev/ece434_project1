#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

//Program will take one input parameter: The file to be read.
int main(int argc, char* argv[]){
	
	//Checking program parameters and if file exists.
	if(argc != 2){
		printf("Program usage: ./Prob2PtA <FileToBeRead>\n");
		exit(-1);
	}
	
	int inputFile = open(argv[1], O_RDONLY);	//File descriptor for file containing numbers
	if(inputFile < 0){
		printf("Fatal Error: Issue opening file '%s'\n", argv[1]);
		exit(-1);
	}
	
	char charIn = 0;
	char readBuf[6] = {0,0,0,0,0,0};	//Integers are at most 5 characters long (32767 for signed, 65535 for unsigned) + \0 terminator
	int readBufIndex = 0;
	
	unsigned long sum = 0;
	int min = 32767;	
	int max = -1;	//Assuming that all input integers are positive.
	
	int eofCheck;
	int inputInt;
	while((eofCheck = read(inputFile, &charIn, 1))!=0){
		switch(charIn){
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				if(readBufIndex > 4){
					printf("Error: Input file contained a non-integer number. Terminating Program\n");
					exit(-1);
				}
				readBuf[readBufIndex] = charIn;
				readBufIndex++;
				break;
			case '\n':
				inputInt = atoi(readBuf);
				if(inputInt > max){
					max = inputInt;
				} else if(inputInt < min){
					min = inputInt;
				}
				sum += inputInt;
				memset(readBuf, 0, 6);
				readBufIndex = 0;
				break;
			default:
				printf("Input File Error: File contains non-digit characters. Terminating program\n");
				exit(-1);
		}
	}
	
	//Writing to output file:
	FILE* output = fopen("Prob2PtA_Output.txt", "w");
	fprintf(output, "Hi I'm process %d and my parent is %d\n", getpid(), getppid());
	fprintf(output, "Max = %d\nMin = %d\nSum = %lu\n", max, min, sum);
	fclose(output);
	return 0;
	
}	
