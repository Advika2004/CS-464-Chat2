#ifndef DICT_H
#define DICT_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Node for the dictionary that holds a key-value pair (string and int)
typedef struct Node {
    char* key;      // key (string)
    int value;      // value (int)
    struct Node *next;  // next node in case of collision
} Node;

// Dictionary structure
typedef struct Dict {
    int cap;         // capacity of the dictionary
    Node **arr;      // array of nodes (buckets)
    int size;        // number of key-value pairs in the dictionary
} Dict;

// Function declarations
Dict *dctCreate();
void dctDestroy(Dict *dct);
void dctInsert(Dict *dct, char *key, int value);
int searchByKey(Dict *dct, char *key); //returns the value
char *searchByValue(Dict *dct, int value); //returns the key
void dctRemoveKey(Dict *dct, char *key);
void dctRemoveValue(Dict *dct, int value);
void dctrehash(Dict *dct);

//globals
struct Dict *table;

#define START_SIZE 2
#define REHASH_SIZE 2

#endif /* DICT_H */