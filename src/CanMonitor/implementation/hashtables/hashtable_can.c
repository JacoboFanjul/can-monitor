#include "hashtable_can.h"
#include "hashtable_common.h"

/*
 * htcan_create() - If the size is a positive integer, allocate the requisite
 * memory for a new hashtable and its indexed array. Assign the size of the
 * array in ht->size.
 * size: The size of the hashtable's array.
 *
 * Return: A pointer to the newly allocated Hashtable. If size is zero or a
 * negative number, return NULL. If a memory allocation fails, return NULL.
 */
HashTableCan *htcan_create(unsigned int size)
{
    HashTableCan *ht;

    if (size < 1)
    {
        return NULL;
    }

    ht = malloc(sizeof(HashTableCan));
    if (ht == NULL)
    {
        return (NULL);
    }

    ht->array = (ListSensorIds **)malloc(size * sizeof(ListSensorIds));
    if (ht->array == NULL)
    {
        return (NULL);
    }
    memset(ht->array, 0, size * sizeof(ListSensorIds));

    ht->size = size;

    return ht;
}

/*
 * htcan_put() - Allocates memory for a new node and calls the node_handler
 * function to either insert the node if the key does not exist, or update
 * a prexisting node in the case that it has the same key as that passed
 * to this function.
 * @key: The key to add to the hash table.
 * @sensorgroup: The corresponding sensorgroup to add to the node.
 *
 * Return: 1 if memory allocation fails, and 0 if the function returns
 * successfully.
 */
int htcan_put(HashTableCan *hashtable_can, const char *key, char *value)
{
    ListSensorIds *node;

    if (hashtable_can == NULL)
    {
        return 1;
    }
    node = malloc(sizeof(ListSensorIds));
    if (node == NULL)
    {
        return 1;
    }

    node->key = strdup(key);
    node->sensor_id = strdup(value);

    node_handler_can(hashtable_can, node);

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
void node_handler_can(HashTableCan *hashtable_can, ListSensorIds *node)
{
    unsigned int i = hash(node->key, hashtable_can->size);
    ListSensorIds *tmp = hashtable_can->array[i];

    if (hashtable_can->array[i] != NULL)
    {
        tmp = hashtable_can->array[i];
        while (tmp != NULL)
        {
            if (strcmp(tmp->key, node->key) == 0)
            {
                break;
            }
            tmp = tmp->next;
        }
        if (tmp == NULL)
        {
            node->next = hashtable_can->array[i];
            hashtable_can->array[i] = node;
        }
        else
        {
            free(tmp->sensor_id);
            tmp->sensor_id = node->sensor_id;
            free(node->sensor_id);
            free(node->key);
            free(node);
        }
    }
    else
    {
        node->next = NULL;
        hashtable_can->array[i] = node;
    }
}

/*
 * htcan_get() - Traverse the list that is at the corresponding array location in
 * the hashtable. If a node with the same key is found as that passed to this
 * function, then return the sensorgroup of that node. Otherwise, return NULL,
 * indicating there is no node with the key passed. Before returning, free
 * the copy of the string `key_cp`.
 * @hashtable: The hashtable in which to search for the data.
 * @key: The key to search the hashtable for.
 *
 * Return: The sensorgroup that corresponds to the key if it is found, and NULL
 * otherwise. If the hashtable is NULL, return NULL.
 */
char * htcan_get(HashTableCan *hashtable_can, const char *key)
{
    char *key_cp;
    unsigned int i;
    ListSensorIds *tmp;

    if (hashtable_can == NULL)
    {
        return NULL;
    }
    key_cp = strdup(key);
    i = hash(key, hashtable_can->size);
    tmp = hashtable_can->array[i];

    while (tmp != NULL)
    {
        if (strcmp(tmp->key, key_cp) == 0)
        {
            break;
        }
        tmp = tmp->next;
    }
    free(key_cp);

    if (tmp == NULL)
    {
        return NULL;
    }
    return tmp->sensor_id;
}

char * htcan_delete(HashTableCan *hashtable_can, const char *key)
{
    unsigned int i;
    ListSensorIds *sensor_ids;

    if (hashtable_can == NULL)
    {
        return NULL;
    }
    i = hash(key, hashtable_can->size);
    sensor_ids = hashtable_can->array[i];
    
    ListSensorIds *tmp = sensor_ids;
    ListSensorIds *prev = NULL;

    if (tmp == NULL)
    {
        return NULL;
    }

    if (strcmp(tmp->key, key) == 0)
    {
        char *sensor_id = tmp->sensor_id;
        hashtable_can->array[i] = tmp->next;
        free(tmp);
        return sensor_id;
    }

    while (tmp != NULL && strcmp(tmp->key, key) != 0)
    {
        prev = tmp;
        tmp = tmp->next;
    }

    if (tmp == NULL)
    {
        return NULL;
    }

    prev->next = tmp->next;
    char *sensor_id = tmp->sensor_id;
    free(tmp);
    return sensor_id;
}

/*
 * htcan_free() - Free the items in a hashtable. Iterate through the hashtable's
 * array. If it is a linked list, then traverse the list and free all the
 * node's attributes and then the node itself. When the end of the list is
 * reached, free the array item itself (i.e., `hashtable->array[i]`). Finally,
 * when all the array items are freed, free the hashtable array pointer and
 * the hashtable itself.
 * @hashtable: The hashtable to free.
 *
 * Return: If the hashtable is NULL, return because there is nothing to free.
 */
void htcan_free(HashTableCan *hashtable_can)
{
    ListSensorIds *tmp;
    unsigned int i;

    if (hashtable_can == NULL)
    {
        return;
    }

    for (i = 0; i < hashtable_can->size; ++i)
    {
        if (hashtable_can->array[i] != NULL)
        {
            /* Traverse the list and free the nodes. */
            while (hashtable_can->array[i] != NULL)
            {
                tmp = hashtable_can->array[i]->next;
                free(hashtable_can->array[i]->key);
                free(hashtable_can->array[i]->sensor_id);
                free(hashtable_can->array[i]);
                hashtable_can->array[i] = tmp;
            }
            free(hashtable_can->array[i]);
        }
    }
    free(hashtable_can->array);
    free(hashtable_can);
}
