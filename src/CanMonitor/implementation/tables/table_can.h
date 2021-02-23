#ifndef _HASHTABLE_CAN_H_
#define _HASHTABLE_CAN_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

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
        uint32_t pos;
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
typedef struct TableCan
{
        unsigned int size;
        ListSensorIds **array;
} TableCan;



// Sensorgroups Table
TableCan * table_can_create(unsigned int size);

int table_can_put(TableCan *table_can, uint32_t key, char *value);

void node_handler_can(TableCan *table_can, ListSensorIds *node);

char * table_can_delete(TableCan *table_can, uint32_t pos, char * sensor_id);

void table_can_free(TableCan *table_can);

#endif
