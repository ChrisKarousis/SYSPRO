#ifndef HISTORY
#define HISTORY

#define MAX_HISTORY_COUNT 20

#endif 

void add_to_history(char** history, char* command, int *count);
void print_history(char** history, int count);