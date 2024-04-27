#include "headers.h"

enum Algo
{
    HPF = 1,
    SRTN = 2,
    RR = 3
};

enum State
{
    arrived = 0,
    started = 1,
    resumed = 2,
    stopped = 3,
    finished = 4
};

struct PData
{
    int id; // id of process in system
    int arrivaltime;
    int runningtime;
    int priority;
    int remaintime;
    int waittime;
    enum State state;
    pid_t pid; // id of forken process (in real system) to run process.out program
};

/*   Process Node      */
struct PNode
{
    struct PData val;
    struct PNode *next;
};

/*    Process Queue          */
struct PQueue
{
    struct PNode *head;
    struct PNode *tail;
    int count;
};

struct PQueue *createQ()
{
    struct PQueue *queue = (struct PQueue *)malloc(sizeof(struct PQueue));
    queue->head = NULL;
    queue->tail = NULL;
    queue->count = 0;
    return queue;
}

void enqueueQ(struct PData process, struct PQueue *queue)
{
    struct PNode *ele = (struct PNode *)malloc(sizeof(struct PNode));
    ele->val = process;
    ele->next = NULL;
    if (queue->count == 0)
    {
        queue->head = ele;
        queue->tail = ele;
    }
    else
    {
        queue->tail->next = ele;
        queue->tail = ele;
    }
    queue->count++;
}

void dequeueQ(struct PQueue *queue)
{
    if (queue->count == 0)
    {
        return;
    }
    struct PNode *temp = queue->head;
    if (queue->count == 1)
    {
        queue->head = NULL;
        queue->tail = NULL;
    }
    else
    {
        queue->head = temp->next;
    }
    free(temp);
    queue->count--;
    return;
}

bool frontQ(struct PQueue *queue, struct PData *data)
{
    if (queue->count == 0)
        return false;
    *data = queue->head->val;
    return true;
}

void printQ(struct PQueue *queue)
{
    if (queue->count == 0)
    {
        return;
    }
    struct PNode *ptr = queue->head;
    for (int i = 0; i < queue->count; i++)
    {
        printf("%d\t%d\t%d\t%d\n", ptr->val.id, ptr->val.arrivaltime, ptr->val.runningtime, ptr->val.priority);
        ptr = ptr->next;
    }
}

void deleteQ(struct PQueue *queue)
{
    while (queue->count != 0)
    {
        struct PNode *temp = queue->head;
        queue->head = temp->next;
        free(temp);
        queue->count--;
    }
    free(queue);
}

/*     Process Circular Queue             */
struct PCQueue
{
    struct PNode *head;
    struct PNode *tail;
    struct PNode *current;
    int count;
};

struct PCQueue *createCQ()
{
    struct PCQueue *queue = (struct PCQueue *)malloc(sizeof(struct PCQueue));
    queue->head = NULL;
    queue->tail = NULL;
    queue->current = NULL;
    queue->count = 0;
    return queue;
}

void enqueueCQ(struct PData process, struct PCQueue *queue)
{
    struct PNode *ele = (struct PNode *)malloc(sizeof(struct PNode));
    ele->val = process;
    ele->next = NULL;
    if (queue->count == 0)
    {
        queue->head = ele;
        queue->tail = ele;
        queue->current = ele;
    }
    else
    {
        queue->tail->next = ele;
        queue->tail = ele;
    }
    ele->next = queue->head;
    queue->count++;
}

void dequeueCQ(struct PCQueue *queue)
{
    if (queue->count == 0)
    {
        return;
    }
    struct PNode *temp = queue->head;
    if (queue->count == 1)
    {
        queue->head = NULL;
        queue->tail = NULL;
        queue->current = NULL;
    }
    else
    {
        queue->head = temp->next;
        queue->tail->next = queue->head;
    }
    free(temp);
    queue->count--;
    return;
}

bool frontCQ(struct PCQueue *queue, struct PData *data)
{
    if (queue->count == 0)
        return false;
    *data = queue->head->val;
    return true;
}

bool getCurCQ(struct PCQueue *queue, struct PData *data)
{
    if (queue->count == 0)
        return false;
    *data = queue->current->val;
    return true;
}

bool moveCurCQ(struct PCQueue *queue)
{
    if (queue->count == 0)
        return false;
    queue->current = queue->current->next;
    return true;
}

void delCurCQ(struct PCQueue *queue)
{
    if (queue->count == 0)
    {
        return;
    }
    struct PNode *prev, *temp;
    if (queue->count == 1)
    {
        temp = queue->current;
        queue->head = NULL;
        queue->tail = NULL;
        queue->current = NULL;
    }
    else
    {
        if (queue->current == queue->head)
        {
            temp = queue->head;
            queue->head = temp->next;
            queue->current = temp->next;
            queue->tail->next = queue->head;
        }
        else
        {
            temp = queue->head;
            prev = NULL;
            while (temp != queue->current)
            {
                prev = temp;
                temp = temp->next;
            }
            queue->current = temp->next;
            prev->next = queue->current;
            // mean that the current was pointing to tail of queue and when moved current pointer to next became pointing to head so we need reset tail pointer
            if (queue->current == queue->head)
                queue->tail = prev;
        }
    }
    free(temp);
    queue->count--;
}

void printCQ(struct PCQueue *queue)
{
    if (queue->count == 0)
    {
        return;
    }
    struct PNode *ptr = queue->head;
    for (int i = 0; i < queue->count; i++)
    {
        printf("%d\t%d\t%d\t%d\n", ptr->val.id, ptr->val.arrivaltime, ptr->val.runningtime, ptr->val.priority);
        ptr = ptr->next;
    }
}

void deleteCQ(struct PCQueue *queue)
{
    while (queue->count != 0)
    {
        struct PNode *temp = queue->head;
        queue->head = temp->next;
        free(temp);
        queue->count--;
    }
    free(queue);
}

/*   Process Priority Node           */
struct PPNode
{
    struct PData val;
    struct PPNode *next;
    int priority;
};

/*   Process Priority Queue   */
struct PPQueue
{
    struct PPNode *head;
    struct PPNode *tail;
    int count;
};

struct PPQueue *createPQ()
{
    struct PPQueue *queue = (struct PPQueue *)malloc(sizeof(struct PPQueue));
    queue->head = NULL;
    queue->tail = NULL;
    queue->count = 0;
    return queue;
}
// sort nodes as smaller priority comes first
void enqueuePQ(struct PData process, int priority, struct PPQueue *queue)
{
    struct PPNode *ele = (struct PPNode *)malloc(sizeof(struct PPNode));
    ele->val = process;
    ele->priority = priority;
    ele->next = NULL;
    if (queue->count == 0)
    {
        queue->head = ele;
        queue->tail = ele;
    }
    else
    {
        if (queue->head->priority > ele->priority)
        {
            ele->next = queue->head;
            queue->head = ele;
        }
        else
        {
            struct PPNode *cur = queue->head;
            struct PPNode *prev = NULL;
            while (cur && cur->priority <= ele->priority)
            {
                prev = cur;
                cur = cur->next;
            }
            ele->next = prev->next;
            prev->next = ele;
        }
    }
    queue->count++;
}

void dequeuePQ(struct PPQueue *queue)
{
    if (queue->count == 0)
    {
        return;
    }
    struct PPNode *temp = queue->head;
    if (queue->count == 1)
    {
        queue->head = NULL;
        queue->tail = NULL;
    }
    else
    {
        queue->head = temp->next;
    }
    free(temp);
    queue->count--;
    return;
}

bool frontPQ(struct PPQueue *queue, struct PData *data)
{
    if (queue->count == 0)
        return false;
    *data = queue->head->val;
    return true;
}

void printPQ(struct PPQueue *queue)
{
    if (queue->count == 0)
    {
        return;
    }
    struct PPNode *ptr = queue->head;
    for (int i = 0; i < queue->count; i++)
    {
        printf("%d\t%d\t%d\t%d\n", ptr->val.id, ptr->val.arrivaltime, ptr->val.runningtime, ptr->val.priority);
        ptr = ptr->next;
    }
}

void deletePQ(struct PPQueue *queue)
{
    while (queue->count != 0)
    {
        struct PPNode *temp = queue->head;
        queue->head = temp->next;
        free(temp);
        queue->count--;
    }
    free(queue);
}

// A linked list node
struct Node
{
    int data;
    struct Node *next;
};

// The queue, front stores the front node of the linked list and rear stores the last node of the linked list
struct Queue
{
    struct Node *front, *rear;
    int count;
};

// A utility function to create a new linked list node.
struct Node *newNode(int k)
{
    struct Node *temp = (struct Node *)malloc(sizeof(struct Node));
    temp->data = k;
    temp->next = NULL;
    return temp;
}

// A utility function to create an empty queue
struct Queue *createQueue()
{
    struct Queue *q = (struct Queue *)malloc(sizeof(struct Queue));
    q->front = q->rear = NULL;
    return q;
}

// The function to add a key k to the queue
void enQueue(struct Queue *q, int k)
{
    // Create a new node
    struct Node *temp = newNode(k);
    q->count++;
    // If queue is empty, then new node is front and rear both
    if (q->rear == NULL)
    {
        q->front = q->rear = temp;
        return;
    }

    // Add the new node at the end of the queue and change rear
    q->rear->next = temp;
    q->rear = temp;
}

// Function to remove a key from the queue
// Function to remove a key from the queue and return the removed key
int deQueue(struct Queue *q)
{
    // If queue is empty, return NULL.
    if (q->front == NULL)
        return -1;
    q->count--;
    // Store previous front and move front one node ahead
    struct Node *temp = q->front;
    int item = temp->data;

    q->front = q->front->next;

    // If front becomes NULL, then change rear also as NULL
    if (q->front == NULL)
        q->rear = NULL;

    free(temp);

    return item;
}
