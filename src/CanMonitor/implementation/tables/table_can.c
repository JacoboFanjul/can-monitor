#include "inttypes.h"
#include "table_can.h"
#include "hashtable_common.h"
#include "../../CanMonitor.h"

/*
 * table_can_create() - If the size is a positive integer, allocate the requisite
 * memory for a new hashtable and its indexed array. Assign the size of the
 * array in ht->size.
 * size: The size of the hashtable's array.
 *
 * Return: A pointer to the newly allocated Hashtable. If size is zero or a
 * negative number, return NULL. If a memory allocation fails, return NULL.
 */
TableCan *table_can_create(unsigned int size)
{
    TableCan *table_can;

    if (size < 1)
    {
        return NULL;
    }

    table_can = malloc(sizeof(TableCan));
    if (table_can == NULL)
    {
        return (NULL);
    }

    table_can->array = (ListSensorIds **)malloc(size * sizeof(ListSensorIds));
    if (table_can->array == NULL)
    {
        return (NULL);
    }
    memset(table_can->array, 0, size * sizeof(ListSensorIds));

    table_can->size = size;

    return table_can;
}

/*
 * table_can_put() - Allocates memory for a new node and calls the node_handler
 * function to either insert the node if the key does not exist, or update
 * a prexisting node in the case that it has the same key as that passed
 * to this function.
 * @key: The key to add to the hash table.
 * @sensorgroup: The corresponding sensorgroup to add to the node.
 *
 * Return: 1 if memory allocation fails, and 0 if the function returns
 * successfully.
 */
int table_can_put(TableCan *table_can, uint32_t pos, char *value)
{
    ListSensorIds *node;

    if (table_can == NULL)
    {
        return 1;
    }
    node = malloc(sizeof(ListSensorIds));
    if (node == NULL)
    {
        return 1;
    }

    char* canid_pos;
    snprintf(canid_pos, 11, "%"PRIu32, pos);
    node->pos = hash(strdup(canid_pos), CAN_IDS_TABLE_SIZE);
    node->sensor_id = strdup(value);

    node_handler_can(table_can, node);

    return 0;
}

/*
 * node_handler_can() - If the index item is a linked list, traverse it to ensure
 * that there is not already an item with the key passed. If there is,
 * reassign the sensorgroup of the prexisting node to the current instead of adding
 * the new node.
 * @hashtable: The hashtable of Lists.
 * @node: The linked list to add a node to or update.
 *
 * Return: Void.
 */
void node_handler_can(TableCan *table_can, ListSensorIds *node)
{
    ListSensorIds *tmp = table_can->array[node->pos];
    if (table_can->array[node->pos] != NULL)
    {
        tmp = table_can->array[node->pos];
        while (tmp != NULL)
        {
            if (strcmp(tmp->sensor_id, node->sensor_id) == 0)
            {
                break;
            }
            tmp = tmp->next;
        }
        if (tmp == NULL)
        {
            node->next = table_can->array[node->pos];
            table_can->array[node->pos] = node;
        }
        else
        {
            free(tmp->sensor_id);
            tmp->sensor_id = node->sensor_id;
            free(node->sensor_id);
            free(node);
        }
    }
    else
    {
        node->next = NULL;
        table_can->array[node->pos] = node;
    }
}

char * table_can_delete(TableCan *table_can, uint32_t pos, char * sensor_id)
{
    ListSensorIds *sensor_ids;

    if (table_can == NULL)
    {
        return NULL;
    }

    char* canid_pos;
    snprintf(canid_pos, 11, "%"PRIu32, pos);
    sensor_ids = table_can->array[hash(canid_pos, 5)];
    
    ListSensorIds *tmp = sensor_ids;
    ListSensorIds *prev = NULL;

    if (tmp == NULL)
    {
        return NULL;
    }

    if (strcmp(tmp->sensor_id, sensor_id) == 0)
    {
        char *sens_id = tmp->sensor_id;
        table_can->array[pos] = tmp->next;
        free(tmp);
        return sens_id;
    }

    while (tmp != NULL && strcmp(tmp->sensor_id, sensor_id) != 0)
    {
        prev = tmp;
        tmp = tmp->next;
    }

    if (tmp == NULL)
    {
        return NULL;
    }

    prev->next = tmp->next;
    char *sens_id = tmp->sensor_id;
    free(tmp);
    return sens_id;
}

/*
 * table_can_free() - Free the items in a hashtable. Iterate through the hashtable's
 * array. If it is a linked list, then traverse the list and free all the
 * node's attributes and then the node itself. When the end of the list is
 * reached, free the array item itself (i.e., `hashtable->array[i]`). Finally,
 * when all the array items are freed, free the hashtable array pointer and
 * the hashtable itself.
 * @hashtable: The hashtable to free.
 *
 * Return: If the hashtable is NULL, return because there is nothing to free.
 */
void table_can_free(TableCan *table_can)
{
    // ListSensorIds *tmp;
    // unsigned int i;

    // if (hashtable_can == NULL)
    // {
    //     return;
    // }

    // for (i = 0; i < hashtable_can->size; ++i)
    // {
    //     if (hashtable_can->array[i] != NULL)
    //     {
    //         /* Traverse the list and free the nodes. */
    //         while (hashtable_can->array[i] != NULL)
    //         {
    //             tmp = hashtable_can->array[i]->next;
    //             free(hashtable_can->array[i]->key);
    //             free(hashtable_can->array[i]->sensor_id);
    //             free(hashtable_can->array[i]);
    //             hashtable_can->array[i] = tmp;
    //         }
    //         free(hashtable_can->array[i]);
    //     }
    // }
    // free(hashtable_can->array);
    // free(hashtable_can);
}
