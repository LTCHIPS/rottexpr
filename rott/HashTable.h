/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   HashTable.h
 * Author: LTCHIPS
 *
 * Created on March 13, 2018, 5:10 PM
 */

#ifndef HASHTABLE_H
#define HASHTABLE_H

typedef struct HashTable
{
    int totalSize;
    int * keys;
    int * table;
} HashTable;

void InitHashTable(HashTable * hashTable, int initSize);

int HashFunc(HashTable * hashTable, int key, int item);

void Delete(HashTable * hashTable, int key);

void ClearHashTable (HashTable * hashTable);

void Insert(HashTable * hashTable, int key, int item);

int Lookup(HashTable * hashTable, int key);


#endif /* HASHTABLE_H */


