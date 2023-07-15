#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include<sys/wait.h>
#include <fcntl.h>
#include <glob.h>
#include "exec.h"

// we have 2 execs : one for simple commands without pipes and one with pipes
// SDI1900078

void exec(char** args, int bg, char* file1, char* file2, int append){
       // if file1 is NULL we dont have < 
       // if file2 is NULL we dont have >
       // if append == 1
       pid_t pid = fork();
       int status, fdin, fdout;
       if(pid == -1){
              perror("fork");
              exit(1);
       }else if(pid == 0){
            if(bg == 0){ // if the process is not on background we set SIG_DFL
                signal(SIGINT, SIG_DFL);
                signal(SIGTSTP, SIG_DFL);
            }
              
              if(file1 != NULL){
                     fdin=open(file1,O_RDONLY);
                     close(0);
                     dup2(fdin,0);
                     close(fdin);
              }
              if(file2 != NULL){
                     if(append == 1){
                            fdout=open(file2,O_WRONLY|O_APPEND|O_CREAT, 0666);
                            close(1);
                            dup2(fdout,1);
                            close(fdout);
                     }else {
                            fdout=open(file2,O_WRONLY|O_TRUNC|O_CREAT, 0666);
                            close(1);
                            dup2(fdout,1);
                            close(fdout);
                     }
              }
              if(execvp(args[0], args) < 0){
                     perror("execvp");
                     exit(1);
              }
       }else {
            if(bg == 0){
                waitpid(pid, &status, WUNTRACED);
            }
            if (WIFSIGNALED(status)) {
                printf("Terminated\n");
            }else if (WIFSTOPPED(status)) {
                printf("Stopped\n");
            }
              
       }
}


void exec_pipe(char***args, char***file, int n){
    // file[i][0] gives us for the i child which file to dup2 for input
    // file[i][1] gives us for the i child which file to dup2 to write on
    // respectively args[i][0] gives us which command to execute the i child
    int fds[2];
    int i;
    int in=0, out;
    pid_t pid[n];
    int fdin, fdout;
    int status;
    for(i=0; i<n; i++) { // create n children for the n processes
        if(i<n-1) { // if not last child
            pipe(fds);
            out=fds[1];
        } else{
            out=1;
        }
        pid[i]=fork();
        if(pid[i] == -1){
            perror("fork");
            exit(1);
        }else if(pid[i]==0){
            
            if(in!=0) {
                dup2(in, 0);
                close(in);
            }
            if(out!=1) {
                dup2(out, 1);
                close(out);
                close(fds[0]);
            }
            if(file[i][0] != NULL){
                fdin=open(file[i][0], O_RDONLY);
                close(0);
                dup2(fdin,0);
                close(fdin);
            }
            if(file[i][1] != NULL){
                fdout=open(file[i][1], O_WRONLY|O_TRUNC|O_CREAT, 0666);
                close(1);
                dup2(fdout,1);
                close(fdout);    
            }
            if(execvp(args[i][0], args[i]) < 0){
                perror("execvp");
                exit(1);
            }
        }else {
            if(out!=1){  
                close(out);
            }
            if(in!=0){ 
                close(in);
            }
            in=fds[0];
        }
        
    }
    for(i=0; i<n; i++) {
        waitpid(pid[i], &status, WUNTRACED);
    }
}
