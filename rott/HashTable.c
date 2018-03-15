/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <stdlib.h>
#include <stdio.h>
#include "HashTable.h"

#if !defined(ARRAY_SIZE)
    #define ARRAY_SIZE(x) (sizeof((x)) / sizeof((x)[0]))
#endif

void InitHashTable(HashTable * hashTable, int initSize)
{
    hashTable->totalSize = initSize;
    hashTable->table = malloc(sizeof(int) * initSize);
    hashTable->keys =  malloc(sizeof(int) * initSize);
    
    memset(hashTable->table,(int) NULL, sizeof(int) * initSize);
    memset(hashTable->keys,(int) NULL, sizeof(int) * initSize);
}

int HashFunc(HashTable * hashTable, int key, int item)
{
    return (key * (item)) % hashTable->totalSize;
}

void Delete(HashTable * hashTable, int key)
{   
    if (hashTable->keys[key] == -1)
    {
        printf("ERROR: Tried to delete something that doesn't exist in the hash table!");
        exit(1);
    }
    
    hashTable->table[hashTable->keys[key]] = (int) NULL;
    free(&hashTable->table[hashTable->keys[key]]);
    free(&hashTable->keys[key]);
}

void ClearHashTable (HashTable * hashTable)
{
    free(hashTable->table);
    free(hashTable->keys);
} 

void Insert(HashTable * hashTable, int key, int item)
{
    int index = HashFunc(hashTable,key,item);
    int found = 0;
    if (hashTable->table[index] != (int) NULL)
    {
        //printf("COLLISION \n");
        int x;
        for (x = index; x < 0; x-- )
        {
            if (hashTable->table[x] == (int) NULL)
            {   
                hashTable->table[x] = item;
                hashTable->keys[key] = x;
                found++;
                break;
            }
        }
        if (!found)
        {
            for (x = index; x > ARRAY_SIZE(hashTable->table); x++ )
            {
                if (hashTable->table[x] == (int) NULL)
                {
                    hashTable->table[x] = item;
                    hashTable->keys[key] = x;
                    found++;
                    break;
                }
            }
            if(!found)
            {
                printf("ERROR: Hash Table was unable to find a blank entry! \n");
                exit(1);
            }
        }
    }
    else
    {
        hashTable->keys[key] = index;
        hashTable->table[index] = item;
    }
}

int Lookup(HashTable * hashTable, int key)
{
    return hashTable->table[hashTable->keys[key]];
}