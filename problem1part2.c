#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>

void WriteInFile (int fd, const char *buff, int len) {
    //printf("%s",buff);
    write(fd, buff, len);
}

void CopyFile (int fd, const char *file_in) {
    int in_fd = open(file_in, O_RDONLY);
    
    if(in_fd < 0) {
        printf("Failed to read %s.\n", file_in);
        return;
    }
    
    char * cha = (char *) malloc(sizeof(char));
    while(read(in_fd, cha, 1) > 0) {
        //printf("%s", cha);
	WriteInFile(fd, cha, 1);
    }
    free(cha);   
    close(in_fd);
}

int main(int argc, char** argv){
	if(argc < 4){
		printf("Error: At least two input files and one output file should be given as argument. The output file should be the last argument.\n");
		return -1;
	}
	int numFiles = argc - 1;

	char* output_name = argv[numFiles];
	int output_fd = open(output_name, O_RDWR | O_CREAT, 0777);

	if(output_fd < 0) {
		printf("Failed to open output file.\n");
		return -2;
	}

	int i;
	for(i = 0; i < numFiles - 1; i++) {
	        CopyFile(output_fd, argv[i+1]);
	}

	close(output_fd);
	return 0;

}
