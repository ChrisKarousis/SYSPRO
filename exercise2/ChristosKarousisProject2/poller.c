#include <stdio.h>
#include <sys/wait.h> /* sockets */
#include <sys/types.h> /* sockets */
#include <sys/socket.h> /* sockets */
#include <netinet/in.h> /* internet sockets */
#include <netdb.h> /* ge th os tb ya dd r */
#include <unistd.h> /* fork */
#include <stdlib.h> /* exit */
#include <ctype.h> /* toupper */
#include <signal.h> /* signal */
#include <pthread.h>
#include <string.h>
#include <errno.h>
#define MAX_NUM_THREADS 60
#define MAX_MESSAGE 100
#define MAX_NAME_LENGTH 60
#define MAX_VOTE_LENGTH 30
#define MAX_VOTES 100
#define MAX_FILE_NAME 20

void perror_exit(char* message){
    perror(message);
    exit(EXIT_FAILURE);
}

struct voteInfo {
    char name[MAX_NAME_LENGTH];
    char vote[MAX_VOTE_LENGTH];
};

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t bufferNotEmpty;
pthread_cond_t bufferNotFull;
int sockBuffer[MAX_NUM_THREADS];
int breakPoint = 1;
struct voteInfo votearray[MAX_VOTES];
int votesCount = 0;
char poll_stats[MAX_FILE_NAME];

void add_to_struct(char* name, char* vote){
    strcpy(votearray[votesCount].name, name);
    strcpy(votearray[votesCount].vote, vote);
    votesCount++;
}

int already_exists(char* name){
    int i;
    for(i=0 ; i<votesCount ; i++){
        if(strcmp(votearray[i].name, name) == 0){
            return 1; // Name exists
        }
    }
    return 0; // Name doesnt exist
}

void signalHandler(int signal){
    if(signal == SIGINT){ // control ^C
        FILE* fp = fopen(poll_stats, "w"); // create file with votes
        if (fp == NULL) {
            perror("fopen");
        }
        int checked[votesCount], i, next=0, j, count=0;
        for(i=0 ; i<votesCount ; i++){
            checked[i] = 0;
        }
        for(i=0 ; i<votesCount ; i++){
            next++;
            count = 1;
            if(checked[i]){
                continue;
            }else{
                for(j=next ; j<votesCount ; j++){
                    if(strcmp(votearray[i].vote, votearray[j].vote) == 0){
                        checked[j] = 1;
                        count++;
                    }
                }
                fprintf(fp, "%s %d\n", votearray[i].vote, count);
            }
        }
        fprintf(fp, "%s %d\n", "TOTAL", votesCount);
        fclose(fp);
        breakPoint=0;
    }
}

void* threadFunc(void* argp){
    int i;
    int err;
    char* fileName = (char*)argp;
    while(breakPoint){
        if(err = pthread_mutex_lock(&mtx)) perror_exit("lock");
        while(sockBuffer[0] == -1){
            pthread_cond_wait(&bufferNotEmpty, &mtx); // wait till buffer is not empty
        }

        int sock = sockBuffer[0];
        for(i=1 ; i < MAX_NUM_THREADS ; i++){ 
            sockBuffer[i-1] = sockBuffer[i];
        }
        
        sockBuffer[i-1] = -1;
        pthread_cond_broadcast(&bufferNotFull);
        pthread_mutex_unlock(&mtx);
        char response1[MAX_MESSAGE];
        char response2[MAX_MESSAGE];
        char message1[MAX_MESSAGE] = "SEND NAME PLEASE";
        if (send(sock, message1,sizeof(message1), 0) < 0) {
            perror_exit("send");
        }
        if(recv(sock, response1, sizeof(response1) - 1, 0) < 0){
            perror_exit("recv");
        }
        response1[sizeof(response1) - 1] = '\0';
        if(already_exists(response1)){
            char message2[MAX_MESSAGE] = "ALREADY VOTED";
            if (send(sock, message2,sizeof(message2), 0) < 0) {
                perror_exit("send");
            }
        }else{
            char message2[MAX_MESSAGE] = "SEND VOTE PLEASE";
            if (send(sock, message2,sizeof(message2), 0) < 0) {
                perror_exit("send");
            }
            if(recv(sock, response2, sizeof(response2) - 1, 0) < 0){
                perror_exit("recv");
            }
            response2[sizeof(response2) - 1] = '\0';
            add_to_struct(response1, response2);
            FILE* fp = fopen(fileName, "a");
            if (fp == NULL) {
                perror_exit("fopen");
            }
            fprintf(fp, "%s %s\n", response1, response2);
            fclose(fp);
            char *prefix = "VOTE for Party ";
            char *postfix = " RECORDED";
            char *responseVoted = malloc((strlen(prefix)+strlen(postfix)+strlen(response2)+1)*sizeof(char));
            strcpy(responseVoted, prefix);
            strcat(responseVoted, response2);
            strcat(responseVoted, postfix);
            if (send(sock, responseVoted, strlen(responseVoted)+1, 0) < 0) {
                perror_exit("send");
            }
        }
        
        //pthread_mutex_unlock(&mtx);
        close(sock);
        
        
    }
    pthread_exit(NULL);
}


int main(int argc, char* argv[]){
    int sock, newsock, socketsCount=0, i;
    struct sigaction sa;
    sa.sa_handler = signalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror_exit("sigaction");
        return 1;
    }
    pthread_cond_init (&bufferNotEmpty , NULL ) ;
    pthread_cond_init (&bufferNotFull , NULL ) ;
    for(i=0 ; i<MAX_NUM_THREADS ; i++){
        sockBuffer[i]=-1;
    }
    struct sockaddr_in server, client;
    socklen_t clientlen = sizeof(client);
    struct sockaddr* serverptr=(struct sockaddr*)&server ;
    struct sockaddr* clientptr=(struct sockaddr*)&client ;
    struct hostent* rem ;

    if(argc != 6){
        perror_exit("number of args");
    }
    int port = atoi(argv[1]);
    int numWorkerThreads = atoi(argv[2]);
    int bufferSize = atoi(argv[3]);
    strcpy(poll_stats, argv[5]);
    if(numWorkerThreads <= 0 || bufferSize <= 0){
        perror_exit("negative args");
    }

    /* Create socket */
    if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        perror_exit("socket");

    server.sin_family = AF_INET; /* Internet domain */
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port); /* The given port */

    /* Bind socket to address */
    if(bind(sock, serverptr, sizeof(server)) < 0)
        perror_exit("bind");
    if(listen(sock , bufferSize) < 0) 
        perror_exit("listen");

    pthread_t workerThreads[numWorkerThreads];
    for(i=0; i<numWorkerThreads; i++){
        pthread_create(&workerThreads[i], NULL, threadFunc, (void *)argv[4]);
    }
    int err;
    while(breakPoint){
        if((newsock = accept(sock, clientptr, &clientlen)) < 0) 
            if (errno == EINTR) {
                continue;
            }else{
                perror_exit("accept");
            }
    
        if(err = pthread_mutex_lock(&mtx)) perror_exit("lock");
        while(sockBuffer[bufferSize-1] != -1){
            pthread_cond_wait(&bufferNotFull, &mtx);
        }
        for(i=0; i<MAX_NUM_THREADS; i++) {
            if(sockBuffer[i] == -1) {
                sockBuffer[i] = newsock;
                break;
            }
        }
        pthread_cond_broadcast(&bufferNotEmpty);
        pthread_mutex_unlock(&mtx);
        
    }
    for (i=0; i<numWorkerThreads; i++) {
        pthread_detach(workerThreads[i]);
    }

    close(sock);
    return 0;
}