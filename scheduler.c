#include "headers.h"
#include "string.h"
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
};
struct msgbuff
{
    long mtype;
    struct message msg;
};


int main(int argc, char * argv[])
{


    // try semaphore





    initClk();


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

    while(num_of_process < 10){
        msgctl(msgq_id, IPC_STAT, &ctrl_status_ds);
        while(ctrl_status_ds.__msg_cbytes){
            rec_val=msgrcv(msgq_id, &msg, sizeof(msg.msg), 0,IPC_NOWAIT );
            if(rec_val != -1)
            {
                num_of_process++;
                printf("SCH %d %d %d %d  Time %d\n",msg.msg.id,msg.msg.arrTime,msg.msg.RunTime,msg.msg.Priority,getClk());
            }
            msgctl(msgq_id, IPC_STAT, &ctrl_status_ds);
        }
        
    }



    //TODO implement the scheduler :)
    //upon termination release the clock resources.
    
    destroyClk(true);
}
