#include "sfish.h"
#include "debug.h"
#include <signal.h>

/*
 * As in previous hws the main function must be in its own file!
 */

int main(int argc, char const *argv[], char* envp[]){
    /* DO NOT MODIFY THIS. If you do you will get a ZERO. */
    rl_catch_signals = 0;
    /* This is disable readline's default signal handlers, since you are going to install your own.*/
    char *cmd;
    char readline_prompt[1024];
    char directory_buffer[1024];
    char oldDirectory[1024];;
    makePromptLine(readline_prompt, directory_buffer);

    //INSTALL THE SIGNAL HANDLER FOR SIGCHLD
    struct sigaction action;
    action.sa_sigaction = sighandler_child;
    //sigfillset (&action.sa_mask);
    action.sa_flags = SA_SIGINFO; //Need to set flag to use sigaction instead of signal
    sigaction (SIGCHLD, &action, NULL);
    //END OF THE INSTALLATION

    //INSTALL THE SIGNAL HANDLER FOR SIGUSR2
    /*struct sigaction usr_action;
    action.sa_handler = sighandler_usr2;
    sigaction (SIGUSR2, &usr_action, NULL);*/
    signal(SIGUSR2, sighandler_usr2);

    //INSTALL THE SIGPROCMASK TO BLOCK
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGTSTP);
    sigprocmask(SIG_BLOCK, &mask, NULL);
    //END OF THE INSTALLATION

    while((cmd = readline(readline_prompt)) != NULL) {
        /* All your debug print statements should use the macros found in debu.h */
        /* Use the `make debug` target in the makefile to run with these enabled. */
        info("Length of command entered: %ld\n", strlen(cmd));
        /* You WILL lose points if your shell prints out garbage values. */
        char exe_command[1024]; //We need this in the case that we are executing a program
        strcpy(exe_command, cmd);
        char* delim = " "; //Delimiter for our strtok function
        char* command = strtok(cmd, delim);
        if(command == NULL){

        }
        else if(strcmp(command, "exit") == 0 && strtok(NULL, delim) == NULL)
            break;
        else if(strcmp(command, "help") == 0 && strtok(NULL, delim) == NULL)
            printHelp();
        else if(strcmp(command, "cd") == 0){
            char* arg = strtok(NULL, delim);
            char* extra_args = strtok(NULL, delim);
            int dir;
            if(extra_args == NULL)
                dir = changeDirectory(arg, oldDirectory);
            else
                dir = -1;
            if(dir == -1)
                write(2, "Directory not found\n", 20);
            else {
                strcpy(oldDirectory, directory_buffer);
                makePromptLine(readline_prompt,directory_buffer);
            }
        }
        else if(strcmp(command, "pwd") == 0)
            printPWD();
        else if(strcmp(command, "alarm") == 0){
            char* alarm_timer = strtok(NULL, " ");
            int timer = atoi(alarm_timer);
            if(timer == 0){
                write(2, "Invalid argument for alarm\n", 27);
            } else {
                call_alarm(alarm_timer);
            }
        }
        else{
            if(need_redirection(exe_command) == 1){
                redirection_execute(exe_command);
            }else if(execute(command, exe_command) == -1)
                write(2, "Command not found\n", 18);
        }
    }

    /* Don't forget to free allocated memory, and close file descriptors. */
    free(cmd);

    return EXIT_SUCCESS;
}
