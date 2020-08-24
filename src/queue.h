//Copyright (C) 2017-2018  Steven LeVesque

#ifndef QUEUE_H
#define QUEUE_H

typedef struct Node 
{
    void * data;
    struct Node * next;

} Node;

typedef struct Queue 
{
    int NumOfItems;
    size_t SizeOfItem;
    Node * Head;
    Node * Tail;

} Queue;

void InitQueue(Queue*,size_t);

void Enqueue(Queue*, const void  *);

void Dequeue(Queue*);

void ClearQueue(Queue*);

//void PrintQueue(Queue*);



#endif /* QUEUE_H */

