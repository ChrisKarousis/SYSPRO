#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include<sys/wait.h>
#include <fcntl.h>
#include <glob.h>
#include "alias.h"

// functions for alias
// SDI1900078

void add_to_alias(struct alias_node** alias_list, char* command1, char* command2, int* count){
       // add a alias node in my struct
       struct alias_node** previous_node = alias_list;
       struct alias_node* current_node = *alias_list;
       while(current_node != NULL){ // we try to find if it already exists
              if(strcmp(current_node->name, command1) == 0){
                     strcpy(current_node->value, command2);
                     return;
              }
              previous_node = &current_node->next;
              current_node = current_node->next;
       }
       // if it doesnt exist
       struct alias_node* new_alias_node = malloc(sizeof(struct alias_node));
       new_alias_node->name = malloc(MAX_ALIAS_LEN*sizeof(char));
       new_alias_node->value = malloc(MAX_ALIAS_LEN*sizeof(char));
       strcpy(new_alias_node->name, command1);
       strcpy(new_alias_node->value, command2);
       new_alias_node->next = NULL;
       *previous_node = new_alias_node;
       *count = *count + 1;
}

void remove_alias(struct alias_node** alias_list, char* command, int* count){
       // remove a certain alias node
       struct alias_node** previous_node = alias_list;
       struct alias_node* current_node = *alias_list;
       while(current_node != NULL){
              if(strcmp(current_node->name, command) == 0){
                     *previous_node = current_node->next;
                     *count = *count - 1;
                     free(current_node->name);
                     free(current_node->value);
                     free(current_node);
                     return ;
              }
              previous_node = &current_node->next;
              current_node = current_node->next;
       }
}

void print_alias(struct alias_node* alias_list) {
    struct alias_node* current_node = alias_list;
    
    while (current_node != NULL) {
       printf("%s=%s\n", current_node->name, current_node->value);
       current_node = current_node->next;
    }
}

char* expand_alias(struct alias_node* alias_list, char* command){
       // find the proper alias to expand it, otherwise return the same command
       struct alias_node* current_node = alias_list;
       while(current_node != NULL){
              if(strcmp(current_node->name, command) == 0){
                     return current_node->value;
              }
              current_node = current_node->next;
       }
       return command; // if not found
}

void unquote(char* command){
       // remove the quotes
       int i, j;
       int len = strlen(command);
       for (i = 0, j = 0; i < len; i++) {
              if (command[i] != '"') {
                     command[j++] = command[i];
              }
       }
       command[j] = '\0';
}

void free_alias(struct alias_node* alias_list){
       // free the memory if we dont need it anymore (in the end of main)
       struct alias_node* current_node = alias_list;
       while(current_node != NULL){
              struct alias_node* next_node = current_node->next;
              free(current_node->name);
              free(current_node->value);
              free(current_node);
              current_node = next_node;
       }
}