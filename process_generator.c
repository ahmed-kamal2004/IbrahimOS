#include "headers.h"
#include "string.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define PROCESS_FILE "processes.txt"
int msgq_id_ID, msgq_id_Arr, msgq_id_Run, msgq_id_Pri;
struct PCB
{
    int id;
    int arrTime;
    int priority;
    int runtime;
    struct PCB *next;
};

struct msgbuff
{
    long mtype;
    int message;
};

void clearResources(int);
struct PCB *PCB_init(int, int, int, int);
struct PCB *PCB_add(struct PCB *, struct PCB *);
struct PCB *PCB_remove(struct PCB *);
int PCB_length(struct PCB *);
void PCB_printer(struct PCB *);

int main(int argc, char *argv[])
{
    signal(SIGINT, clearResources);
    // Variables
    struct PCB *head = NULL;
    int arrvial_time, id, runtime, priority;
    char file_buffer[512];
    // TODO Initialization

    // 1. Read the input files.
    /*
        Source : https://www.programiz.com/c-programming/c-file-input-output
        Source : https://stackoverflow.com/questions/5827931/c-reading-a-multiline-text-file
    */
    FILE *fptr;
    fptr = fopen(PROCESS_FILE, "r");
    if (fptr == NULL)
    {
        printf("File doesn't exist");
        perror("File Doesn't exist");
        exit(1);
    }
    char line[512];
    while (fgets(line, 512, fptr) != NULL)
    {
        char output = sscanf(line, "%d\t%d\t%d\t%d\n", &id, &arrvial_time, &runtime, &priority);
        if (output == 0)
        {
            continue;
        }
        struct PCB *new_struct = PCB_init(id, arrvial_time, runtime, priority);
        head = PCB_add(head, new_struct);
    }
    fclose(fptr);

    // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.   DONE
    char sch_alg[10];
    int quantum = 0;
    while (strcmp(sch_alg, "RR") && strcmp(sch_alg, "SRTN") && strcmp(sch_alg, "HPF"))
    {
        printf("Choose Scheduling algorithm  RR | SRTN | HPF\n");
        scanf("%s", sch_alg);
    }
    if (!strcmp(sch_alg, "RR"))
    {
        printf("Enter Quantum for RR \n   ");
        scanf("%d", &quantum);
    }

    // 3. Initiate and create the scheduler and clock processes.  DONE
    int clk_pid, sch_pid;
    clk_pid = fork();
    if (clk_pid == -1)
    {
        perror("error in clk fork");
        exit(-1);
    }
    else if (clk_pid == 0)
    {
        execl("clk.out", "clk.out", NULL);
    }
    else
    {
        sch_pid = fork();
        if (sch_pid == -1)
        {
            perror("error in schedular fork");
            exit(-1);
        }
        else if (sch_pid == 0)
        {
            execl("scheduler.out", "scheduler.out", sch_alg, quantum, NULL); // sch_alg as argument to the code // 0 as a quantum for rr , 0 else
        }
    }

    key_t key_id_ID, key_id_Arr, key_id_Run, key_id_Pri;

    int send_val_id, send_val_arr, send_val_pri, send_val_run;

    key_id_ID = ftok("keyfile_id", 65);
    key_id_Arr = ftok("keyfile_arr", 65);
    key_id_Pri = ftok("keyfile_pri", 65);
    key_id_Run = ftok("keyfile_run", 65);

    msgq_id_ID = msgget(key_id_ID, 0666 | IPC_CREAT);
    msgq_id_Arr = msgget(key_id_Arr, 0666 | IPC_CREAT);
    msgq_id_Pri = msgget(key_id_Pri, 0666 | IPC_CREAT);
    msgq_id_Run = msgget(key_id_Run, 0666 | IPC_CREAT);

    if (msgq_id_ID == -1 || msgq_id_Run == -1 || msgq_id_Pri == -1 || msgq_id_Arr == -1)
    {
        perror("Error in create");
        exit(-1);
    }
    struct msgbuff message_id;
    struct msgbuff message_arr;
    struct msgbuff message_run;
    struct msgbuff message_pri;

    message_id.mtype = 7;
    message_arr.mtype = 7;
    message_pri.mtype = 7;
    message_run.mtype = 7;

    initClk();
    while (PCB_length(head))
    {
        while (head  != NULL && getClk() >= head->arrTime)
        {
            printf("current time is %d\n", getClk());
            message_id.message = head->id;
            message_arr.message = head->arrTime;
            message_pri.message = head->priority;
            message_run.message = head->runtime;

            send_val_id = msgsnd(msgq_id_ID, &message_id, sizeof(message_id.message), IPC_NOWAIT);
            send_val_arr = msgsnd(msgq_id_Arr, &message_arr, sizeof(message_arr.message), IPC_NOWAIT);
            send_val_pri = msgsnd(msgq_id_Pri, &message_pri, sizeof(message_pri.message), IPC_NOWAIT);
            send_val_run = msgsnd(msgq_id_Run, &message_run, sizeof(message_run.message), IPC_NOWAIT);

            if (send_val_arr == -1 || send_val_id == -1 || send_val_pri == -1 || send_val_run == -1)
            {
                perror("Error in send");
            }

            head = PCB_remove(head);
            printf("\n");
        }
    }

    int status;
    waitpid(sch_pid, &status, 0);
    destroyClk(true);
}

void clearResources(int signum)
{
    // TODO Clears all resources in case of interruption

    // should removes all message queues
    msgctl(msgq_id_Arr, IPC_RMID, (struct msqid_ds *)0);
    msgctl(msgq_id_ID, IPC_RMID, (struct msqid_ds *)0);
    msgctl(msgq_id_Pri, IPC_RMID, (struct msqid_ds *)0);
    msgctl(msgq_id_Run, IPC_RMID, (struct msqid_ds *)0);
    exit(1);
}

struct PCB *PCB_init(int id, int arrivalTime, int Runtime, int Prority)
{
    struct PCB *new_pcb = malloc(sizeof(struct PCB) * 2);
    new_pcb->arrTime = arrivalTime;
    new_pcb->id = id;
    new_pcb->priority = Prority;
    new_pcb->runtime = Runtime;
    new_pcb->next = NULL;

    return new_pcb;
}
struct PCB *PCB_add(struct PCB *head, struct PCB *new)
{
    struct PCB *looper = head;
    if (head == NULL)
        return new;
    while (looper->next != NULL)
        looper = looper->next;
    looper->next = new;
    return head;
}
struct PCB *PCB_remove(struct PCB *head)
{
    if (head == NULL)
        return NULL;
    struct PCB *new_head = head->next;
    free(head);
    return new_head;
}
int PCB_length(struct PCB *head)
{
    struct PCB *looper = head;
    int size = 0;
    while (looper != NULL)
    {
        looper = looper->next;
        size++;
    }

    return size;
}
void PCB_printer(struct PCB *head)
{
    struct PCB *looper = head;
    while (looper != NULL)
    {
        printf("%d %d %d %d \n", looper->id, looper->arrTime, looper->runtime, looper->priority);
        looper = looper->next;
    }
    return;
}