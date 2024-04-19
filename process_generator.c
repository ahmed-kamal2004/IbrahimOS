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

int msgq_id;
struct PCB *head = NULL;

struct PCB
{
    int id;
    int arrTime;
    int priority;
    int runtime;
    struct PCB *next;
};

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



void clearResources(int);
struct PCB *PCB_init(int, int, int, int);
struct PCB *PCB_add(struct PCB *, struct PCB *);
struct PCB *PCB_remove(struct PCB *);
int PCB_length(struct PCB *);
void PCB_printer(struct PCB *);
struct PCB*File_Reader();
void Sch_algorithm(char [],int*);



int main(int argc, char *argv[])
{
    signal(SIGINT, clearResources);

    // 1. Read the input files.
    /*
        Source : https://www.programiz.com/c-programming/c-file-input-output
        Source : https://stackoverflow.com/questions/5827931/c-reading-a-multiline-text-file
    */

    head = File_Reader();
    PCB_printer(head);


    // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.   DONE
    char sch_alg[10];
    int quantum = 0;
    Sch_algorithm(sch_alg,&quantum);




    // 3. Initiate and create the scheduler and clock processes.  DONE
    int clk_pid, sch_pid;
    clk_pid = fork();
    if (clk_pid == -1)
    {
        perror("error in clk fork");
        exit(-1);
    }
    if (clk_pid == 0)
    {
        execl("clk.out", "clk.out", NULL);
    }
    sch_pid = fork();
    printf("Sch_pid : %d\n",sch_pid);
    if (sch_pid == -1)
    {
            perror("error in schedular fork");
            exit(-1);
    }
    if (sch_pid == 0)
    {
        printf("scheduler.out\n");
        execl("scheduler.out", "scheduler.out", sch_alg, quantum, NULL); // sch_alg as argument to the code // 0 as a quantum for rr , 0 else
    }

    printf("Generator : %d\n\n",getpid());

    // 4 - sending the number of processes

    initClk();

    key_t key_id;

    int send_val;

    key_id = ftok("keyFile", 65);


    msgq_id= msgget(key_id, 0666 | IPC_CREAT);

    struct msqid_ds ctrl_status_ds;
    msgctl(msgq_id, IPC_STAT, &ctrl_status_ds);
    ctrl_status_ds.msg_qbytes *= 4;
    msgctl(msgq_id,IPC_SET,&ctrl_status_ds);


    if (msgq_id == -1)
    {
        perror("Error in create");
        exit(-1);
    }
    struct msgbuff msg;

    msg.mtype = 7;

    while (PCB_length(head))
    {
        while (head  != NULL && getClk() == head->arrTime)
        {
            msg.msg.id = head->id;
            msg.msg.arrTime = head->arrTime;
            msg.msg.Priority = head->priority;
            msg.msg.RunTime = head->runtime;
            send_val= msgsnd(msgq_id, &msg, sizeof(msg.msg), IPC_NOWAIT);
            if (send_val == -1 )
            {
                perror("Error in send");
            }
            head = PCB_remove(head);
            printf("\n");
            printf("current time is %d %d\n", getClk(),PCB_length(head));
        }
    }
    int status;
    wait(&status);
    msgctl(msgq_id, IPC_RMID, (struct msqid_ds *)0);
    destroyClk(true);
    exit(1);
}

void clearResources(int signum)
{
    // TODO Clears all resources in case of interruption
    msgctl(msgq_id, IPC_RMID, (struct msqid_ds *)0);
    while(PCB_remove(head));
    destroyClk(true);
    raise(SIGKILL);
    exit(-1);
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
    printf("NULL\n");
    return;
}
struct PCB* File_Reader(){

    struct PCB *head = NULL;
    FILE *fptr;
    int arrvial_time, id, runtime, priority;
    char file_buffer[512];
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
    return head;
}
void Sch_algorithm(char sch_alg[],int* quantum){
    while (strcmp(sch_alg, "RR") && strcmp(sch_alg, "SRTN") && strcmp(sch_alg, "HPF"))
    {
        printf("Choose Scheduling algorithm  RR | SRTN | HPF\n");
        scanf("%s", sch_alg);
    }
    if (!strcmp(sch_alg, "RR"))
    {
        printf("Enter Quantum for RR \n   ");
        scanf("%d", quantum);
    }
}