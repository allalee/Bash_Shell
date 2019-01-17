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
    //char oldDirectory[1024];
    makePromptLine(readline_prompt, directory_buffer);

    while((cmd = readline(readline_prompt)) != NULL) {
        char new_cmd[1024];
        char* prompt = trimWhiteSpace(cmd, new_cmd);
        if(strcmp(prompt, "exit") == 0)
            break;
        if(strncmp(prompt, "help", 4) == 0){
            printHelp(prompt);
        }

        printf("%s\n", prompt);
        printf("%s\n",cmd);
        /* All your debug print statements should use the macros found in debu.h */
        /* Use the `make debug` target in the makefile to run with these enabled. */
        info("Length of command entered: %ld\n", strlen(cmd));
        /* You WILL lose points if your shell prints out garbage values. */
    }

    /* Don't forget to free allocated memory, and close file descriptors. */
    free(cmd);

    return EXIT_SUCCESS;
}
