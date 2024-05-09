#include "headers.h"
typedef struct memorySegment memorySegment;
struct memorySegment
{
    int free_size;
    int size;
    struct memorySegment *left;
    struct memorySegment *right;
    int containing_pid;
    int start;
    int end;
};
memorySegment *createSegment(int size, int start, int end)
{
    memorySegment *temp = (memorySegment *)malloc(sizeof(memorySegment));
    temp->containing_pid = -1;
    temp->free_size = size;
    temp->left = NULL;
    temp->right = NULL;
    temp->start = start;
    temp->end = end;
    return temp;
}
memorySegment *allocateMemory(memorySegment *temp, int pid, int size)
{
    size = pow(2, ceil(log(size) / log(2)));
    if (temp->free_size / 2 < size)
    {
        temp->containing_pid = pid;
        temp->free_size -= size;
        return temp;
    }
    if (temp->right != NULL)
    {
        if (temp->right->free_size >= size)
        {
            temp->free_size -= size;
            memorySegment *res = allocateMemory(temp->left, pid, size);
        }
        else
        {
            // create a one :)  ;
            memorySegment *l = createSegment(temp->size / 2, 0, temp->size / 2 - 1);
            temp->left = l;
            temp->free_size -= size;
            memorySegment *res = allocateMemory(temp->left, pid, size);
        }
    }
    else if (temp->right != NULL)
    {
        if (temp->left->free_size >= size)
        {
            temp->free_size -= size;
            memorySegment *res = allocateMemory(temp->left, pid, size);
        }
        else
        {
            memorySegment *r = createSegment(temp->size / 2, 0, temp->size / 2 - 1);
            temp->right = r;
            temp->free_size -= size;
            memorySegment *res = allocateMemory(temp->right, pid, size);
        }
    }
    return temp;
}
memorySegment *deleteMemory(memorySegment *temp, int pid)
{
    if (temp && temp->containing_pid == pid)
    {
        free(temp);
        return NULL;
    }
    if (temp->left && !temp->right)
        return temp;
    else if (temp->left)
        temp->left = deleteMemory(temp->left, pid);
    else if (temp->right)
        temp->right = deleteMemory(temp->right, pid);
    return temp;
}