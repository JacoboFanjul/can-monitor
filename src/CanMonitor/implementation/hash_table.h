#ifndef _HASHTABLE_H_
#define _HASHTABLE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "impl.h"

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
        sensor *value;
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
        sensorgroup *value;
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

/*
 * @key: The key to hash
 *
 * @size: The size of the hashtable
 *
 * @return: An integer N like 0 <= N < @size
 * This integer represents the index of @key in an array of size @size
 */
unsigned int hash(const char *key, unsigned int size);

HashTableSensors * hts_create(unsigned int size);

int hts_put(HashTableSensors *hashtable_sensors, const char *key, sensor *value);

void node_handler_sensors(HashTableSensors *hashtable_sensors, ListSensors *node);

sensor * hts_get(HashTableSensors *hashtable_sensors, const char *key);

void hts_free(HashTableSensors *hashtable_sensors);

HashTableSensorgroups * htsg_create(unsigned int size);

int htsg_put(HashTableSensorgroups *hashtable_sensorgroups, const char *key, sensorgroup *value);

void node_handler_sensorgroups (HashTableSensorgroups *hashtable_sensorgroups, ListSensorgroups *node);

sensorgroup * htsg_get(HashTableSensorgroups *hashtable_sensorgroups, const char *key);

void htsg_free(HashTableSensorgroups *hashtable_sensorgroups);

#endif