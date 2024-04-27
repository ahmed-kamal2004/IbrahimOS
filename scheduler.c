//#include "headers.h"
#include "string.h"
#include "DataStructure.h"
#include "Algorithms.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

struct message{
    int id;
    int arrTime;
    int RunTime;
    int Priority;
    pid_t pid;
};
struct msgbuff
{
    long mtype;
    struct message msg;
};
PData *currentprocess=NULL;
bool RunSRTN(struct PPQueue *queue, struct PData *data);
bool RunHPF(struct PPQueue *queue, struct PData *data);
bool RunRR(struct PPQueue *queue, struct PData *data);
int x,y;
int main(int argc, char * argv[])
{


    // try semaphore





    initClk();
    int quantum=atoi(argv[2]);
    char *Algorithm = malloc(strlen(argv[1]) + 1);
    strcpy(Algorithm, argv[1]);
    enum Algo chosen_algo;
    struct PPQueue * HPFqueue;
    struct PPQueue * SRTNqueue;
    x=getClk();
    if(Algorithm=="HPF")
    {
        chosen_algo=HPF;
        HPFqueue=createPQ();
    }
    else if(Algorithm=="SRTN")
    {
        chosen_algo=SRTN;
        SRTNqueue=createPQ();
    }
    else if(Algorithm=="RR")
    {
        chosen_algo=RR;
    }
    else{
        printf("error in algo");
    }

    printf("Scheduler : %d\n\n",getpid());

    int num_of_process = 0;

    key_t key_id;
    int msgq_id;
    int rec_val;

    key_id= ftok("keyFile", 65);


    msgq_id= msgget(key_id, 0666 | IPC_CREAT );


    struct msgbuff msg;

    msg.mtype = 7;

    struct msqid_ds ctrl_status_ds;
    pid_t pid;
    while(true){
        msgctl(msgq_id, IPC_STAT, &ctrl_status_ds);
        while(ctrl_status_ds.__msg_cbytes){
            rec_val=msgrcv(msgq_id, &msg, sizeof(msg.msg), 0,IPC_NOWAIT );
            if(rec_val != -1)
            {
                num_of_process++;
                printf("SCH %d %d %d %d  Time %d\n",msg.msg.id,msg.msg.arrTime,msg.msg.RunTime,msg.msg.Priority,getClk());
                struct PData pdata;
                pdata.id=msg.msg.id;
                pdata.arrivaltime=msg.msg.arrTime;
                pdata.runningtime=msg.msg.RunTime;
                pdata.priority=msg.msg.Priority;
                pdata.remaintime=msg.msg.RunTime;
                pdata.state=arrived;
                pid=fork();
                if(pid==-1)
                {
                    printf("error in forking");
                }
                else if(pid==0)
                {
                    msg.msg.pid=getpid();
                    pdata.pid=getpid();
                    execl("process.c","process.c",msg.msg.RunTime,NULL);
                }
                kill(pdata.pid,SIGSTOP);
                if(chosen_algo==SRTN)
                {
                    enqueuePQ(pdata,pdata.remaintime,SRTNqueue);
                }
                else if(chosen_algo==HPF){
                    enqueuePQ(pdata,pdata.priority,HPFqueue);
                }
                else{
                    printf("ahmed mostafa attia ");
                }
            }
            msgctl(msgq_id, IPC_STAT, &ctrl_status_ds);
        }
        if(chosen_algo==SRTN)
        {
           RunSRTN(SRTNqueue,currentprocess);
        }
        else if(chosen_algo==HPF){
           RunSRTN(HPFqueue,currentprocess);
        }
        else{
            
        }
    }
    //TODO implement the scheduler :)
    //upon termination release the clock resources.
    
    destroyClk(true);
}
bool RunSRTN(struct PPQueue *queue, struct PData *currentprocess)
{
    if(currentprocess!=NULL)
    {
        y=getClk();
        if(y!=x)
        {
            currentprocess->remaintime--;
            x=y;
            if(currentprocess->remaintime==0)
            {
                free(currentprocess);
                return true;
            }
        }
        struct PData *fprocess;
        if(frontPQ(queue,fprocess))
        {
            if(currentprocess->remaintime<=fprocess)
            {
                return true;
            }
            else{
                kill(currentprocess->pid,SIGSTOP);
                kill(fprocess->pid,SIGCONT);
                dequeuePQ(queue);
                enqueuePQ(*currentprocess,currentprocess->remaintime,queue);
                currentprocess=fprocess;
                return true;
            }
        }
        else
        {
            return true;
        }
        
    }
    else
    {
        if(!frontPQ(queue,currentprocess))
        {
           return false;
        }
        else{
            kill(currentprocess->pid,SIGCONT);
            return true;
        }
    }

}
bool RunHPF(struct PPQueue *queue, struct PData *currentprocess)
{
    if(currentprocess!=NULL)
    {
        y=getClk();
        if(y!=x)
        {
            currentprocess->remaintime--;
            x=y;
            if(currentprocess->remaintime==0)
            {
                free(currentprocess);
                return true;
            }
        }
    }
    else
    {
        if(!frontPQ(queue,currentprocess))
        {
           return false;
        }
        else
        {
            kill(currentprocess->pid,SIGCONT);
            return true;
        }
    }
}
bool RunRR(struct PPQueue *queue, struct PData *data)
{

}