#include "sfish.h"
#include "sys/wait.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>

char signal_alarm_timer[1024];
struct sigaction action;

char* trimWhiteSpace(char* string, char* new_string){
	int length, start_index, end_index;
	length = strlen(string);
	start_index = 0;
	end_index = length-1;

	//Get the starting index of the string after removing all whitespace
	for(int i = 0; i < length; i++){
		if(*(string+i) == ' '){
			start_index++;
		}
		else
			break;
	}

	if(start_index == length){
		char* empty = "";
		return empty;
	}

	//Get the ending index of the string after removing all trailing whitespace
	for(int i = length - 1; i > -1; i--){
		if(*(string+i) == ' '){
			end_index--;
		}
		else
			break;
	}

	int size = end_index - start_index + 1;
    strncpy(new_string, string + start_index, size);
    memset(new_string+size, '\0', 1);
    return new_string;
}

void makePromptLine(char* buffer, char* directory_buffer){
	char* netid = "<allalee> : ";
	char* l_carrot = "<";
	char* directory = getcwd(directory_buffer, 1024);
	char* r_carrot = "> $";

	snprintf(buffer, 1024, "%s%s%s%s", netid, l_carrot, directory, r_carrot);
}

void printHelp(){
		printf("help : Print a list of all builtins and their basic usage in a single column\n"
		"exit : Exits the shell by using the exit(3) function\n"
		"cd : Changes the current working directory of the shell using the chdir(2) system call\n"
		"	* cd - should change the working directory to the last directory the user was in\n"
		"	* cd with no arguments should go the user's home directory which is stored in the HOME environment variable\n"
		"	* cd . leaves the user in the current directory\n"
		"	* cd .. moves the user up one directory\n"
		"pwd : Prints the absolute path of the current working directory\n");
}

int changeDirectory(char* arg, char* oldDirectory){
	int chdir_return = -1;
	if(arg == NULL)
		chdir_return = chdir(getenv("HOME"));
	else if(strcmp(arg, "-") == 0)
		chdir_return = chdir(oldDirectory);
	else if(strcmp(arg, ".") == 0)
		chdir_return = chdir(".");
	else if(strcmp(arg, "..") == 0)
		chdir_return = chdir("..");
	else
		chdir_return = chdir(arg);

	return chdir_return;
}

void printPWD(){
	int child_status;
	int fork_process = fork();
	if(fork_process == -1){
		printf("Error with forking a child\n");
		return;
	}
	if(fork_process == 0){
		char path[1024];
		getcwd(path, 1024);
		printf("%s\n", path);
		exit(0);
	}
	else {
		wait(&child_status);
		//printf("%d\n", child_status);
	}
}

int execute(char* arg, char* argv){
	int child_status = -1;
	char *path_variable = (char*)malloc(strlen(getenv("PATH")+1));
	strcpy(path_variable, getenv("PATH"));
	//printf("%s\n", path_variable);
	struct stat buf;
	char* exe_path;
	int exists;
	int allocated = 0;
	char* name = strtok(arg, " ");
	if(strpbrk(name, "/") == NULL){ //Search for the path and then exe
		char* path = strtok(path_variable, ":");
		while(path != NULL){
			int len = strlen(path) + 2 + strlen(name);
			char* appended_path = (char*)calloc(1, len);
			strcat(appended_path, path);
			strcat(appended_path, "/");
			strcat(appended_path, name);
			exists = stat(appended_path, &buf);
			if(exists == 0){
				exe_path = (char*) malloc(strlen(appended_path)+1);
				allocated = 1;
				strcpy(exe_path, appended_path);
				//exe_path[strlen(appended_path)-1] = '\0';
				//printf("%s\n", exe_path);
				free(appended_path);
				break;
			} else {
				free(appended_path);
				path = strtok(NULL, ":");
			}
		}
	} else { //Check via stat function and exe
		exists = stat(name, &buf);
		if(exists == 0){
			exe_path = name;
		} else {
			write(2, "File not found\n", 15);
		}
	}
	free(path_variable);
	if(exists != -1){
		char** arguments = (char**)malloc(sizeof(char*));
		*arguments = strtok(argv, " ");
		char* next_arg = strtok(NULL, " ");
		int number_of_args = 1;
		while(next_arg != NULL){
			number_of_args++;
			arguments = (char**)realloc(arguments, sizeof(char*)*number_of_args);
			arguments[number_of_args-1] = next_arg;
			next_arg = strtok(NULL, " ");
		}
		int fork_process = fork();
		if(fork_process == -1){
			printf("Error with forking a child\n");
			return -1;
		}
		if(fork_process == 0){
			number_of_args++;
			arguments = (char**)realloc(arguments, sizeof(char*)*number_of_args);
			arguments[number_of_args-1] = NULL;
			execv(exe_path, arguments);
			if(allocated == 1){
				free(exe_path);
			}
			free(arguments);
			exit(0);
		} else {
			wait(&child_status);
			if(allocated == 1){
				free(exe_path);
			}
			free(arguments);
		}
	}
	return child_status;
}

int need_redirection(char* arg){
	int redirection = 0; //0 if redirection not required, 1 otherwise
	char command[strlen(arg)];
	strcpy(command, arg);
	char* current_token = strtok(command, " ");
	while(current_token != NULL){
		if(strcmp(current_token, ">") == 0 || strcmp(current_token, "<") == 0 || strcmp(current_token, "|") == 0){
			redirection = 1;
			break;
		}
		else {
			current_token = strtok(NULL, " ");
		}
	}
	return redirection;
}

void search_for_exe(char* arg, char** exe_path, int allocated){
	char *path_variable = (char*)malloc(strlen(getenv("PATH"))+1);
	strcpy(path_variable, getenv("PATH"));
	struct stat buf;
	int exists;

	if(strpbrk(arg, "/") == NULL){ //Search for the path and then exe
		//char* execution_path;
		char* path = strtok(path_variable, ":");
		while(path != NULL){
			int len = strlen(path) + 3 + strlen(arg);
			char* appended_path = (char*)calloc(1, len);
			strcat(appended_path, path);
			strcat(appended_path, "/");
			strcat(appended_path, arg);
			exists = stat(appended_path, &buf);
			if(exists == 0){
				*exe_path = (char*) malloc(strlen(appended_path)+1);
				strcpy(*exe_path, appended_path);
				//exe_path = execution_path;
				free(appended_path);
				break;
			} else {
				free(appended_path);
				path = strtok(NULL, ":");
			}
		}
	} else { //Check via stat function and exe
		exists = stat(arg, &buf);
		if(exists == 0){
			*exe_path = arg;
		}
	}
}

int check_for_both_redirection(char* command){
	int ret = -1;
	char* left_carrot = strstr(command, "<");
	char* right_carrot = strstr(command, ">");
	if(left_carrot != NULL && right_carrot != NULL)
		ret = 0;
	return ret;
}

int redirection_execute(char* arg){
 	int child_status = -1;
 	//These two strings are must haves for any redirection
 	char* prog1_name = NULL;
 	char** prog1_args;
 	int no_prog1_args = 1;
 	int allocated = 0;
 	char* entered_command = (char*) malloc(strlen(arg));
 	strcpy(entered_command, arg);
 	char* entered_command2 = (char*) malloc(strlen(arg));
 	strcpy(entered_command2, arg);

 	int input_out = check_for_both_redirection(entered_command);
 	char* current_token = strtok(arg, " ");
 	search_for_exe(current_token, &prog1_name, allocated);
 	char* next_arg = strtok(entered_command, " ");
 	if(prog1_name == NULL){
 		write(2, "File not found\n", 15);
 		return child_status;
 	} else {
 		//printf("%s\n", prog1_name);
 		prog1_args = (char**)malloc(sizeof(char*));
 		prog1_args[0] = prog1_name;
 		next_arg = strtok(NULL, " ");
 		while(!(strcmp(next_arg, "<") == 0 || strcmp(next_arg, ">") == 0 || strcmp(next_arg, "|") == 0)){
			no_prog1_args++;
			prog1_args = (char**)realloc(prog1_args, sizeof(char*)*no_prog1_args);
			prog1_args[no_prog1_args-1] = next_arg;
			next_arg = strtok(NULL, " ");
		}
 	}

 	//At this point, we have configured the name of the program and its arguments. Now we have to handle redirection
 	//You are either piping or you are not
 	if(strcmp(next_arg, ">") == 0 || strcmp(next_arg, "<") == 0){
 		if(strcmp(next_arg, ">") == 0){
 			char* file_path = strtok(NULL, " ");
 			FILE* output = fopen(file_path, "w");
 			if(output == NULL){
 				write(2, "File is not found\n", 18);
 				return -1;
 			}
 			int output_fd = fileno(output);
 			int fork_process = fork();
 			if(fork_process == -1){
 				printf("Error with forking a child\n");
 				return -1;
 			}
 			if(fork_process==0){
 				no_prog1_args++;
 				prog1_args = (char**)realloc(prog1_args, sizeof(char*)*no_prog1_args);
 				prog1_args[no_prog1_args-1] = NULL;
 				dup2(output_fd, 1);
 				execv(prog1_name, prog1_args);
 				free(prog1_args);
 				exit(0);
 			} else {
 				wait(&child_status);
 				free(prog1_args);
 				fclose(output);
 			}
 		} else {
 	 		if(strcmp(next_arg, "<") == 0 && input_out == -1){
 				char* file_path = strtok(NULL, " ");
 				FILE* input = fopen(file_path, "r");
 				if(input == NULL){
 					write(2, "File is not found\n", 18);
 					return -1;
 				}
				int input_fd = fileno(input);
				int fork_process = fork();
				if(fork_process == -1){
					printf("Error with forking a child process\n");
					return -1;
				}
				if(fork_process == 0){
 					no_prog1_args++;
					prog1_args = (char**)realloc(prog1_args, sizeof(char*)*no_prog1_args);
					prog1_args[no_prog1_args-1] = NULL;
					dup2(input_fd, 0);
					execv(prog1_name, prog1_args);
					free(prog1_args);
					exit(0);
 				} else {
 					wait(&child_status);
 					free(prog1_args);
 					fclose(input);
 				}
 			} else {
 				char* file_path = strtok(NULL, " ");
 				FILE* input = fopen(file_path, "r");
 				while(strcmp(file_path, ">") != 0){
 					file_path = strtok(NULL, " ");
 				}
 				file_path = strtok(NULL, " ");
 				FILE* output = fopen(file_path, "w");
 				int input_fd = fileno(input);
 				int output_fd = fileno(output);
 				int fork_process = fork();
 				if(fork_process == -1){
 					printf("Error with forking a child\n");
 					return -1;
 				}
 				if(fork_process == 0){
 					no_prog1_args++;
					prog1_args = (char**)realloc(prog1_args, sizeof(char*)*no_prog1_args);
					prog1_args[no_prog1_args-1] = NULL;
					dup2(input_fd, 0);
					dup2(output_fd, 1);
					execv(prog1_name, prog1_args);
					free(prog1_args);
					exit(0);
 				} else {
 					wait(&child_status);
 					free(prog1_args);
 					fclose(input);
 					fclose(output);
 				}
 			}
 		}
 	} else{
 		//Else we are piping
 		char* prog2_name = NULL;
 		int allocated2 = 0;
 		int no_prog2_args = 1;
 		char** prog2_args;
 		current_token =  strtok(NULL, " ");
 		search_for_exe(current_token, &prog2_name, allocated2);
 		if(prog2_name == NULL){
 			write(2, "File not found\n", 15);
 			return child_status;
 		} else {
 			next_arg = strtok(entered_command2, " ");
 			for(int i = 0; i < no_prog1_args+1; i++){
 				next_arg = strtok(NULL, " ");
 			}
 			prog2_args = (char**)malloc(sizeof(char*));
 			prog2_args[0] = prog2_name;
 			next_arg = strtok(NULL, " ");
 			while(next_arg != NULL){
 				no_prog2_args++;
				prog2_args = (char**)realloc(prog2_args, sizeof(char*)*no_prog2_args);
				prog2_args[no_prog2_args-1] = next_arg;
				next_arg = strtok(NULL, " ");
 			}
 		}
 		int child1_pid = fork();
 		if(child1_pid == -1){
 			printf("Error with forking a child\n");
 			return -1;
 		}
 		if(child1_pid == 0){
 			no_prog2_args++;
 			prog2_args = (char**)realloc(prog2_args, sizeof(char*)*no_prog2_args);
 			prog2_args[no_prog2_args-1] = NULL;
 			int pipe_fd[2];
  			int pipe_return = pipe(pipe_fd);
  			printf("%d\n", pipe_return);
  			//int child_status2 = -1; //Note: We should check on the return code for this
  			int child2_pid = fork();
  			if(child2_pid == -1){
  				printf("Error with forking a child\n");
  				return -1;
  			}
  			if(child2_pid == 0){
  				close(pipe_fd[0]);
  				dup2(pipe_fd[1], 1); //Replace stdout with the pipe's write
  				close(pipe_fd[1]);
  				execv(prog1_name, prog1_args);
  				_exit(0);
  			} else {
  				close(pipe_fd[1]);
  				dup2(pipe_fd[0], 0);
  				close(pipe_fd[0]);
  				execv(prog2_name, prog2_args);
  			}
  			_exit(0);
 		} else {
 			waitpid(child1_pid, &child_status, 0);
 		}
 		printf("%s\n", prog2_name);
 	}
 	return child_status;
}

void sighandler_alarm(int sig){
	write(2, signal_alarm_timer, strlen(signal_alarm_timer));
	signal(SIGALRM, sighandler_alarm);
}

void call_alarm(char* timer){
	signal(SIGALRM, sighandler_alarm);
	int delay = atoi(timer);
	char alarm_message[1024] = "";
	char* your = "Your ";
	char* print_statement = " second timer has finished!\n";
	strcat(alarm_message, your);
	strcat(alarm_message, timer);
	strcat(alarm_message, print_statement);

	strcpy(signal_alarm_timer, alarm_message);
	alarm(delay);
	pause();
}

void sighandler_child(int sig, siginfo_t* siginfo, void *unused){
	int child_pid = siginfo->si_pid;
	double time_spent = ((double)(siginfo->si_utime + (double)(siginfo->si_stime))/(sysconf(_SC_CLK_TCK)/1000.0));
	printf("Child with PID %i has died. It spent %f seconds utilizing the CPU\n", child_pid, time_spent);
}

void sighandler_usr2(int sig){
	write(2, "Well that was easy...\n", 22);
	signal(SIGUSR2, sighandler_usr2);
}
