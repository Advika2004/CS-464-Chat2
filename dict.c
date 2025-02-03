#include "dict.h"

// Simple hash function to map a string key to an index
static unsigned long int dcthash(char *key)
{
    unsigned long int code = 0;
    int i;

    for (i = 0; key[i] != '\0'; i++)
    {
        code = key[i] + 31 * code;
    }

    return code;
}

// Create a new dictionary with an initial capacity
Dict *dctCreate()
{
    int i;
    Dict *dict = malloc(sizeof(Dict));
    if (dict == NULL)
    {
        return NULL;
    }

    dict->cap = START_SIZE;
    dict->arr = malloc(sizeof(Node *) * dict->cap);
    if (dict->arr == NULL)
    {
        free(dict);
        return NULL;
    }

    for (i = 0; i < dict->cap; i++)
    {
        dict->arr[i] = NULL;
    }

    dict->size = 0;
    return dict;
}

// Destroy the dictionary and free allocated memory
void dctDestroy(Dict *dct)
{
    int i;
    Node *current;
    Node *next;

    if (dct == NULL)
    {
        return;
    }

    for (i = 0; i < dct->cap; i++)
    {
        current = dct->arr[i];
        while (current != NULL)
        {
            next = current->next;
            free(current->key);
            free(current);
            current = next;
        }
    }

    free(dct->arr);
    free(dct);
}

// Insert a key-value pair into the dictionary
void dctInsert(Dict *dct, char *key, int value)
{
    if (dct == NULL || key == NULL)
    {
        return;
    }

    // Check if rehashing is needed
    if ((float)dct->size / dct->cap >= 1.0)
    {
        dctrehash(dct);
    }

    unsigned long int hashedKey = dcthash(key);
    unsigned long int index = hashedKey % dct->cap;
    Node *newNode = malloc(sizeof(Node));
    if (newNode == NULL)
    {
        return;
    }

    newNode->key = strdup(key);  // Duplicate the string key
    newNode->value = value;
    newNode->next = dct->arr[index];
    dct->arr[index] = newNode;
    dct->size++;
}

// Get the value associated with a key
int searchByKey(Dict *dct, char *key)
{
    if (dct == NULL || key == NULL)
    {
        return -1;  // Return an invalid value if key is not found
    }

    unsigned long int hashedKey = dcthash(key);
    unsigned long int index = hashedKey % dct->cap;
    Node *current = dct->arr[index];

    while (current != NULL)
    {
        if (strcmp(current->key, key) == 0)
        {
            return current->value;
        }
        current = current->next;
    }

    return -1;  // Return -1 if key is not found
}

// Get the key associated with a value
char *searchByValue(Dict *dct, int value)
{
    if (dct == NULL)
    {
        return NULL;
    }

    for (int i = 0; i < dct->cap; i++)
    {
        Node *current = dct->arr[i];
        while (current != NULL)
        {
            if (current->value == value)
            {
                return current->key;
            }
            current = current->next;
        }
    }

    return NULL;  // Return NULL if value is not found
}

// Remove a key-value pair from the dictionary
void dctRemoveKey(Dict *dct, char *key)
{
    if (dct == NULL || key == NULL)
    {
        return;
    }

    unsigned long int hashedKey = dcthash(key);
    unsigned long int index = hashedKey % dct->cap;
    Node *current = dct->arr[index];
    Node *prev = NULL;

    while (current != NULL)
    {
        if (strcmp(current->key, key) == 0)
        {
            if (prev == NULL)
            {
                dct->arr[index] = current->next;
            }
            else
            {
                prev->next = current->next;
            }

            free(current->key);
            free(current);
            dct->size--;
            return;
        }
        prev = current;
        current = current->next;
    }
}

void dctRemoveValue(Dict *dct, int value){

    if (dct == NULL)
    {
        return;
    }

    for (int index = 0; index < dct->cap; index++)
    {
        Node *current = dct->arr[index];
        Node *prev = NULL;

        while (current != NULL)
        {
            if (current->value == value)
            {
                if (prev == NULL)
                {
                    dct->arr[index] = current->next;
                }
                else
                {
                    prev->next = current->next;
                }

                free(current);
                dct->size--;
                return;
            }
            prev = current;
            current = current->next;
        }
    }
}

// Rehash the dictionary to a new capacity
void dctrehash(Dict *dct)
{
    int oldCap = dct->cap;
    int newCap = oldCap + REHASH_SIZE;
    Node **newArr = malloc(sizeof(Node *) * newCap);
    if (newArr == NULL)
    {
        return;
    }

    for (int i = 0; i < newCap; i++)
    {
        newArr[i] = NULL;
    }

    for (int i = 0; i < oldCap; i++)
    {
        Node *current = dct->arr[i];
        while (current != NULL)
        {
            unsigned long int newIndex = dcthash(current->key) % newCap;
            Node *temp = current->next;
            current->next = newArr[newIndex];
            newArr[newIndex] = current;
            current = temp;
        }
    }

    free(dct->arr);
    dct->arr = newArr;
    dct->cap = newCap;
}

// dctkeys: Enumerates all of the keys in a dictionary.

char **dctkeys(Dict *dct) {
    char **keys;
    Node *current;
    int i = 0;
    int j = 0;
    int keyCount = dct->size;

    if (dct==NULL){
        return NULL;
    }

    if (keyCount == 0){
        return NULL;
    }

    keys = (char **)malloc(sizeof(char*) * keyCount);
    if (keys == NULL) {  /*have to check if the MALLOCKING failed */
        return NULL;
    }

    for (i = 0; i < dct->cap; i++) { /*keeping track of the backing array going through indeces of that */
        current = dct->arr[i]; /*the current node is the index I am at*/

        while (current != NULL){ /* while I still have nodes*/
            keys[j] = current->key; /*go through the array I have mallocked */
            j++; /*increment that*/
            current = current->next; /*go to the next node*/
            }
        }
    return keys; 
}