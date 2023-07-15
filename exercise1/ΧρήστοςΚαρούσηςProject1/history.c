#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include<sys/wait.h>
#include <fcntl.h>
#include "history.h"

// we control the char** history with 2 functions add and print
// SDI1900078

void add_to_history(char** history, char* command, int *count){
       int i;
       if(strncmp(command, "myHistory", strlen("myHistory")) == 0){
              return;
       }
       if((*count) < MAX_HISTORY_COUNT){
              strcpy(history[*count], command);
              *count = *count+1;
       }else {
              for(i=0 ; i<MAX_HISTORY_COUNT-1 ; i++){
                     strcpy(history[i], history[i+1]);
              }
              strcpy(history[MAX_HISTORY_COUNT-1], command);
       }
}

void print_history(char** history, int count){
       int i;
       for(i = 0; i < count; i++) {
              printf("%d. %s", i+1, history[i]);
       }
}