// Client

#include <stdio.h>
#include <sys/types.h> /* sockets */
#include <sys/socket.h> /* sockets */
#include <netinet/in.h> /* internet sockets */
#include <unistd.h> /* read , write , close */
#include <netdb.h> /* ge th os tb ya dd r */
#include <stdlib.h> /* exit */
#include <string.h> /* strlen */
#include <pthread.h>
#define MAX_BUFFER 100
#define MAX_NAME_LENGTH 60
#define MAX_VOTE_LENGTH 60
#define MAX_NUM_THREADS 100

struct voteInfo {
    char name[MAX_NAME_LENGTH];
    char vote[MAX_VOTE_LENGTH];
};

void perror_exit(char* message){
    perror(message);
    exit(EXIT_FAILURE);
}

int port;
char serverName[100];


void* sendVoteThread(void* argp){
    struct voteInfo* voteLine = malloc(sizeof(struct voteInfo));
    memcpy(voteLine, argp, sizeof(struct voteInfo));
    int i;
    struct sockaddr_in server;
    int sock;
    struct sockaddr* serverptr=(struct sockaddr*)&server ;
    struct hostent* rem ;
    if ((sock = socket (AF_INET, SOCK_STREAM, 0)) < 0)
        perror_exit("socket");
    if ((rem = gethostbyname(serverName)) == NULL ) {
        perror_exit("gethostbyname");
    }
    server.sin_family = AF_INET ; 
    memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
    server.sin_port = htons(port) ;
    if (connect(sock, serverptr, sizeof(server)) < 0){ // connect to socket
        perror_exit("connect");
    }
    
    char buffer[MAX_BUFFER];
    if(recv(sock, buffer, sizeof(buffer), 0) < 0){ // receive message
        perror_exit("recv");
    }
    if (send(sock, voteLine->name, strlen(voteLine->name)+1, 0) < 0) { // send respone name
        perror_exit("send");
    }
    if(recv(sock, buffer, sizeof(buffer), 0) < 0){
        perror_exit("recv");
    }
    if(strcmp(buffer, "ALREADY VOTED") != 0){ // if not voted
        if (send(sock, voteLine->vote, strlen(voteLine->vote)+1, 0) < 0) { // send vote
            perror_exit("send");
        }
        char message[MAX_BUFFER];
        if(recv(sock, message, sizeof(message), 0) < 0){
            perror_exit("recv");
        }
    }
    close(sock);
    pthread_exit(NULL);
}



int main (int argc, char*argv[]) {
    int i;
    int count=0;
    FILE* fptr;
    char line[256];
    struct voteInfo voteptr[200];
    pthread_t thread[MAX_NUM_THREADS];
    int err;
    if (argc != 4) {
        printf("Wrong args!\n");
        exit(1);
    }

    strcpy(serverName, argv[1]);
    port = atoi(argv[2]);
    char* inputFile = argv[3];

    fptr = fopen(inputFile, "r");
    if(fptr == NULL){
        perror_exit("fopen");
    }
    char name[MAX_NAME_LENGTH];
    char surname[MAX_NAME_LENGTH];
    char vote[MAX_VOTE_LENGTH];
    while(fgets(line, sizeof(line), fptr)){ // iterate over file 
         if (count >= MAX_NUM_THREADS) {
            printf("Too many number of threads\n");
            break;
        }
        char* token = strtok(line, " ");
        strcpy(voteptr[count].name, token);

        token = strtok(NULL, " ");
        strcat(voteptr[count].name, " ");
        strcat(voteptr[count].name, token);
        
        token = strtok(NULL, " \n");
        strcpy(voteptr[count].vote, token);
        if(err = pthread_create(&thread[count], NULL, sendVoteThread, (void *)&voteptr[count])){ // create thread passing the vote
            perror_exit("pthread_create");
        }
        count++;
    }

    for(i=0 ; i<count ; i++){
        if(err = pthread_join(thread[i], NULL)){
            perror_exit("pthread_join");
        }
    }
    fclose(fptr);
    return 0;
}





    