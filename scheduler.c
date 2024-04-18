#include "headers.h"
#include "string.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

struct msgbuff
{
    long mtype;
    int message;
};

int main(int argc, char * argv[])
{
    // initClk();
    



    key_t key_id_ID,key_id_Arr,key_id_Run,key_id_Pri;
    int msgq_id_ID,msgq_id_Arr,msgq_id_Run,msgq_id_Pri;
    int send_val_id,send_val_arr,send_val_pri,send_val_run;



    key_id_ID = ftok("keyfile_id", 65);
    key_id_Arr = ftok("keyfile_arr", 65);
    key_id_Pri = ftok("keyfile_pri", 65);
    key_id_Run = ftok("keyfile_run", 65);


    msgq_id_ID= msgget(key_id_ID, 0666 | IPC_CREAT );
        msgq_id_Arr= msgget(key_id_Arr, 0666 | IPC_CREAT );
            msgq_id_Pri= msgget(key_id_Pri, 0666 | IPC_CREAT );
                msgq_id_Run= msgget(key_id_Run, 0666 | IPC_CREAT );



    struct msgbuff message_id;
        struct msgbuff message_arr;
            struct msgbuff message_run;
                struct msgbuff message_pri;

    message_id.mtype = 7;
    message_arr.mtype = 7;
    message_pri.mtype = 7;
    message_run.mtype = 7;
    /* 0 -> receive all types of messages */
    for(int i =0;i<10;i++){
        msgrcv(msgq_id_ID, &message_id, sizeof(message_id.message), 0,!IPC_NOWAIT );
                msgrcv(msgq_id_Arr, &message_arr, sizeof(message_arr.message), 0,!IPC_NOWAIT );
                        msgrcv(msgq_id_Pri, &message_pri, sizeof(message_pri.message), 0,!IPC_NOWAIT );
                                msgrcv(msgq_id_Run, &message_run, sizeof(message_run.message), 0,!IPC_NOWAIT );

                                printf("SCH %d %d %d %d\n",message_id.message,message_arr.message,message_run.message,message_pri.message);
    }



    //TODO implement the scheduler :)
    //upon termination release the clock resources.
    
    destroyClk(true);
}
