#ifndef _HASHTABLE_CAN_H_
#define _HASHTABLE_CAN_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * @key: The key string of a pair
 * The key is unique in the HashTable
 *
 * @value: The value corresponding to a key
 * A value is not unique. It can correspond to several keys
 *
 * @next: A pointer to the next node of the List
 */
typedef struct ListSensorIds
{
        char *key;
        char *sensor_id;
        struct ListSensorIds *next;
} ListSensorIds;

/*
 * @size: The size of the array
 *
 * @array: An array of size @size
 * Each cell of this array is a pointer to the first node of a linked list,
 * because we want our HashTable to use a Chaining collision handling
 */
typedef struct HashTableCan
{
        unsigned int size;
        ListSensorIds **array;
} HashTableCan;



// Sensorgroups Table
HashTableCan * htcan_create(unsigned int size);

int htcan_put(HashTableCan *hashtable_can, const char *key, char *value);

void node_handler_can(HashTableCan *hashtable_can, ListSensorIds *node);

char * htcan_get(HashTableCan *hashtable_can, const char *key);

char * htcan_delete(HashTableCan *hashtable_can, const char *key);

void htcan_free(HashTableCan *hashtable_can);

#endif
