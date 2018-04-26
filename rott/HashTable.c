#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "HashTable.h"
//#include "LinkedList.h"

#if !defined(ARRAY_SIZE)
    #define ARRAY_SIZE(x) (sizeof((x)) / sizeof((x)[0]))
#endif

void InitHashTable(HashTable * hashTable, int initSize)
{
    hashTable->totalSize = initSize;
    
    hashTable->table = calloc(sizeof(listNode), initSize);
    //hashTable->table = malloc(sizeof(LinkedList) * initSize);
    
    //memset(hashTable->table,0, sizeof(LinkedList) * initSize);
    memset(hashTable->table, NULL, sizeof(listNode)*initSize);
}

int HashFunc(HashTable * hashTable, int key)
{
    return abs((key)) % hashTable->totalSize;
}

void Delete(HashTable * hashTable, int key)
{   
    int index = HashFunc(hashTable,key);
    
    while(hashTable->table[index]->key != key) 
    {
      //go to next cell
        ++index;
		
      //wrap around the table
        index %= hashTable->totalSize;
    }
    
    //LinkedList * list = hashTable->table[index];
    
    //DeleteWithKey(list, key);
    free(&hashTable->table[index]);
}

void ClearHashTable (HashTable * hashTable)
{ 
    int x = 0;
    for (x; x < ARRAY_SIZE(hashTable->table); x++)
    {
        if (hashTable->table[x] != NULL)
        {
            //DestroyList(hashTable->table[x]);
            free(hashTable->table[x]);
        }
        
    }
    free(hashTable->table);
}

void Insert(HashTable * hashTable, int key, int item)
{
    int index = HashFunc(hashTable,key);
    
    
    while(hashTable->table[index] != NULL) 
    {
      //go to next cell
        ++index;
		
      //wrap around the table
        index %= hashTable->totalSize;
    }
    
    listNode * newNode = malloc(sizeof(listNode));
    
    newNode->data = item;
    newNode->key = key;
	
    hashTable->table[index] = newNode;

}

int Lookup(HashTable * hashTable, int key)
{
    int index = HashFunc(hashTable,key);
    
    int origIndex = index;
    
    
    //If it starts off at a NULL spot, it never existed...
    if (hashTable->table[index] == NULL)
    {
        return 0;
    }
    
    while(hashTable->table[index]->key != key) 
    {
      //go to next cell
        ++index;
		
      //wrap around the table
        index %= hashTable->totalSize;
        
        if (index == origIndex)
        {
            return 0;
        }
        
    }
    
    return hashTable->table[index]->data;
}