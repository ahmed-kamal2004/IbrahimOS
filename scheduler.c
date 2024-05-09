#include "headers.h"
#include <string.h>
#include "queue.h"
struct message
{
    int id;
    int arrTime;
    int RunTime;
    int Priority;
    int memsize;
};
struct msgbuff
{
    long mtype;
    struct message msg;
};

void printStatistics(int, int);
void finishProcess(int);
void clearResourcesScheduler(int);
queueNode *readyProcesses = NULL;
queueNode *readyProcessesSRTN = NULL;
Process *currentProcess = NULL;
queueNode *processDone = NULL;
int process_quantum, processWorking, nProcesses, x, y, current_time, msgq_id, rec_val, shmid, quantum;
float CPU_Utilization = 0, avg_WA = 0, avg_WTA = 0, std_avg_WTA = 0, CPU_active_time = 0;
FILE *logFile;
key_t key_id, shd_key;
char *sch_algo;
int main(int argc, char **argv)
{
    signal(SIGUSR1, finishProcess);
    signal(SIGINT, clearResourcesScheduler);
    // Parameters Initialization
    int num_of_process = atoi(argv[2]);
    sch_algo = strdup(argv[1]); // 1 - Why ? not just = ?
    processWorking = num_of_process;
    nProcesses = num_of_process;
    current_time = 0;
    if (strcmp(sch_algo, "RR") == 0)
    {
        quantum = atoi(argv[3]);
    }
    //
    key_id = ftok("keyFile", 65);                             // for Queue
    shd_key = ftok("README", 65);                             // for Shared Memeory
    msgq_id = msgget(key_id, 0666 | IPC_CREAT);               // Queue
    shmid = shmget(shd_key, sizeof(pid_t), 0666 | IPC_CREAT); // Shared Memory

    if (msgq_id == -1 || shmid == -1) // Errors in Shared Memory
    {
        perror("Error in IPC");
        exit(-1);
    }
    // Opening File with "WRITE" mode
    logFile = fopen("scheduler.log", "w");
    // Init file
    fprintf(logFile, "%s", "#At time x process y state arr w total z remain y wait k\n");

    struct msgbuff msg;
    msg.mtype = 7;
    struct msqid_ds ctrl_status_ds;
    initClk();
    x = getClk();
    while (true)
    {
        // Arrived Process Using Message Queue IPC

        // Getting Arrived processes is Finished

        // Lets start forking and scheduling

        y = getClk();
        if (x != y)
        {
            printf("Time Step %d \n", y);
            x = y;
            msgctl(msgq_id, IPC_STAT, &ctrl_status_ds);
            while (ctrl_status_ds.__msg_cbytes && num_of_process)
            {
                rec_val = msgrcv(msgq_id, &msg, sizeof(msg.msg), 7, !IPC_NOWAIT);
                if (rec_val != -1)
                {
                    num_of_process--;
                    printf("I received the process %d we now at time step %d \n", msg.msg.id, getClk());
                    if (!strcmp(sch_algo, "SRTN"))
                    {
                        readyProcessesSRTN = enqueuePQSRTN(readyProcessesSRTN, initProcess(msg.msg.id, msg.msg.arrTime, msg.msg.Priority, msg.msg.RunTime, msg.msg.memsize)); // Ahmed Kamal
                    }
                    else if (!strcmp(sch_algo, "HPF"))
                    {
                        readyProcesses = enqueuePQHPF(readyProcesses, initProcess(msg.msg.id, msg.msg.arrTime, msg.msg.Priority, msg.msg.RunTime, msg.msg.memsize)); // Abo kamal
                    }
                    else
                    {
                        readyProcesses = enqueue(readyProcesses, initProcess(msg.msg.id, msg.msg.arrTime, msg.msg.Priority, msg.msg.RunTime, msg.msg.memsize)); // edition of memsize 3obal el memes
                    }
                }
                msgctl(msgq_id, IPC_STAT, &ctrl_status_ds);
            }
            if (!strcmp(sch_algo, "RR")) // Enters If Scheduling algorithm is RR
            {
                if (readyProcesses && readyProcesses->head->state == READY) // Then It is in Ready State
                {
                    if (readyProcesses->head->pid == -1) // First Time
                    {
                        int *child;
                        int pid = fork();
                        if (pid == -1)
                        {
                            perror("Error in Forking");
                            exit(-1);
                        }
                        else if (pid == 0)
                        {

                            child = shmat(shmid, NULL, 0); // Attach to Shared Memory
                            *child = getpid();
                            shmdt(child); // DeAttach to shared Memery
                            char remTime[32];
                            sprintf(remTime, "%d", readyProcesses->head->remainingTime);
                            if (execl("process.out", "process.out", remTime, NULL) == -1)
                            {
                                perror("Error in execution");
                                exit(-1);
                            }
                            // The Process went to execute on another Process in the OS
                        }
                        else // In the Kernel "Imagine"
                        {
                            raise(SIGSTOP); // Stops Waiting for Process Signal to wake up.
                            child = shmat(shmid, NULL, 0);
                            readyProcesses->head->pid = *child;
                            fprintf(logFile, "At time %d process %d started arr %d total %d remain %d wait %d\n", y, readyProcesses->head->id, readyProcesses->head->arrivalTime, readyProcesses->head->runningTime, readyProcesses->head->remainingTime, readyProcesses->head->waitingTime);
                            printf("At time %d process %d started arr %d total %d remain %d wait %d\n", y, readyProcesses->head->id, readyProcesses->head->arrivalTime, readyProcesses->head->runningTime, readyProcesses->head->remainingTime, readyProcesses->head->waitingTime);
                        }
                    }
                    else // If the Process is in Ready But is already created in the Kernel
                    {
                        kill(readyProcesses->head->pid, SIGCONT); // Sends signal to the process to continue
                        readyProcesses->head->waitingTime += getClk() - readyProcesses->head->lastTimeRun;
                        fprintf(logFile, "At time %d process %d resumed arr %d total %d remain %d wait %d\n", y, readyProcesses->head->id, readyProcesses->head->arrivalTime, readyProcesses->head->runningTime, readyProcesses->head->remainingTime, readyProcesses->head->waitingTime);
                        printf("At time %d process %d resumed arr %d total %d remain %d wait %d\n", y, readyProcesses->head->id, readyProcesses->head->arrivalTime, readyProcesses->head->runningTime, readyProcesses->head->remainingTime, readyProcesses->head->waitingTime);
                    }
                    readyProcesses->head->state = RUNNING;
                    process_quantum = quantum;
                }
                if (readyProcesses && readyProcesses->head->state == RUNNING)
                {

                    if (process_quantum)
                    {
                        kill(readyProcesses->head->pid, SIGUSR1); // To make remaining time of the process decrease
                        CPU_active_time++;
                        readyProcesses->head->remainingTime--;
                        process_quantum--;
                    }
                    if (!process_quantum)
                    {
                        readyProcesses->head->state = READY;
                        readyProcesses->head->lastTimeRun = getClk();
                        Process *toDelete;
                        kill(readyProcesses->head->pid, SIGSTOP);
                        fprintf(logFile, "At time %d process %d stopped arr %d total %d remain %d wait %d\n", y, readyProcesses->head->id, readyProcesses->head->arrivalTime, readyProcesses->head->runningTime, readyProcesses->head->remainingTime, readyProcesses->head->waitingTime);
                        printf("At time %d process %d stopped arr %d total %d remain %d wait %d\n", y, readyProcesses->head->id, readyProcesses->head->arrivalTime, readyProcesses->head->runningTime, readyProcesses->head->remainingTime, readyProcesses->head->waitingTime);
                        readyProcesses = dequeue(readyProcesses, &toDelete); // Remove from the begin of the queue
                        readyProcesses = enqueue(readyProcesses, toDelete);  // Add the end of the queue
                    }
                }
            }
            else if (!strcmp(sch_algo, "SRTN"))
            {
                if (currentProcess == NULL)
                {
                    if (readyProcessesSRTN && readyProcessesSRTN->head->state == READY) // Then It is in Ready State
                    {
                        readyProcessesSRTN = dequeue(readyProcessesSRTN, &currentProcess);
                        if (currentProcess->pid == -1) // First Time
                        {
                            int *child;
                            int pid = fork();
                            if (pid == -1)
                            {
                                perror("Error in Forking");
                                exit(-1);
                            }
                            else if (pid == 0)
                            {
                                child = shmat(shmid, NULL, 0); // Attach to Shared Memory
                                *child = getpid();
                                shmdt(child); // DeAttach to shared Memery
                                char remTime[32];
                                sprintf(remTime, "%d", currentProcess->remainingTime);
                                if (execl("process.out", "process.out", remTime, NULL) == -1)
                                {
                                    perror("Error in execution");
                                    exit(-1);
                                }
                                // The Process went to execute on another Process in the OS
                            }
                            else // In the Kernel "Imagine"
                            {
                                raise(SIGSTOP); // Stops Waiting for Process Signal to wake up.
                                child = shmat(shmid, NULL, 0);
                                currentProcess->pid = *child;
                                fprintf(logFile, "At time %d process %d started arr %d total %d remain %d wait %d\n", y, currentProcess->id, currentProcess->arrivalTime, currentProcess->runningTime, currentProcess->remainingTime, currentProcess->waitingTime);
                                printf("At time %d process %d started arr %d total %d remain %d wait %d\n", y, currentProcess->id, currentProcess->arrivalTime, currentProcess->runningTime, currentProcess->remainingTime, currentProcess->waitingTime);
                            }
                        }
                        else // If the Process is in Ready But is already created in the Kernel
                        {
                            kill(currentProcess->pid, SIGCONT); // Sends signal to the process to continue
                            currentProcess->waitingTime += getClk() - currentProcess->lastTimeRun;
                            fprintf(logFile, "At time %d process %d resumed arr %d total %d remain %d wait %d\n", y, currentProcess->id, currentProcess->arrivalTime, currentProcess->runningTime, currentProcess->remainingTime, currentProcess->waitingTime);
                            printf("At time %d process %d resumed arr %d total %d remain %d wait %d\n", y, currentProcess->id, currentProcess->arrivalTime, currentProcess->runningTime, currentProcess->remainingTime, currentProcess->waitingTime);
                        }
                        currentProcess->state = RUNNING;
                    }
                }
                if (currentProcess && currentProcess->state == RUNNING)
                {
                    CPU_active_time++;
                    currentProcess->remainingTime--;
                    /*if(currentProcess->id==3)
                    {
                        printf("remaingtime=%d\n",currentProcess->remainingTime);
                    }*/
                    currentProcess->lastTimeRun = getClk();
                    kill(currentProcess->pid, SIGUSR1); // To make remaining time of the process decrease
                    if (currentProcess && readyProcessesSRTN && readyProcessesSRTN->head->remainingTime < currentProcess->remainingTime)
                    {
                        kill(currentProcess->pid, SIGSTOP);
                        currentProcess->state = READY;

                        fprintf(logFile, "At time %d process %d stopped arr %d total %d remain %d wait %d\n", y, currentProcess->id, currentProcess->arrivalTime, currentProcess->runningTime, currentProcess->remainingTime, currentProcess->waitingTime);
                        printf("At time %d process %d stopped arr %d total %d remain %d wait %d\n", y, currentProcess->id, currentProcess->arrivalTime, currentProcess->runningTime, currentProcess->remainingTime, currentProcess->waitingTime);
                        readyProcessesSRTN = enqueuePQSRTN(readyProcessesSRTN, currentProcess);
                        // printProcess(currentProcess);
                        // readyProcessesSRTN=enqueuePQSRTN(readyProcessesSRTN,currentProcess);
                        currentProcess = NULL;
                        // readyProcessesSRTN=dequeue(readyProcessesSRTN,&currentProcess);

                        // Process * tempp;
                        // readyProcessesSRTN=dequeue(readyProcessesSRTN,&tempp);
                    }
                }
            }
            else if (!strcmp(sch_algo, "HPF"))
            {
                if (readyProcesses && readyProcesses->head->state == READY) // Then It is in Ready State
                {
                    int *child;
                    int pid = fork();
                    if (pid == -1)
                    {
                        perror("Error in Forking");
                        exit(-1);
                    }
                    else if (pid == 0)
                    {
                        child = shmat(shmid, NULL, 0); // Attach to Shared Memory
                        *child = getpid();
                        shmdt(child); // DeAttach to shared Memery
                        char remTime[32];
                        sprintf(remTime, "%d", readyProcesses->head->remainingTime);
                        if (execl("process.out", "process.out", remTime, NULL) == -1)
                        {
                            perror("Error in execution");
                            exit(-1);
                        }
                        // The Process went to execute on another Process in the OS
                    }
                    else // In the Kernel "Imagine"
                    {
                        raise(SIGSTOP); // Stops Waiting for Process Signal to wake up.
                        child = shmat(shmid, NULL, 0);
                        readyProcesses->head->pid = *child;
                        readyProcesses->head->waitingTime += getClk() - readyProcesses->head->arrivalTime;
                        fprintf(logFile, "At time %d process %d started arr %d total %d remain %d wait %d\n", y, readyProcesses->head->id, readyProcesses->head->arrivalTime, readyProcesses->head->runningTime, readyProcesses->head->remainingTime, readyProcesses->head->waitingTime);
                        printf("At time %d process %d started arr %d total %d remain %d wait %d\n", y, readyProcesses->head->id, readyProcesses->head->arrivalTime, readyProcesses->head->runningTime, readyProcesses->head->remainingTime, readyProcesses->head->waitingTime);
                    }
                    readyProcesses->head->state = RUNNING;
                    readyProcesses->head->temppriority = 0;
                }
                else if (readyProcesses && readyProcesses->head->state == RUNNING)
                {
                    kill(readyProcesses->head->pid, SIGUSR1); // To make remaining time of the process decrease
                    CPU_active_time++;
                    readyProcesses->head->remainingTime--;
                    readyProcesses->head->lastTimeRun = y;
                }
            }

            // For ending Processes
            if (processWorking == 0)
            {
                break;
            }
        }
    }
    print(processDone);
    printStatistics(nProcesses, y);
    destroyClk(true);
}
void printStatistics(int nProcesses, int current_time)
{
    FILE *file;
    file = freopen("scheduler.perf", "w", stdout);
    float waitingTime = 0;
    float WTA[nProcesses];
    while (!isEmpty(processDone))
    {
        Process *toDelete;
        processDone = dequeue(processDone, &toDelete);
        waitingTime += toDelete->waitingTime;
        WTA[toDelete->id - 1] = (toDelete->finishTime - toDelete->arrivalTime) / (float)toDelete->runningTime;
    }
    for (int i = 0; i < nProcesses; i++)
    {
        avg_WTA += WTA[i];
    }
    avg_WTA /= nProcesses;
    avg_WA = waitingTime / nProcesses;
    for (int i = 0; i < nProcesses; i++)
    {
        std_avg_WTA += (WTA[i] - avg_WTA) * (WTA[i] - avg_WTA);
    }
    std_avg_WTA /= nProcesses;
    std_avg_WTA = sqrt(std_avg_WTA);
    printf("CPU Utiltization : %0.3f %%\n", (CPU_active_time) * 100 / (float)(current_time - 1));
    printf("Average Weighted Turnaround Time : %0.3f \n", avg_WTA);
    printf("Average Waiting Time : %0.3f\n", avg_WA);
    printf("Standard deviation for average weighted turnaround time: %.3f\n", std_avg_WTA);
    fclose(file);
}
void finishProcessSRTN()
{
    currentProcess->state = TERMINATED;
    processWorking--;
    currentProcess->finishTime = getClk();
    float TA = currentProcess->finishTime - currentProcess->arrivalTime;

    fprintf(logFile, "At time %d process %d finished arr %d total %d remain %d wait %d TA %0.2f WTA %0.2f\n", y, currentProcess->id, currentProcess->arrivalTime, currentProcess->runningTime, currentProcess->remainingTime, currentProcess->waitingTime, TA, TA / currentProcess->runningTime);
    if (currentProcess->id == 3)
    {
        printf("process done id=%d\n", currentProcess->id);
    }
    processDone = enqueue(processDone, currentProcess);
    currentProcess = NULL;
}
void finishProcess(int signum)
{
    if (!strcmp(sch_algo, "SRTN"))
    {
        finishProcessSRTN();
        return;
    }
    readyProcesses->head->state = TERMINATED;
    processWorking--;
    readyProcesses->head->finishTime = getClk();
    Process *toDelete;
    float TA = readyProcesses->head->finishTime - readyProcesses->head->arrivalTime;
    fprintf(logFile, "At time %d process %d finished arr %d total %d remain %d wait %d TA %0.2f WTA %0.2f\n", y, readyProcesses->head->id, readyProcesses->head->arrivalTime, readyProcesses->head->runningTime, readyProcesses->head->remainingTime, readyProcesses->head->waitingTime, TA, TA / readyProcesses->head->runningTime);
    printf("At time %d process %d finished arr %d total %d remain %d wait %d TA %0.2f WTA %0.2f\n", y, readyProcesses->head->id, readyProcesses->head->arrivalTime, readyProcesses->head->runningTime, readyProcesses->head->remainingTime, readyProcesses->head->waitingTime, TA, TA / readyProcesses->head->runningTime);
    readyProcesses = dequeue(readyProcesses, &toDelete);
    processDone = enqueue(processDone, toDelete);
}
void clearResourcesScheduler(int signum)
{
    shmctl(shmid, IPC_RMID, (struct shmid_ds *)0);
}