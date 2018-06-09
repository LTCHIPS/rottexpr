#include <stdlib.h>
#include <stdio.h>
#include "LinkedList.h"

void InitLinkedList(LinkedList * linkedList)
{
    linkedList->NumOfItems = 0;
    linkedList->head = NULL;
    linkedList->IsInit = 1;
}

void InsertInList(LinkedList * linkedList, int key, int item)
{
    if (linkedList->head == NULL)
    {
        listNode * newNode = malloc(sizeof(listNode));
        newNode->data = item;
        newNode->key = key;
        
        linkedList->head = malloc(sizeof(listNode));
        linkedList->head = newNode;
        
        linkedList->head->next = NULL;
        
        linkedList->NumOfItems++;
        return;
    }
    else
    {
        listNode * x;
        int counter = 0;
        
        for (x = linkedList->head; counter < linkedList->NumOfItems; counter++, x = x->next )
        {
            if (x->next == NULL)
            {
                listNode * newNode = malloc(sizeof(listNode));
                newNode->data = item;
                newNode->key = key;
             
                x->next = malloc(sizeof(listNode));
                x->next = newNode;
                
                newNode->next = NULL;
        
                linkedList->NumOfItems++;
                break;
            }
        } 
    }
}

int SearchWithKey(LinkedList * linkedList, int key)
{
    listNode * x;
    int counter = 0;
    
    for (x = linkedList->head; counter < linkedList->NumOfItems; counter++, x = x->next )
    {
        if (x->key == key)
        {
            return x->data;
        }
        if (x->next == NULL)
        {
            break;
        }
    }
    return 0;
    
}

void DeleteAtIndex(LinkedList* linkedList,int index)
{
    listNode * x;
    int counter = 0;
    
    for (x = linkedList->head; counter < linkedList->NumOfItems; counter++, x = x->next )
    {
        if (counter == index)
        {
            if (counter == 0)
            {
                linkedList->head = linkedList->head->next;
            }
            free(x);
            linkedList->NumOfItems--;
            break;
        }
    }
    if (counter == linkedList->NumOfItems)
    {
        printf("ERROR: LinkedList could not find item at %d \n", index);
        exit(1);
    }
}
void DeleteWithKey(LinkedList* linkedList,int key)
{
    listNode * x = linkedList->head;
    int counter = 0;
    
    for (counter; counter < linkedList->NumOfItems; counter++, x = x->next )
    {
        if (x->key == key)
        {
            if (counter == 0)
            {
                linkedList->head = linkedList->head->next;
            }
            
            free(x);
            linkedList->NumOfItems--;
            break;
        }
    }
    if (counter == linkedList->NumOfItems)
    {
        printf("ERROR: LinkedList could not find item with key %d", key);
        exit(1);
    }
}

void DestroyList(LinkedList* linkedList)
{
    listNode *x = linkedList->head;
    int counter = 0;
    for (counter; counter < linkedList->NumOfItems; counter++ )
    {
        printf("%d ", x->data);
        listNode * temp = x->next;
        free(x);
        x = temp;
    }
}
