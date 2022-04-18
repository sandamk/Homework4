#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<time.h>
#include<signal.h>
#include<errno.h>
#include<sys/wait.h>
#include"queue.h"

#define MAX_COMMAND_LEN 100
#define MAX_JOBS 100
#define CORES 8

enum STATUS {RUNNING, WAITING, SUCCESS, FAILED};
//this structure part is done by both sai keerthana andam and rahul yadav eddu
struct JOB{
    int id;
    char command[MAX_COMMAND_LEN];
    enum STATUS status;
    time_t starttime;
    time_t endtime;
    pid_t pid;
} *alljobs[MAX_JOBS];
struct JOB *runningjobs[CORES];
struct JOB completedjobs[MAX_JOBS];
int filled = -1, completed = -1, P = -1;
queue *waitingjobs;
pid_t childid = -1;

//there is a submithistory() part done by rahul yadav eddu (which can be seen in schedule.c)

void showjobs(queue *waitingjobs, struct JOB *runningjobs[], int P){
    printf("%2s %30s %30s\n", "jobid", "command", "status");
    char status[20];
    for(int i=waitingjobs->start;i<waitingjobs->end;i++){
        strcpy(status, "Waiting");
        printf("%2d %30s %30s\n", alljobs[waitingjobs->buffer[i]]->id, alljobs[waitingjobs->buffer[i]]->command, status);
    }
    for(int i=0;i<P;i++){
        strcpy(status, "Running");
        if(runningjobs[i] != NULL){
            printf("%2d %30s %30s\n", runningjobs[i]->id, runningjobs[i]->command, status);
        }
    }
}
//there is startprocess() function done by rahul yadav eddu.

void child_handler(int sig) {
    pid_t p;
    int status;

    while (1) {
       /* retrieve child process ID (if any) */
       p = waitpid(-1, &status, WNOHANG);

       /* check for conditions causing the loop to terminate */
        if (p == -1) {
            if (errno == EINTR) {continue;}
            break;
        }else if(p == 0) {break;}

        for(int i=0;i<P;i++){
            if(runningjobs[i] != NULL && runningjobs[i]->pid == p){
                runningjobs[i]->endtime = time(NULL);
                runningjobs[i]->status = SUCCESS;
                completedjobs[++completed] = *runningjobs[i];
                runningjobs[i] = NULL;
                break;
            }
        }
        int jobid = queue_delete(waitingjobs);
        if(jobid == -1) continue;
        struct JOB *jobtorun = alljobs[jobid];
        alljobs[jobid] = NULL;
        startprocess(jobtorun);
    }   
}

//main function here is done by both after completion of all functions.

int main(int argc, char *argv[]){
    if(argc < 2){
        printf("USAGE: ./scheduler P\n");
        exit(0);
    }
    signal(SIGCHLD, child_handler);
    P = atoi(argv[1]);
    if(P > CORES){
        P = CORES;
    }
    char command[MAX_COMMAND_LEN];
    waitingjobs = queue_init(MAX_JOBS);
    for(int i=0;i<P;i++){
        runningjobs[i] = NULL;
    }
    while(1){
        printf("\rEnter command> ");
        bzero(command, MAX_COMMAND_LEN);
        fgets(command, MAX_COMMAND_LEN, stdin);
        // if(command[strlen(command)-1] == '\n')
        command[strlen(command)-1] = '\0';
        
        //this submithistory part is done by rahul yadav eddu
        if(strncmp(command, "submithistory", 13) == 0){ 
            submithistory(completedjobs);
        }else if(strncmp(command, "submit", 6) == 0){
            struct JOB *t = malloc(sizeof(struct JOB));
            strncpy(t->command, &command[7], strlen(command) - 7);
            filled++;
            t->id = filled;
            if(startprocess(t)) continue;
            for(int i=0;i<MAX_JOBS;i++){
                if(alljobs[i] == NULL){
                    alljobs[i] = t;
                    queue_insert(waitingjobs, i);
                    printf("Job %d added to the queue\n", t->id);
                    break;
                }
            }
	//this show jobs part is done by sai keerthana andam

        }else if(strncmp(command, "showjobs", 8) == 0){
            showjobs(waitingjobs, runningjobs, P);
        }else if(strncmp(command, "exit", 4) == 0){
            printf("Exiting. Bye!\n");
            exit(0);
        }else{
            if(strlen(command) > 0)
                printf("Unknown Command! \n");
        }
    }

    return 0;
}