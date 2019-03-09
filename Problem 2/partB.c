#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <sys/wait.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <limits.h>

int* findMinMaxSum(int * nums, int firstIndex, int lastIndex) {
	int mid = (lastIndex - firstIndex)/2;
	if(mid == 0) {
		//don't fork
		int * minMax = (int *)malloc(sizeof(int) * 3);
		minMax[0] = INT_MAX; //min
		minMax[1] = INT_MIN; //max
		minMax[2] = 0; //sum
		for(int i = firstIndex; i <= lastIndex; i++ ) {
			minMax[2] += nums[i];
			if(nums[i] < minMax[0]) {
				minMax[0] = nums[i];
			}if(nums[i] > minMax[1]) {
				minMax[1] = nums[i];
			}
		}
		//pipe back result
		return minMax;
	}
	mid = firstIndex + mid;
	int pipeFD[2];
	pipe(pipeFD);
	pid_t child = fork();
    if(child != 0) {
    	//printf("%s\n","I am the parent.");
    	int * minMax = (int *)malloc(sizeof(int) * 3);
		minMax[0] = INT_MAX; //min
		minMax[1] = INT_MIN; //max
		minMax[2] = 0; //sum
		for(int i = firstIndex; i <= mid; i++ ) {
			minMax[2] += nums[i];
			if(nums[i] < minMax[0]) {
				minMax[0] = nums[i];
			}if(nums[i] > minMax[1]) {
				minMax[1] = nums[i];
			}
		}
		int minMaxSumChild[3];
		close(pipeFD[1]);
		read(pipeFD[0],minMaxSumChild,sizeof(int)*3);
		minMax[0] = minMaxSumChild[0] ^ ((minMax[0] ^ minMaxSumChild[0]) & -(minMax[0] < minMaxSumChild[0]));
		minMax[1] = minMax[1] ^ ((minMax[1] ^ minMaxSumChild[1]) & -(minMax[1] < minMaxSumChild[1]));
		minMax[2] = minMaxSumChild[2] + minMax[2];
		wait(&child);
		return minMax;
    }else{
    	printf("Hi I'm process %d and my parent is %d.\n", getpid(), getppid());
    	int * minMaxSum = findMinMaxSum(nums, mid+1, lastIndex);
    	write(pipeFD[1],minMaxSum, sizeof(int)*3);
    }

	
}

int main(int argc, char ** argv)
{
	int MAXLENGTH = 1000;
	int AMOUNTOFNUMBERS = atoi(argv[2]);
	int sharedMemID = shmget(IPC_PRIVATE, sizeof(int) * AMOUNTOFNUMBERS, IPC_CREAT | 0666);
	printf("%d\n", sharedMemID);
	int * numbers = shmat(sharedMemID, NULL, 0);
	int * numbersT = numbers;
	char str[MAXLENGTH];
	FILE * numList = fopen(argv[1],"r");
	while(fgets(str, MAXLENGTH, numList) != NULL) {
		int num = atoi(str);
		*numbersT = num;
		numbersT+=1;
	}
	int firstIndex = 0;
	int lastIndex = AMOUNTOFNUMBERS - 1;
	printf("%d\n",lastIndex);
	int * minMaxSum = findMinMaxSum(numbers, firstIndex, lastIndex);
	printf("Min=%d\nMax=%d\nSum=%d\n",minMaxSum[0],minMaxSum[1],minMaxSum[2]);
    return 0;
}
