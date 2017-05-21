#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <termios.h>
#include <dirent.h>

#define MAXTOKEN 256 // max number of tokens for a command
#define MAXLINE 1024 // max number of characters from user input

extern char** environ;
char* currentDirectory;
pid_t pid;
struct sigaction act_child;
struct sigaction act_int;

int standardOut;
int standardIn;

int environ_fun(char * args[]){
	char **env_show;
	for(env_show = environ; *env_show != 0; env_show ++){
		printf("%s\n", *env_show);
	}
	return 0;
 }
 
int export_fun(char * args[]){
	if((args[1] == NULL) && args[2] == NULL){
		char **env_show;
		for(env_show = environ; *env_show != 0; env_show ++){
			printf("%s\n", *env_show);
		}
		return 0;
	}
	
	if(getenv(args[1]) != NULL){
		printf("%s", "Overwrite the variable.\n");
	}else{
		printf("%s", "Create new variable.\n");
	}
	
	//set value 
	if (args[2] == NULL){
		setenv(args[1], "", 1);
	}else{
		setenv(args[1], args[2], 1);
	}
	return 0;
}
 
int unset_fun(char * args[]){
	if(args[1] == NULL){
		printf("%s","Too few arguments.\n");
		return -1;
	}
	if(getenv(args[1]) != NULL){
		unsetenv(args[1]);
		printf("%s", "Erase the variable.\n");
	}else{
		printf("%s", "The variable does not exist.\n");
	}
	return 0;
}
 

int command_fun(char * args[], bool background,bool stdin_redirect,char *input_file,bool stdout_redirect,char *output_file,bool characters_mul){
	int err = -1;
	int fileDescriptor; // between 0 and 19, describing the output or input file
	char *arg_list[MAXTOKEN];
	
	if((pid=fork())==-1){
		printf("Creat child process failed\n");
		return -1;
	}
	// pid == 0 implies the following code is related to the child process
	if(pid==0){
		// We set the child to ignore SIGINT signals (we want the parent
		// process to handle them with signalHandler_int)	
		signal(SIGINT, SIG_IGN);

		// We set parent=<pathname>/simple-c-shell as an environment variable
		// for the child
		setenv("parent",getcwd(currentDirectory, 1024),1);	

	// If we launch non-existing commands we end the process
		// 'pwd' command : prints the current directory
		if (strcmp(args[0],"pwd") == 0){
			if (stdout_redirect){
				// If we want file output
				if ( output_file != NULL ){
					fileDescriptor = open(output_file, O_CREAT | O_TRUNC | O_WRONLY, 0600); 
					//replace standard output with the appropriate file
					standardOut = dup(STDOUT_FILENO); 	//保留原本的stdout						
					dup2(fileDescriptor, STDOUT_FILENO); 
					close(fileDescriptor);
					printf("%s\n", getcwd(currentDirectory, 1024));
					
					dup2(standardOut, STDOUT_FILENO);	//還原stdout
				}
				else{
					printf("Missing name for redirect.\n");
					kill(getpid(),SIGTERM);
				}
			}else{
				printf("%s\n", getcwd(currentDirectory, 1024));
			}
		} 
		// 'clear' command clears the screen
		//else if (strcmp(args[0],"clear") == 0) system("clear");
		// 'cd' command to change directory
		//else if (strcmp(args[0],"cd") == 0) changeDirectory(args);
		// 'environ' command to list the environment variables
		else if (strcmp(args[0],"env") == 0){
			if (stdout_redirect){
				if ( output_file != NULL ){
					fileDescriptor = open(output_file, O_CREAT | O_TRUNC | O_WRONLY, 0600); 
					standardOut = dup(STDOUT_FILENO); 	
					dup2(fileDescriptor, STDOUT_FILENO); 
					close(fileDescriptor);
					environ_fun(args);
					dup2(standardOut, STDOUT_FILENO);
				}
				else{
					printf("Missing name for redirect.\n");
					kill(getpid(),SIGTERM);
				}
			}else{
				environ_fun(args);
			}
		}
		else if (strcmp(args[0],"export") == 0){
			if (stdout_redirect){
				if ( output_file != NULL ){
					fileDescriptor = open(output_file, O_CREAT | O_TRUNC | O_WRONLY, 0600); 
					standardOut = dup(STDOUT_FILENO); 	
					dup2(fileDescriptor, STDOUT_FILENO); 
					close(fileDescriptor);
					export_fun(args);
					dup2(standardOut, STDOUT_FILENO);
				}
				else{
					printf("Missing name for redirect.\n");
					kill(getpid(),SIGTERM);
				}
			}else{
				export_fun(args);
			}
		} 
		else if (strcmp(args[0],"unset") == 0){
			if (stdout_redirect){
				if ( output_file != NULL ){
					fileDescriptor = open(output_file, O_CREAT | O_TRUNC | O_WRONLY, 0600); 
					standardOut = dup(STDOUT_FILENO); 	
					dup2(fileDescriptor, STDOUT_FILENO); 
					close(fileDescriptor);
					unset_fun(args);
					dup2(standardOut, STDOUT_FILENO);
				}
				else{
					printf("Missing name for redirect.\n");
					kill(getpid(),SIGTERM);
				}
			}else{
				unset_fun(args);
			}
		} 
		else{
			//printf("QQQ???\n");
			//redirection
			if (stdout_redirect){
				if ( output_file != NULL ){
					fileDescriptor = open(output_file, O_CREAT | O_TRUNC | O_WRONLY, 0600); 
					standardOut = dup(STDOUT_FILENO); 	
					dup2(fileDescriptor, STDOUT_FILENO); 
					close(fileDescriptor);
					//unset_fun(args);
					//dup2(standardOut, STDOUT_FILENO);
				}
				else{
					printf("Missing name for redirect.\n");
					kill(getpid(),SIGTERM);
				}
			}
			if (stdin_redirect){
				if ( input_file != NULL ){
					fileDescriptor = open(input_file, O_CREAT | O_TRUNC | O_WRONLY, 0600); 
					standardIn = dup(STDIN_FILENO); 	
					dup2(fileDescriptor, STDIN_FILENO); 
					close(fileDescriptor);
					//unset_fun(args);
					//dup2(standardIn, STDIN_FILENO);
				}
				else{
					printf("Missing name for redirect.\n");
					kill(getpid(),SIGTERM);
				}
			}
			
			 /*char *arg_list[] = {
			  "ls",
			  "-l",
			  NULL };*/
			if (characters_mul){
				DIR *dir_ptr;
				struct dirent *direntp;  
				
				dir_ptr = opendir(currentDirectory);
				
				if(dir_ptr){
					int i;
					
					for(i=0;args[i]!=NULL;i++) {
						arg_list[i]=args[i];
						//printf("%d:%s\n",i,args[i]);
						//printf("orgin[%d]:%s\n",i,arg_list[i]);
					}
					/*int j=0;
					arg_list[i] = NULL;
					while(arg_list[j]!=NULL) {
						printf("%d:%s\n",j,arg_list[j]);
						j++;
					}*/
					while(direntp = readdir(dir_ptr)){
						if(!strcmp(direntp->d_name,"..") || !strcmp(direntp->d_name,".")){
							continue;
						}
						arg_list[i++] = direntp->d_name;
						//printf("%d:%s\n",i-1,arg_list[i-1]);
					}
					closedir(dir_ptr);  
					arg_list[i] = NULL;
					int j=0;
					while(arg_list[j]!=NULL) {
						printf("%d:%s\n",j,arg_list[j]);
						j++;
					}
					if(execvp(arg_list[0],arg_list)==err){
						printf("Command not found.\n");
						if (stdout_redirect){
							dup2(standardOut, STDOUT_FILENO);
						}
						if (stdin_redirect){
							dup2(standardIn, STDIN_FILENO);
						}
						kill(getpid(),SIGTERM);
					}
				}
			}
			else if(execvp(args[0],args)==err){
			//if (execvp(arg_list[0],arg_list)==err){
				printf("Command not found.\n");
				/*int i=0;
				while(args[i]!=NULL){
					printf("%d:%s\n",i,args[i]);
					i++;
				}*/
				//還原stdout、stdin
				if (stdout_redirect){
					dup2(standardOut, STDOUT_FILENO);
				}
				if (stdin_redirect){
					dup2(standardIn, STDIN_FILENO);
				}
				kill(getpid(),SIGTERM);
			}
			//還原stdout、stdin
			if (stdout_redirect){
				dup2(standardOut, STDOUT_FILENO);
			}
			if (stdin_redirect){
				dup2(standardIn, STDIN_FILENO);
			}
		}
	}

	// The following will be executed by the parent

	// If the process is not requested to be in background, we wait for
	// the child to finish.
	if (!background){
		waitpid(pid,NULL,0);
	}else{
		// In order to create a background process, the current process
		// should just skip the call to wait. The SIGCHILD handler
		// signalHandler_child will take care of the returning values
		// of the childs.
		printf("Process created with PID: %d\n",pid);
	}	 
}
 
void pipeHandler(char * args[]/*, bool background*/,bool stdin_redirect,char *input_file,bool stdout_redirect,char *output_file,bool characters_mul){
	// File descriptors
	int filedes[2]; // pos. 0 output, pos. 1 input of the pipe
	int filedes2[2];
	int fileDescriptor;
	
	int num_cmds = 0;
	
	char *command[256];
	
	pid_t pid;
	
	int err = -1;
	int end = 0;// end of command
	
	// Variables used for the different loops
	int i = 0;
	int j = 0;
	int k = 0;
	int l = 0;
	
	// First we calculate the number of commands (they are separated
	// by '|')
	while (args[l] != NULL){
		if (strcmp(args[l],"|") == 0){
			num_cmds++;
		}
		l++;
	}
	num_cmds++;
	
	// Main loop of this method. For each command between '|', the
	// pipes will be configured and standard input and/or output will
	// be replaced. Then it will be executed
	while (args[j] != NULL && end != 1){
		k = 0;
		// We use an auxiliary array of pointers to store the command
		// that will be executed on each iteration
		while (strcmp(args[j],"|") != 0){
			command[k] = args[j];
			j++;	
			if (args[j] == NULL){
				// 'end' variable used to keep the program from entering
				// again in the loop when no more arguments are found
				end = 1;
				k++;
				break;
			}
			k++;
		}
		// Last position of the command will be NULL to indicate that
		// it is its end when we pass it to the exec function
		command[k] = NULL;
		j++;		
		
		// Depending on whether we are in an iteration or another, we
		// will set different descriptors for the pipes inputs and
		// output. This way, a pipe will be shared between each two
		// iterations, enabling us to connect the inputs and outputs of
		// the two different commands.
		if (i % 2 != 0){
			pipe(filedes); // for odd i
		}else{
			pipe(filedes2); // for even i
		}
		
		pid=fork();
		
		if(pid==-1){			
			if (i != num_cmds - 1){
				if (i % 2 != 0){
					close(filedes[1]); // for odd i
				}else{
					close(filedes2[1]); // for even i
				} 
			}			
			printf("Child process could not be created\n");
			return;
		}
		if(pid==0){
			// If we are in the first command
			if (i == 0){
				if (stdin_redirect){
					if ( input_file != NULL ){
						fileDescriptor = open(input_file, O_CREAT | O_TRUNC | O_WRONLY, 0600); 
						standardIn = dup(STDIN_FILENO); 	
						dup2(fileDescriptor, STDIN_FILENO); 
						close(fileDescriptor);
						//unset_fun(args);
						//dup2(standardIn, STDIN_FILENO);
					}
					else{
						printf("Missing name for redirect.\n");
						kill(getpid(),SIGTERM);
					}
				}
				
				
				dup2(filedes2[1], STDOUT_FILENO);
			}
			// If we are in the last command, depending on whether it
			// is placed in an odd or even position, we will replace
			// the standard input for one pipe or another. The standard
			// output will be untouched because we want to see the 
			// output in the terminal
			else if (i == num_cmds - 1){
				if (stdout_redirect){
					if ( output_file != NULL ){
						fileDescriptor = open(output_file, O_CREAT | O_TRUNC | O_WRONLY, 0600); 
						standardOut = dup(STDOUT_FILENO); 	
						dup2(fileDescriptor, STDOUT_FILENO); 
						close(fileDescriptor);
						//unset_fun(args);
						//dup2(standardOut, STDOUT_FILENO);
					}
					else{
						printf("Missing name for redirect.\n");
						kill(getpid(),SIGTERM);
					}
				}
				
				
				if (num_cmds % 2 != 0){ // for odd number of commands
					dup2(filedes[0],STDIN_FILENO);
				}else{ // for even number of commands
					dup2(filedes2[0],STDIN_FILENO);
				}
			// If we are in a command that is in the middle, we will
			// have to use two pipes, one for input and another for
			// output. The position is also important in order to choose
			// which file descriptor corresponds to each input/output
			}else{ // for odd i
				if (i % 2 != 0){
					dup2(filedes2[0],STDIN_FILENO); 
					dup2(filedes[1],STDOUT_FILENO);
				}else{ // for even i
					dup2(filedes[0],STDIN_FILENO); 
					dup2(filedes2[1],STDOUT_FILENO);					
				} 
			}
			
			if (execvp(command[0],command)==err){
				kill(getpid(),SIGTERM);
			}	

			if ((i == num_cmds - 1)&&(stdout_redirect)){
				dup2(standardOut, STDOUT_FILENO);
			}
			if ((i == 0)&&(stdin_redirect)){
				dup2(standardIn, STDIN_FILENO);
			}
		}
				
		// CLOSING DESCRIPTORS ON PARENT
		if (i == 0){
			close(filedes2[1]);
		}
		else if (i == num_cmds - 1){
			if (num_cmds % 2 != 0){					
				close(filedes[0]);
			}else{					
				close(filedes2[0]);
			}
		}else{
			if (i % 2 != 0){					
				close(filedes2[0]);
				close(filedes[1]);
			}else{					
				close(filedes[0]);
				close(filedes2[1]);
			}
		}
				
		waitpid(pid,NULL,0);
				
		i++;	
	}
}
 
int commandHandler(char * args[]){
	int i = 0;
	
	int fileDescriptor;
	
	int aux;
	bool background = false;
	bool characters_mul = false;
	bool stdin_redirect = false;
	bool stdout_redirect = false;
	bool pipe_redirect = false;
	
	char *args_aux[MAXTOKEN];
	char *input_file=NULL;
	char *output_file=NULL;
	
	// We look for the special characters and separate the command itself
	// in a new array for the arguments
	int j = 0;
	for (  ; args[j] != NULL ; j++){
		if ((strcmp(args[j],"&") == 0)){
			background = true;
			continue;
		}
		else if(strcmp(args[j],"*") == 0){
			characters_mul = true;
			continue;
		}
		else if(strcmp(args[j],">") == 0){
			stdout_redirect = true;
			j++;
			output_file=args[j];
			continue;
		}
		else if(strcmp(args[j],"<") == 0){
			stdin_redirect = true;
			j++;
			input_file=args[j];
			continue;
		}
		else if(strcmp(args[j],"|") == 0){
			pipe_redirect = true;
			
		}
		args_aux[j] = args[j];
	}
	args_aux[j] = NULL;
	
	// 'exit' command quits the shell
	if(strcmp(args[0],"exit") == 0) exit(0);
	else{
		//use exec to run the  have to detect if I/O redirection,
		// piped execution or background execution were solicited

		// If background execution was solicited (last argument '&')
		// we exit the loop
		if (pipe_redirect){
			pipeHandler(args_aux/*,background*/,stdin_redirect,input_file,stdout_redirect,output_file,characters_mul);
			return 1;
		}
		else{
			// We launch the program with our method, indicating if we
			// want background execution or not
			//printf("Command:%s\n",args_aux[0]);
			command_fun(args_aux,background,stdin_redirect,input_file,stdout_redirect,output_file,characters_mul);
		}
		/**
		 * For the part 1.e, we only had to print the input that was not
		 * 'exit', 'pwd' or 'clear'. We did it the following way
		 */
		//	i = 0;
		//	while(args[i]!=NULL){
		//		printf("%s\n", args[i]);
		//		i++;
		//	}
	}
return 1;
}

int main(int argc, char *argv[], char ** envp) {
	char line[MAXLINE]; // buffer for the user input
	char * tokens[MAXTOKEN]; // array for the different tokens in the command
	int numTokens;
	
	currentDirectory = (char*) calloc(1024, sizeof(char));
	
	while(true){
		printf("shell$ ");
		fflush(NULL);
		getcwd(currentDirectory, 1024);
		
		
		fgets(line, MAXLINE, stdin);
		if((tokens[0] = strtok(line," \n\t")) == NULL) continue;
		numTokens = 1;
		while((tokens[numTokens] = strtok(NULL, " \n\t")) != NULL) numTokens++;
		
		//printf("%d:%s\n",numTokens,tokens[0]);
		commandHandler(tokens);
		
		
	}
	exit(0);
}


