#include <stdio.h>
#include <string.h>
#include "sort.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/wait.h> 

//string array of input
char *tokens[513];

//error message for the errors we are supposed to catch
char error_message [30] = "An error has occured \n" ;





void docmd(){
	/**
	*This function runs shell commands, some that 
	*would not work if in child process	
	*/
	char temp[1024];
	if (strcmp(tokens[0], "pwd") == 0){
		getcwd(temp,1000);
		printf("%s\n",temp);
	}else if(strcmp(tokens[0], "exit") == 0){
		exit(0);
	}else if(strcmp(tokens[0], "cd") == 0){
		if(tokens[1] == NULL){
			chdir(getenv("HOME"));
		}else{
			chdir(tokens[1]);
		}
		
	}else if(strcmp(tokens[0], "show-dirs") == 0){
		DIR * pDir;
		getcwd(temp,1000);
		pDir = opendir("./");
		struct dirent *entry;
		while((entry = readdir(pDir)) != NULL){
			if(entry->d_type == DT_DIR){
				printf("%s\n",entry->d_name);
			}
		}
		closedir(pDir);
	}else if(strcmp(tokens[0], "show-files") == 0){
		DIR * pDir;
		getcwd(temp,1000);
		pDir = opendir("./");
		struct dirent *entry;
		while((entry = readdir(pDir)) != NULL){
			if(entry->d_type == DT_DIR){
				
			}else{
				printf("%s\n",entry->d_name);		
			}
		}
		closedir(pDir);
	}else if (strcmp(tokens[0],"clear")==0){
		int i =0;
		for(i=0;i<50;i++){
			printf("\n");
		}
	}else if (strcmp(tokens[0],"touch")==0){
		FILE *out;
		out = fopen(tokens[1], "w");
		fclose(out);
	}else if(strcmp(tokens[0],"mkdir")==0){
		struct stat st = {0};

		if(stat(tokens[1],&st) == -1){
			mkdir(tokens[1],0700);		
		}else{printf("%s already exists\n",tokens[1]);}
	}
}

int isbuiltin(){
	//checks if command is built in so it runs in parent process
	if (strcmp(tokens[0], "pwd") == 0){
		return 1;
	}else if(strcmp(tokens[0], "exit") == 0){
		return 1;
	}else if(strcmp(tokens[0], "cd") == 0){
		return 1;	
	}else if(strcmp(tokens[0], "show-dirs") == 0){
		return 1;
	}else if(strcmp(tokens[0], "show-files") == 0){
		return 1;
	}else if (strcmp(tokens[0],"clear")==0){
		return 1;
	}else if (strcmp(tokens[0],"touch")==0){
		return 1;
	}else if(strcmp(tokens[0],"mkdir")==0){
		return 1;
	}else if(strcmp(tokens[0],"mysh")==0){
		return 1;
	}else{
		return 0;
	}
	
}

int isPython(){
	//A function that tests if the command is python
	char *filename;
	char tempname[513];
	strcpy(tempname,tokens[0]);
	char dot[2] = ".";
	filename = strtok(tempname,dot);
	filename = strtok(NULL,dot);
	if (filename == NULL){
		return 0;
	}
	if(strcmp(filename,"py") == 0){
		return 1;	
	}else{
		return 0;
	}
}

void insertPython(){
	/**
	*inserts python into the tokens so execvp can run it
	*/
	int i;
	for(i=512;i>0;i--){
		tokens[i]= tokens[i-1];
		if(i == 1){
			tokens[0] = "python";
		}	
	}
}

int isRedirect(){
	/**
	*Redirects the output tot he file specified
	*If the file does not exist it makes one
	*If it does then it overwrites the file
	*/
	int i;
	for(i=0;i<512;i++){
		if(tokens[i] == NULL){
			if (strcmp(tokens [i-1],">")==0){
				write(STDERR_FILENO, error_message , strlen(error_message) ) ;
				tokens[i-1] = NULL;
				return 0;
			}else if(i>1 && strcmp(tokens [i-2],">")==0){
					int file;
					if (access(tokens[i-1],F_OK) != -1){
						file = remove(tokens[i-1]);
						file = open(tokens[i-1], O_APPEND | O_WRONLY| O_CREAT,S_IRWXU);
					}else{
						file = open(tokens[i-1], O_APPEND | O_WRONLY | O_CREAT,S_IRWXU);
					}
					if(file < 0){ 
						//problem opening file
						write(STDERR_FILENO, error_message , strlen(error_message) ) ;
					}else{
						if (dup2(file,1)<0){
							write(STDERR_FILENO, error_message , strlen(error_message) ) ;
						}else{
							tokens[i-1] = NULL;
							tokens[i-2] = NULL;
						}	
					}
				return 1;	
			}
			return 1;		
		}
	}
}


void tokenize(char *in){
	/**
	*splits input at spaces into tokens
	*/
	char space[3]=" ";
	int i =1;
	tokens[0] = strtok(in,space);
	while(tokens[i-1] != NULL){
		tokens[i] = strtok(NULL,space);
		if (tokens[i] != NULL){
		}
		i = i + 1;
	}
}

void getinput(char username[]){
	//takes in input and gets it ready for processing
	char *in;
	in = (char *)malloc(513 * sizeof(char));
	printf("%s> ",username);
	fgets (in, 513,stdin);

	//this verifies that the input is less than 512 characters long
	if(in[strlen (in) - 1] != '\n'){
		int dropped = 0;
		while(fgetc(stdin) != '\n'){
			dropped++;
		}
		if (dropped > 0){
			write(STDERR_FILENO, error_message , strlen(error_message) ) ;
		}		
	}
	else{
		in[strlen (in) -1] = '\0';
	}
	tokenize(in);

	
}

void getinputbatch(char username[], char * in, FILE *fp){
	if(in[strlen (in) - 1] != '\n'){
		int dropped = 0;
		while(fgetc(fp) != '\n'){
			dropped++;
		}
		if (dropped > 0){
			write(STDERR_FILENO, error_message , strlen(error_message) ) ;
		}		
	}
	else{
		in[strlen (in) -1] = '\0';
	}
	write(STDOUT_FILENO, in, strlen(in));
	write(STDOUT_FILENO,"\n",1);
	tokenize(in);
}




void mainloop(){
	/**
	*main logic of shell, decides what to do with each command
	*/
	if(tokens[0] != NULL && isbuiltin()){
		docmd();
	}else{
		int rc = fork();
		if (rc <0){
			write(STDERR_FILENO, error_message , strlen(error_message) ) ;	
		}else if(rc == 0){
			if(isPython()==1){
				insertPython();
				execvp("python",tokens);
			}else{	
				//just runs command not as python
				if(isRedirect()==1){
				execvp(tokens[0],tokens);}
				//kills child process if not a valid command
				kill(getpid(),SIGTERM);
			}
		}else{
			int wc = wait(NULL);
		}	
	}
}

void runbatch(char filename[], char username[]){
	/**	
	*reads an input file line by line and runs them as commands
	*also prints the commands being run 
	*/
	char *in;
	in = (char *)malloc(513 * sizeof(char));
	FILE * fp;
	fp = fopen (filename,"r");
	if (fp == NULL){
		write(STDERR_FILENO, error_message , strlen(error_message) ) ;
		exit(0);
	}else{	
		while(fgets(in, 513,fp)){
			getinputbatch(username,in,fp);
			mainloop();
		}
		exit(0);	
	}
}
	

int main(int argc, char *argv[]){
	char *user;
	user = getenv("USER");
	while(1){
		//checks if there is a batch command files and runs the approriate functions
		if(argc == 1){
			getinput(user);
			mainloop();
		}else if (argc == 2){
			runbatch(argv[1],user);
		}	
	}
}




