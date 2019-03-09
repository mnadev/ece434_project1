#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <string.h>

typedef
struct _ReturnVals{
	int min;
	int max;
	long sum;
} ReturnVals;

//Program will take one input parameter: The file to be read.
int main(int argc, char* argv[]){
	
	//Checking program parameters and if file exists.
	if(argc != 2){
		printf("Program usage: ./Prob2PtC <FileToBeRead>\n");
		exit(-1);
	}
	
	int inputFile = open(argv[1], O_RDONLY);	//File descriptor for file containing numbers
	if(inputFile < 0){
		printf("Fatal Error: Issue opening file '%s'\n", argv[1]);
		exit(-1);
	}
	
	int* numArray = (int*)malloc(sizeof(int)*10);
	int arraySize = 10;
	int arrayIndex = 0;
	char charIn = 0;
	char readBuf[6] = {0,0,0,0,0,0};	//Integers are at most 5 characters long (32767 for signed, 65535 for unsigned) + \0 terminator
	int readBufIndex = 0;
	
	unsigned long finalSum;
	int finalMin;
	int finalMax;
	
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
				if(arrayIndex >= arraySize){
					arraySize *= 10;
					numArray = realloc(numArray, sizeof(int)*arraySize);
				}
				numArray[arrayIndex] = inputInt;
				arrayIndex++;
				memset(readBuf, 0, 6);
				readBufIndex = 0;
				break;
			default:
				printf("Input File Error: File contains non-digit characters. Terminating program\n");
				exit(-1);
		}
	}
	close(inputFile);
	arrayIndex--; //Fixing a segfault without having to refactor all the remaining code;
	numArray = realloc(numArray, sizeof(int)*(arrayIndex+1));
	//Setting up shared memory to put array inside of:
	int sharedMemID = shmget(IPC_PRIVATE, sizeof(int)*(arrayIndex+1), IPC_CREAT|0777);	//ORed with 0777 for permission reasons.
	int* sharedArray = (int*)shmat(sharedMemID, NULL, 0);
	memcpy(sharedArray, numArray, sizeof(int)*(arrayIndex+1));
	free(numArray);
	
	//Output file to write results to
	FILE* output = fopen("Prob2PtC_Output.txt", "w");
	fprintf(output, "Hi I'm process %d and my parent is %d\n", getpid(), getppid());
	fflush(output);
	
	//Process creation
	int origPID = getpid();
	int left = 0;
	int right = arrayIndex;
	int parentAccess;	//fd containing the write end of a pipe)
	
	while(right-left > 1){
		//Creating pipe to return returnVal struct back to parent)
		int returnPipeLeft[2];
		pipe(returnPipeLeft);
		//Fork left half
		ReturnVals* retValLeft;
		int pid1 = fork();
		if(pid1){
			close(returnPipeLeft[1]);
		}else{
			right = (right+left)/2;
			close(returnPipeLeft[0]);
			parentAccess = returnPipeLeft[1];
			continue;
		}
		
		//Fork right half
		int returnPipeRight[2];
		pipe(returnPipeRight);
		ReturnVals* retValRight;
		int pid2 = fork();
		if(pid2){
			close(returnPipeRight[1]);
		}else{
			left = (right+left)/2 + 1;
			close(returnPipeRight[0]);
			parentAccess = returnPipeRight[1];
			continue;
		}
		
		if(getpid() != origPID){
			retValLeft = (ReturnVals*)malloc(sizeof(ReturnVals));
			read(returnPipeLeft[0], retValLeft, sizeof(ReturnVals));
			wait(pid1);
			close(returnPipeLeft[0]);
			
			retValRight = (ReturnVals*)malloc(sizeof(ReturnVals));
			read(returnPipeRight[0], retValRight, sizeof(ReturnVals));
			wait(pid2);
			close(returnPipeRight[0]);
			
			int min = (retValRight->min <= retValLeft->min) ? retValRight->min : retValLeft->min;
			int max = (retValRight->max >= retValLeft->max) ? retValRight->max : retValLeft->max;
			unsigned long sum = retValRight->sum + retValLeft->sum;
			ReturnVals ret = {min, max, sum};
			free(retValRight);
			free(retValLeft);
			write(parentAccess, &ret, sizeof(ret));
			fprintf(output, "Hi I'm process %d and my parent is %d\n", getpid(), getppid());
			fflush(output);
			exit(1);
		} else{
			retValLeft = (ReturnVals*)malloc(sizeof(ReturnVals));
			read(returnPipeLeft[0], retValLeft, sizeof(ReturnVals));
			wait(pid1);
			close(returnPipeLeft[0]);
			
			retValRight = (ReturnVals*)malloc(sizeof(ReturnVals));
			read(returnPipeRight[0], retValRight, sizeof(ReturnVals));
			wait(pid2);
			close(returnPipeRight[0]);
			
			finalMin = (retValRight->min <= retValLeft->min) ? retValRight->min : retValLeft->min;
			finalMax = (retValRight->max >= retValLeft->max) ? retValRight->max : retValLeft->max;
			finalSum = retValRight->sum + retValLeft->sum;
			free(retValRight);
			free(retValLeft);
			break;
		}
	}
	if(right - left <= 1){
		int tempMin = 32767;
		int tempMax = -1;
		unsigned long tempSum = 0;
		int i;
		for(i = left; i <= right; i++){
			tempMin = (tempMin < sharedArray[i]) ? tempMin : sharedArray[i];
			tempMax = (tempMax > sharedArray[i]) ? tempMax : sharedArray[i];
			tempSum += sharedArray[i];
		}
		ReturnVals baseRet = {tempMin, tempMax, tempSum};
		write(parentAccess, &baseRet, sizeof(baseRet));
		fprintf(output, "Hi I'm process %d and my parent is %d\n", getpid(), getppid());
		fflush(output);
		exit(1);
	}
	fprintf(output, "Max = %d\nMin = %d\nSum = %lu\n", finalMax, finalMin, finalSum);
	fflush(output);
	shmdt(sharedArray);
	fclose(output);
	return 0;
	
}	
