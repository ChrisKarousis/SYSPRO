#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <glob.h>
#include "exec.h"
#include "alias.h"
#include "history.h"

#define MAX_FILE_LENGTH 200
#define MAX_CMD_LEN 1000
#define MAX_PATH_LEN 500
#define MAX_ARGS 20

// main function 
// SDI1900078

int typeOfInput(char* line){ // define the type of the command
       if(strstr(line, "&") != NULL){
              return 6;
       }
       if(strstr(line, "|") != NULL){
              return 5;
       }
       if(strstr(line, ">>") != NULL){
              return 4;
       }
       if(strstr(line, "<") != NULL){
              if(strstr(line, ">") != NULL){
                     return 3;
              }else{
                     return 1;
              }
       }else if(strstr(line, ">") != NULL){
              return 2;
       }
       return 0;
}

void sigchld_handler(int sig) { // collect the children that have been terminated
    int status;
    while (waitpid(-1, &status, WNOHANG) > 0);
}


void printshell(){ // print message
       printf("in-mysh-now:>");
}



int main(){
       fflush(stdout);
       int k;
       char **history = malloc(sizeof(char*)*MAX_HISTORY_COUNT);
       struct alias_node* alias_list = NULL;
       char *line = malloc(MAX_CMD_LEN*sizeof(char));
       char *command = malloc(MAX_CMD_LEN*sizeof(char));
       char *strline = malloc(MAX_CMD_LEN*sizeof(char));
       char **args = malloc(MAX_ARGS*sizeof(char*));
       char* file = malloc(MAX_FILE_LENGTH*sizeof(char));
       char* file1 = malloc(MAX_FILE_LENGTH*sizeof(char));
       char* file2 = malloc(MAX_FILE_LENGTH*sizeof(char));
       for(k=0 ; k<MAX_HISTORY_COUNT ; k++){
              history[k] = malloc(sizeof(char)*MAX_CMD_LEN);
       }
       char** pipe_tokens = malloc(sizeof(char*)*MAX_CMD_LEN);
       int history_count=0;
       int alias_count=0;
       int boolean = 0;
       signal(SIGINT, SIG_IGN);
       signal(SIGTSTP, SIG_IGN);
       while(1){
              
              glob_t glob_expansion;
              int value;
              int j;
              int i=0;
              printshell();
              fgets(line, MAX_CMD_LEN, stdin);
              strcpy(strline, line); // strtok "destroys" the string so we store it in strline
              add_to_history(history, strline, &history_count); // add the command in history
              if(strcmp(line, "exit\n") == 0){
                     break;
              }
              int type = typeOfInput(line); // get the type of input
              if(type != 5){ // if we dont have pipes
                     args[i] = strtok(line, " <>&\n");
                     strcpy(command, expand_alias(alias_list, args[i]));
                     if(strcmp(command, args[i]) != 0){ // is expanded
                            unquote(command);
                            args[i] = strtok(command, " <>&\n");
                     }
                     if(strcmp(args[i], "createalias") == 0){ // createalias
                            char* command1 = strtok(NULL, " ");
                            char* command2 = strtok(NULL, "\n");
                            add_to_alias(&alias_list, command1, command2, &alias_count);
                            continue;
                     }else if(strcmp(args[i], "destroyalias") == 0){ // destroyalias
                            char* command = strtok(NULL, " \n");
                            remove_alias(&alias_list, command, &alias_count);
                            continue;
                     }else if(strcmp(args[i], "myHistory") == 0){ // myHistory
                     
                            char* command = malloc(sizeof(char)*MAX_CMD_LEN);
                            if((command = strtok(NULL, " ")) != NULL){
                                   int cmd_number = atoi(command)-1; 
                                   strcpy(command, history[cmd_number]);
                                   args[0] = strtok(command, " <>&\n");
                                   i=0;
                            }else{
                                   print_history(history, history_count);
                                   continue;
                            }
                     }else if(strcmp(args[i], "cd") == 0){ // cd
                            i++;
                            if((args[i] = strtok(NULL, " <>\n")) != NULL){
                                   if(chdir(args[i]) != 0){
                                          perror("chdir");
                                   }
                            }else {
                                   perror("cd");
                            }
                            continue;
                     }
                     i++;
                     while((args[i] = strtok(NULL, " <>&\n")) != NULL){
                            value = glob(args[i], 0, NULL, &glob_expansion);
                            if(value == 0){   // check if it has * or ?                              
                                   for(j = 0; j<glob_expansion.gl_pathc; j++){
                                          args[i] = glob_expansion.gl_pathv[j];
                                          i++;
                                   }
                                   i--;
                            } else if(value == GLOB_NOSPACE || value == GLOB_ABORTED){
                                   perror("glob");
                                   exit(0);
                            }
                            if(strncmp(args[i], "$", 1) == 0){ // check if it is system variable
                                   memmove(args[i], args[i]+1, strlen(args[i]));
                                   char* system_variable = getenv(args[i]);
                                   if(system_variable != NULL){
                                          strcpy(args[i], system_variable);
                                   }else {
                                          perror("getenv");
                                   }
                            }
                            i++;
                     }
                     args[i] = NULL;
              }
              if(type == 0){
                     exec(args, 0, NULL, NULL, 0);
              }else if(type == 1){ // <
                     strcpy(file, args[i-1]);
                     args[i-1] = NULL; // so we dont pass the file as arg
                     exec(args, 0, file, NULL ,0);
                     
              }else if(type == 2){ // >
                     strcpy(file, args[i-1]);
                     args[i-1] = NULL;
                     exec(args, 0, NULL, file, 0);
              }else if(type == 3){ // < >
                     strcpy(file1, args[i-2]);
                     strcpy(file2, args[i-1]);
                     args[i-2] = NULL;
                     args[i-1] = NULL;
                     exec(args, 0, file1, file2,0);
              }else if(type == 4){ // >>
                     strcpy(file1, args[i-2]);
                     strcpy(file2, args[i-1]);
                     args[i-2] = NULL;
                     args[i-1] = NULL;
                     exec(args, 0, file1, file2,1);
              }else if(type == 5){ // |
                     
                     char*** args_pipe;
                     char*** pipe_files;
                     int w=0, z=0, k;
                     int i=0;
                     int j;
                     pipe_tokens[i] = strtok(line, "|\n");
                     i++;
                     // first get the commands in the pipes
                     while((pipe_tokens[i] = strtok(NULL, "|\n")) != NULL){
                            i++;
                     }
                     args_pipe = malloc(sizeof(char**)*i);
                     for(j=0 ; j<i ; j++){
                            args_pipe[j] = malloc(sizeof(char *)*MAX_ARGS);
                     }
                     // get the types of every command
                     int* pipe_types = malloc(sizeof(int)*i);
                     for(j=0 ; j<i ; j++){
                            pipe_types[j] = typeOfInput(pipe_tokens[j]);
                     }
                     int* count = malloc(sizeof(int)*i);
                     for(j = 0 ; j < i ; j++){
                            count[j] = 0;
                     }
                     for(j=0 ; j<i ; j++){ // for every pipe break it into args
                            w = 0;
                            args_pipe[j][w] = strtok(pipe_tokens[j], " <>\n");
                            w++;
                            while((args_pipe[j][w] = strtok(NULL, " <>\n")) != NULL){
                                   value = glob(args_pipe[j][w], 0, NULL, &glob_expansion);
                                   if(value == 0){ // check if it has * or ?                 
                                          for(int k = 0; k<glob_expansion.gl_pathc; k++){
                                                 args_pipe[j][w] = glob_expansion.gl_pathv[k];
                                                 w++;
                                          }
                                          w--;
                                   } else if(value == GLOB_NOSPACE || value == GLOB_ABORTED){
                                          perror("glob");
                                          exit(0);
                                   }
                                   if(strncmp(args_pipe[j][w], "$", 1) == 0){ // check if it is system variable
                                          memmove(args_pipe[j][w], args_pipe[j][w+1], strlen(args_pipe[j][w]));
                                          char* system_variable = getenv(args_pipe[j][w]);
                                          if(system_variable != NULL){
                                                 strcpy(args_pipe[j][w], system_variable);
                                          }else {
                                                 perror("getenv");
                                          }
                                   }
                                   w++;
                            }
                            count[j] = w;
                            args_pipe[j][w] = NULL;
                     }
                     pipe_files = malloc(sizeof(char**)*i);
                     // store the files of every command to redirect, if none null 
                     for(j=0 ; j<i ; j++){
                            pipe_files[j] = malloc(sizeof(char*)*2); // 2 files can be in a redirection
                     }
                     for(j = 0 ; j < i ; j++){
                            pipe_files[j][0] = malloc(sizeof(char)*MAX_FILE_LENGTH);
                            pipe_files[j][1] = malloc(sizeof(char)*MAX_FILE_LENGTH);
                     }
                     for(j = 0 ; j < i ; j++){
                            if(pipe_types[j] == 0){
                                   pipe_files[j][0]= NULL;
                                   pipe_files[j][1]= NULL;
                            }else if(pipe_types[j] == 1){ // <
                                   strcpy(pipe_files[j][0], args_pipe[j][i-1]);
                                   pipe_files[j][1] = NULL;
                                   args_pipe[j][i-1] = NULL;                              
                            }else if(pipe_types[j] == 2){ // >
                                   strcpy(pipe_files[j][1], args_pipe[j][count[j]-1]);
                                   pipe_files[j][0] = NULL;
                                   args_pipe[j][count[j]-1] = NULL;
                            }else if(pipe_types[j] == 3){ // < >
                                   strcpy(pipe_files[j][0], args_pipe[j][i-2]);
                                   strcpy(pipe_files[j][1], args_pipe[j][i-1]);
                                   args_pipe[j][i-2] = NULL;
                                   args_pipe[j][i-1] = NULL;
                            }
                     }
                     exec_pipe(args_pipe, pipe_files, i);
                     for(j = 0 ; j < i ; j++){
                            free(pipe_files[j][0]);
                            free(pipe_files[j][1]);
                     }
                     for(j=0 ; j<i ; j++){
                            free(pipe_files[j]);
                     }
                     free(pipe_files);

                     free(count);
                     free(pipe_types);

                     for(j=0 ; j<i ; j++){
                            free(args_pipe[j]);
                     }
                     free(args_pipe);
              
              }else if(type == 6){ // bg
                     char ** args_bg = malloc(sizeof(char *) * MAX_ARGS);
                     for(int j = 0 ; j < MAX_ARGS ; j++){
                            args_bg[j] = malloc(sizeof(char)*50);
                     }
                     int w=0;
                     for(int j = 0 ; j<i ; j++){
                            if(strcmp(args[j], ";") == 0){
                                   args_bg[w] = NULL;
                                   w=0;
                                   exec(args_bg, 1, NULL, NULL, 0);
                            }else {
                                   strcpy(args_bg[w], args[j]);
                                   w++;
                            }
                     }
                     if(w>0){ // if the command doesnt have ';'
                            args_bg[w] = NULL;
                            exec(args_bg, 1, NULL, NULL, 0);
                     }    
              }
              signal(SIGCHLD, sigchld_handler); // we collect terminated children
              fflush(stdout);
       }
       for(k=0 ; k<MAX_HISTORY_COUNT ; k++){
              free(history[k]);
       }
       free(history);
       free(line);
       free(command);
       free(strline);
       free(args);
       free(file);
       free(file1);
       free(file2);
       free(pipe_tokens);
       free_alias(alias_list);
       return 0;
}
