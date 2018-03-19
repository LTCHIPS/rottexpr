/* 
 * File:   LinkedList.h
 * Author: LTCHIPS
 *
 * Created on March 19, 2018, 3:40 PM
 */

#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct listNode 
{
    int key;
    int data;
    struct listNode * next;
} listNode;

typedef struct LinkedList
{
    int NumOfItems;
    struct listNode * head;
} LinkedList;

int SearchWithKey(LinkedList *, int);

void InitLinkedList(LinkedList*);

void InsertInList(LinkedList*,int, int);

void DeleteAtIndex(LinkedList*,int);

void DeleteWithKey(LinkedList*,int);

void DestroyList(LinkedList*);


#ifdef __cplusplus
}
#endif

#endif /* LINKEDLIST_H */

