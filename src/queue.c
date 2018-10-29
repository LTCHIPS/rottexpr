//Copyright (C) 2017-2018  Steven LeVesque

#include <stdlib.h>
#include <stdio.h>
#include "queue.h"
#include <string.h>


void InitQueue(Queue* queue,size_t sizeOfItem)
{
    queue->NumOfItems = 0;
    queue->SizeOfItem = sizeOfItem;
    queue->Head = NULL;
    queue->Tail = queue->Head;

}

void Enqueue(Queue* queue, const void  * item)
{
    Node * newNode = (Node *) malloc(sizeof(Node));
    
    newNode->next = NULL;
    
    newNode->data =  malloc(queue->SizeOfItem);
    
    memcpy(newNode->data, item, queue->SizeOfItem);
    
    if (queue->NumOfItems == 0)
    {
        queue->Head = newNode;
        queue->Tail = newNode;
    }
    else if (queue->Head != NULL)
    {
        newNode->next = queue->Head;
        if (queue->Tail == queue->Head)
            queue->Tail = newNode->next;
        queue->Head = newNode;
    }
    
    queue->NumOfItems++;
    
}

void Dequeue(Queue* queue)
{
    if (queue->NumOfItems == 0)
    {
        return;
    }
    else if (queue->NumOfItems == 1)
    {
        
        free(queue->Head->data);
        free(queue->Head);
        queue->Head = NULL;
        queue->Tail = NULL;
        
        queue->NumOfItems--;
    }
    else 
    {
        
        Node * tempNode = queue->Head;
        
        queue->Head = queue->Head->next;
        
        free(tempNode->data);
        free(tempNode);
        
        queue->NumOfItems--;
        
    }    

}

void ClearQueue(Queue* queue)
{
    Node * currNode = queue->Head;
    
    Node * tempNode = NULL;
    while(queue->NumOfItems > 0)
    {
        tempNode = currNode;
        currNode = tempNode->next;
        //free(tempNode->data);
        free(tempNode);
        queue->NumOfItems--;
        
    }
    queue->Head = NULL;
    queue->Tail = NULL;

}
