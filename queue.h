#include <stdio.h>      //if you don't use scanf/printf change this include
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#define RUNNING 0
#define READY 1
#define TERMINATED 2
typedef struct queueNode queueNode;
typedef struct Process Process;
struct Process {
    int id;
    int pid;
    int arrivalTime;
    int runningTime;
    int remainingTime;
	int waitingTime;
	int lastTimeRun;
	int priority;
	int temppriority;
    int finishTime;
    int state;
	int memsize; // Ahmed Kamal
};

struct queueNode {
    Process * head;
    queueNode * Next;
};
Process * initProcess(int id,int arrivalTime,int priority,int runningTime,int memsize){ // Ahmed Kamal
	Process * temp = (Process * ) malloc(sizeof(Process));
	temp->id = id;
	temp->pid = -1;
    temp ->arrivalTime = arrivalTime;
	temp->runningTime = runningTime;
	temp->remainingTime = runningTime;
	temp->waitingTime = 0;
	temp->lastTimeRun = -1;
	temp->priority = priority;
	temp->temppriority=priority;
	temp ->state = READY;
	temp->finishTime = -1;
	temp->memsize = memsize; // Ahmed Kamal
	return temp;
}
Process * copyProcess(Process * item){
	Process * temp = (Process * ) malloc(sizeof(Process));
	temp->id = item->id;
	temp->pid = item->pid;
    temp ->arrivalTime = item->arrivalTime;
	temp->lastTimeRun = item->lastTimeRun;
	temp->waitingTime = item->waitingTime;
	temp->priority = item->priority;
	temp->temppriority=item->temppriority;
	temp->runningTime = item->runningTime;
	temp->remainingTime = item->remainingTime;
	temp ->state = item->state;
	temp->finishTime = item->finishTime;
	temp->memsize = item->memsize; // Ahmed Kamal
	return temp;
}
void printProcess(Process * item){
    printf("\nid:%d  pid:%d  arrivalTime:%d  runningTime:%d  remainingTime:%d lastTimeRun:%d waitingTime:%d finishingTime:%d  priority: %d state:%d\n",item->id,item->pid,item->arrivalTime,item->runningTime,item->remainingTime,item->lastTimeRun,item->waitingTime,item->finishTime,item->priority,item->state);
}
queueNode* setHead (Process *  item,queueNode * Next){
	queueNode* temp = (queueNode *) malloc ( sizeof(queueNode)); 
    temp -> head = item;
    temp -> Next = Next;
	return temp ;
}
queueNode * enqueue(queueNode* queue,Process * item){
	if (queue==NULL){
		queue= setHead(item,NULL);
	}
	else {
		queueNode * temp = queue;
		while (temp->Next)
				temp = temp->Next;
		queueNode * added =setHead(item,NULL); // set the next queue node
        temp -> Next = added; 
	}
    return queue;
}
queueNode * dequeue(queueNode* queue,Process ** dequeued){
	queueNode* toDelete = queue;
	Process * toget = copyProcess(queue->head);
	*dequeued = toget;
    queue = queue-> Next;
    free(toDelete);
	return queue;
}
int isEmpty(queueNode * queue){
	return queue ? 0 : 1;
}
void print(queueNode * queue){
    queueNode * temp = queue;
    while(temp){
        printProcess(temp->head);
		temp = temp->Next;
    }
}
queueNode * enqueuePQSRTN(queueNode* queue,Process * item){
	if (queue==NULL){
		queue= setHead(item,NULL);
	}
	else{
		queueNode * temp = queue;
		queueNode * prev = queue;
		while (temp&&temp->head->remainingTime<=item->remainingTime)
		{
			prev=temp;
			temp = temp->Next;
		}
		queueNode * added =setHead(item,temp); // set the next queue node
		if(prev!=temp)
		{
        	prev-> Next = added;
		}
		else{
			queue=added;
		}
	}
    return queue;
}
queueNode * enqueuePQHPF(queueNode* queue,Process * item){
	if (queue==NULL){
		queue= setHead(item,NULL);
	}
	else{
		queueNode * temp = queue;
		queueNode * prev = queue;
		while (temp&&temp->head->temppriority<=item->temppriority)
		{
			prev=temp;
			temp = temp->Next;
		}
		queueNode * added =setHead(item,temp); // set the next queue node
		if(prev!=temp)
		{
        	prev-> Next = added;
		}
		else{
			queue=added;
		}
	}
    return queue;
}


Process *frontPQSRTN(queueNode* queue)
{
	if (queue==NULL){
		return NULL;
	}
	else {
		Process * temp= copyProcess(queue->head);
		return temp;
	}
}
