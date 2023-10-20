#include <stdio.h>
#include <stdlib.h>
#include <wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include "tokens.h"



int shell_run(char **tokens){
  char **iter = tokens;


  while(*iter != NULL){
    if(**iter == ';'){
      pid_t pid = fork();
      assert(pid != -1);

      if(pid == 0){
        *iter = NULL;
        shell_run(tokens);
        exit(0);
      }else{
        wait(NULL);
        shell_run(++iter);
        return 0;
      }
    }
  
    if(**iter == '|'){
      pid_t pid = fork();
      assert(pid != -1);

      if(pid == 0){
        int pipe_fd[2];
        assert(pipe(pipe_fd) != -1);

        pid_t pid2 = fork();

        if(pid2 == 0){
          close(1);
          dup(pipe_fd[1]);
          close(pipe_fd[0]);
          *iter = NULL;
          shell_run(tokens);
          close(pipe_fd[1]);
          exit(0);
        }else{
          close(0);
          dup(pipe_fd[0]);
          close(pipe_fd[1]);
          shell_run(++iter);
          wait(NULL);
          close(pipe_fd[0]);
          exit(0);
        }
      
      }else{
        wait(NULL);
        return 0;
      }

    }

    if(**iter == '<' || **iter == '>'){
      pid_t pid = fork();
      assert(pid != -1);
      if(pid == 0){
        assert(*(iter+1) != NULL);
        if(**iter == '<'){
          close(0);
          assert(open(*(iter+1), O_RDONLY) != -1);
        }else{
          close(1);
          assert(open(*(iter+1), O_WRONLY | O_CREAT, 0644) != -1);
        }
        *iter = NULL;
        shell_run(tokens);
        exit(0);
      }else{
        wait(NULL);
        shell_run(iter + 2);
        return 0;
      }
    } 
     

    ++iter;
  }

  pid_t pid = fork();
  assert(pid != -1);
  if(pid == 0){

    if(execvp(tokens[0], tokens) == -1){
      printf("%s: command not found\n", tokens[0]);
    } 
    exit(0);
  }else{
    wait(NULL);
  }
  return 0;
}





int main(int argc, char **argv) {
  char input[256];
  char prev[256];

  printf("Welcome to mini-shell.\n");

  while(1){
   
    if((argc > 1 && *argv[1] != '0') || argc <= 1){ 
      printf("shell $ ");
    }
    strcpy(prev, input);

    if(fgets(input, 256, stdin) == NULL){
      return 0;
    }
    char **tokens = get_tokens(input);
    assert(tokens != NULL);
   

    if(tokens[0] != NULL && strcmp(tokens[0], "exit") == 0){
      printf("Bye bye.\n");
      free_tokens(tokens);
      return 0;
    }

    if(tokens[0] != NULL && strcmp(tokens[0], "source") == 0){
      char command[256];
      command[0] = '\0';
      strcat(command, "./shell 0 < ");
      strcat(command, tokens[1]);
      printf("%s\n", command);
      char **source_tokens = get_tokens(command); 
      shell_run(source_tokens);
      free_tokens(source_tokens);
    }

    if(tokens[0] != NULL && strcmp(tokens[0], "cd") == 0){
      assert(tokens[1] != NULL);
      chdir(tokens[1]);
    }

    if(tokens[0] != NULL && strcmp(tokens[0], "prev") == 0){
      free_tokens(tokens);
      tokens = get_tokens(prev);
      shell_run(tokens);
      strcpy(input, prev);
    
    }

    else if(tokens[0] != NULL){
      shell_run(tokens);      
    }

    free_tokens(tokens);
  
  }
   
  return 0;
}

