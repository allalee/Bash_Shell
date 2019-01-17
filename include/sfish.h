#ifndef SFISH_H
#define SFISH_H
#include <readline/readline.h>
#include <readline/history.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <string.h>
#include <signal.h>

#endif

extern char signal_alarm_timer[1024];
extern struct sigaction child_action;

//This method trims leading and trailing whitespace and returns a substring of the string
char* trimWhiteSpace(char* string, char* new_string);

//This method builds the readline prompt that will be used for our bash
void makePromptLine(char* buffer, char* directory_buffer);

//This method will print out basic usage from typing in help
void printHelp();

//This methid will print out the absolute path of the current working directory
void printPWD();

//This method will assist in changing directory based off the string cmd
int changeDirectory(char* arg, char* oldDirectory);

//This method will execute a program within our shell
int execute(char* arg, char* argv);

//This method will test for redirection
int need_redirection(char* arg);

//This method will execute with redirection
int redirection_execute(char* arg);

//This method will search for files
void search_for_exe(char* arg, char** exe_path, int allocated);

int check_for_both_redirection(char* command);

//The method is to notify the call to alarm
void call_alarm(char* timer);

//Signal handler for SIG_ALARM
void sighandler_alarm(int sig);

//Signal handler for SIGCHLD
void sighandler_child(int sig, siginfo_t* siginfo, void *unused);

//Signal handler for SIGUSR2
void sighandler_usr2(int sig);

