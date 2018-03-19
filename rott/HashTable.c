#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "HashTable.h"
#include "LinkedList.h"

#if !defined(ARRAY_SIZE)
    #define ARRAY_SIZE(x) (sizeof((x)) / sizeof((x)[0]))
#endif

void InitHashTable(HashTable * hashTable, int initSize)
{
    hashTable->totalSize = initSize;
    hashTable->table = malloc(sizeof(LinkedList) * initSize);
    
    memset(hashTable->table,0, sizeof(LinkedList) * initSize);
}

int HashFunc(HashTable * hashTable, int key)
{
    return abs((key)) % hashTable->totalSize;
}

void Delete(HashTable * hashTable, int key)
{   
    int index = HashFunc(hashTable,key);
    
    LinkedList * list = hashTable->table[index];
    
    DeleteWithKey(list, key);
    free(&hashTable->table[index]);
}

void ClearHashTable (HashTable * hashTable)
{ 
    int x = 0;
    for (x; x < ARRAY_SIZE(hashTable->table); x++)
    {
        if (hashTable->table[x] != 0)
        {
            DestroyList(hashTable->table[x]);
        }
        
    }
    free(hashTable->table);
}

void Insert(HashTable * hashTable, int key, int item)
{
    int index = HashFunc(hashTable,key);
    int found = 0;
    if (hashTable->table[index] != 0)
    {
        InsertInList(hashTable->table[index], key, item);    
    }
    else
    {
        LinkedList * newList = malloc(sizeof(LinkedList));
        
        InitLinkedList(newList);
        
        InsertInList(newList, key, item);
        
        hashTable->table[index] = newList;
    }
}

int Lookup(HashTable * hashTable, int key)
{
    int index = HashFunc(hashTable,key);
    
    if (hashTable->table == 0)
    {
        printf("ERROR: HashTable Lookup lead to a NULL Entry.");
        exit(1);
    }
    
    return SearchWithKey(hashTable->table[index], key);
}