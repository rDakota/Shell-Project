/**
 * Implementation of bash shell in c => 
 * shell displays a prompt where the user types commands.
 * After each command is entered, the shell creates
 * a child process to execute the requested program,
 * then prompts for another command once the child 
 * process has finished.
 * @author Rowan Richter
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

/*
 * Exceptions, no fork or execvp
 * - exit, pwd, cd
 * exit(0): exits the program
 * return 0: success of command
 * return -1: error in command
 * return 1: no command triggered
*/
int exceptionCommands(char *tokens[]) {
  // exit Command
  if (strcmp(tokens[0], "exit") == 0) {
    exit(0); // exit
  }
  // pwd Command
  else if (strcmp(tokens[0], "pwd") == 0) {
    char cwd[256];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
      perror("getcwd() error");
      return -1;
    }
    printf("%s\n", cwd);
    return 0; // success
  }
  // cd Command 
  else if (strcmp(tokens[0], "cd") == 0) {
    // no second command
    if (tokens[1] == NULL) {
      perror("cd error");
      return -1;
    }
    // yes second command
    // chdir returns 0 on success
    if (chdir(tokens[1]) != 0) { 
      // print an error message
      perror("Error changing directory"); 
      return -1;
    }
    // no error in chdir
    else {
      return 0; // success
    }
  }
  return 1; // no commands triggered
}

/*
 * Execvp commands and redirections
 * takes in string array of tokens from input
*/
void execvpCommands(char *tokens[]) {
  int rc = fork();
  if (rc == 0) {

    // find redirect symbol
    char *output_file = NULL;
    for (int i = 0; tokens[i] != NULL; i++) {
      if (strcmp(tokens[i], ">") == 0) {
        // filename is right after ">"
        output_file = tokens[i+1];
        // set to null so only e.g ls -l
        // is processed by execvp
        tokens[i] = NULL; 
      }
    }

    
    // if a file name has been given after ">"
    if (output_file != NULL) {
      // Close stdout, freeing its filedescriptor
      close(1);
      // redirect to outputfile
      open(output_file, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
    }

    // pass the command and its arguments to execvp
    execvp(tokens[0], tokens);
        
    // We should never reach here
    perror("execvp");
    return;
  }
  // Parent waits for child to finish
  else if (rc > 0) {
    // The basic wait system call suspends the calling
    // process until one of its children terminates
    int rc = wait(NULL);
    if (rc < 0) {
      perror("wait");
      return;
    }
  }
  else {
    perror("fork");
    return;
  }
}


/*
 * Main
*/
int main(int argc, char *argv[]) {

  char buf[1024]; 
  
  //--- Print the shell loop while true
  while (1) {
    printf("shell$ ");
    if (fgets(buf, sizeof(buf), stdin) == NULL) {
      perror("input error: ");
      return -1;
    }

    //--- Tokenizing the input
    // place tokens into a list called args
    char *args[256];
    int i = 0;
    char *token = strtok(buf, " \n");
    while (token != NULL) {
      args[i++] = token;
      token = strtok(NULL, " \n"); // next token
    }
    args[i] = NULL; // null terminated

    //--- Printing test of tokenized input
    //int len = i;
    //for(int i = 0; i < len; i++) {
    //    printf("%s\n", args[i]);
    //}

    //--- Process Commands
    // non execvp commands
    // value of 1 means no exception commands were triggered
    // in the exception commands method 
    if (exceptionCommands(args) == 1) {
      // if there are no execption commands 
      // then try execvp commands and redirections
      execvpCommands(args);
    }
  
  }
}
