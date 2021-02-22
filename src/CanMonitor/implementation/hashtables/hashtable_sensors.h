#ifndef _HASHTABLE_SENSORS_H_
#define _HASHTABLE_SENSORS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../sensor.h"

/*
 * @key: The key string of a pair
 * The key is unique in the HashTable
 *
 * @value: The value corresponding to a key
 * A value is not unique. It can correspond to several keys
 *
 * @next: A pointer to the next node of the List
 */
typedef struct ListSensors
{
        char *key;
        sensor *sensor;
        struct ListSensors *next;
} ListSensors;

/*
 * @size: The size of the array
 *
 * @array: An array of size @size
 * Each cell of this array is a pointer to the first node of a linked list,
 * because we want our HashTable to use a Chaining collision handling
 */
typedef struct HashTableSensors
{
        unsigned int size;
        ListSensors **array;
} HashTableSensors;

// Sensors Table
HashTableSensors * hts_create(unsigned int size);

int hts_put(HashTableSensors *hashtable_sensors, const char *key, sensor *value);

void node_handler_sensors(HashTableSensors *hashtable_sensors, ListSensors *node);

sensor * hts_get(HashTableSensors *hashtable_sensors, const char *key);

sensor * hts_delete(HashTableSensors *hashtable_sensors, const char *key);

void hts_free(HashTableSensors *hashtable_sensors);

#endif
