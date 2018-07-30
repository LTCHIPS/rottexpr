
/* 
 * File:   HashTable.h
 * Author: Steven LeVesque
 *
 * Created on March 13, 2018, 5:10 PM
 */

#ifndef HASHTABLE_H
#define HASHTABLE_H

typedef struct listNode 
{
    int isAvaliable;
    int key;
    int data;
} listNode;


typedef struct HashTable
{
    int totalSize;
    listNode ** table;
} HashTable;

void InitHashTable(HashTable * hashTable, int initSize);

int HashFunc(HashTable * hashTable, int key);

void Delete(HashTable * hashTable, int key);

void ClearHashTable (HashTable * hashTable);

void Insert(HashTable * hashTable, int key, int item);

int Lookup(HashTable * hashTable, int key);


#endif /* HASHTABLE_H */


