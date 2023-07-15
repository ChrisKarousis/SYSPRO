#ifndef ALIAS_NODE
#define ALIAS_NODE

#define MAX_ALIAS_LEN 200

struct alias_node {
       char* name;
       char* value;
       struct alias_node* next;
};
#endif

void add_to_alias(struct alias_node** alias_list, char* command1, char* command2, int* count);
void remove_alias(struct alias_node** alias_list, char* command, int* count);
void print_alias(struct alias_node* alias_list);
char* expand_alias(struct alias_node* alias_list, char* command);
void unquote(char* command);
void free_alias(struct alias_node* alias_list);