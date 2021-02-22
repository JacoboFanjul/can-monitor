#ifndef _HASHTABLE_SENSORGROUPS_H_
#define _HASHTABLE_SENSORGROUPS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../sensorgroup.h"

/*
 * @key: The key string of a pair
 * The key is unique in the HashTable
 *
 * @value: The value corresponding to a key
 * A value is not unique. It can correspond to several keys
 *
 * @next: A pointer to the next node of the List
 */
typedef struct ListSensorgroups
{
        char *key;
        sensorgroup *sensorgroup;
        struct ListSensorgroups *next;
} ListSensorgroups;

/*
 * @size: The size of the array
 *
 * @array: An array of size @size
 * Each cell of this array is a pointer to the first node of a linked list,
 * because we want our HashTable to use a Chaining collision handling
 */
typedef struct HashTableSensorgroups
{
        unsigned int size;
        ListSensorgroups **array;
} HashTableSensorgroups;



// Sensorgroups Table
HashTableSensorgroups * htsg_create(unsigned int size);

int htsg_put(HashTableSensorgroups *hashtable_sensorgroups, const char *key, sensorgroup *value);

void node_handler_sensorgroups (HashTableSensorgroups *hashtable_sensorgroups, ListSensorgroups *node);

sensorgroup * htsg_get(HashTableSensorgroups *hashtable_sensorgroups, const char *key);

sensorgroup * htsg_delete(HashTableSensorgroups *hashtable_sensorgroups, const char *key);

void htsg_free(HashTableSensorgroups *hashtable_sensorgroups);

#endif
