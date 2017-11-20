#ifndef QUEUE_H_INCLUDED
#define QUEUE_H_INCLUDED

//Fetched from https://codereview.stackexchange.com/questions/141238/implementing-a-generic-queue-in-c


typedef struct Node
{
  void *data;
  struct Node *next;
}node;

typedef struct QueueList
{
    int sizeOfQueue;
    size_t memSize;
    node *head;
    node *tail;
}Queue;

void queueInit(Queue *q, size_t memSize);
int enqueue(Queue *, const void *);
void dequeue(Queue *, void *);
void queuePeek(Queue *, void *);
void clearQueue(Queue *);
int getQueueSize(Queue *);

#endif /* QUEUE_H_INCLUDED */