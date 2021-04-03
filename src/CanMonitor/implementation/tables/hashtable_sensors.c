#include "hashtable_sensors.h"
#include "hashtable_common.h"

/*
 * hts_create() - If the size is a positive integer, allocate the requisite
 * memory for a new hashtable and its indexed array. Assign the size of the
 * array in ht->size.
 * size: The size of the hashtable's array.
 *
 * Return: A pointer to the newly allocated Hashtable. If size is zero or a
 * negative number, return NULL. If a memory allocation fails, return NULL.
 */
HashTableSensors *hts_create(unsigned int size)
{
    HashTableSensors *ht;

    if (size < 1)
    {
        return NULL;
    }

    ht = malloc(sizeof(HashTableSensors));
    if (ht == NULL)
    {
        return (NULL);
    }

    ht->array = (ListSensors **)malloc(size * sizeof(ListSensors));
    if (ht->array == NULL)
    {
        return (NULL);
    }
    memset(ht->array, 0, size * sizeof(ListSensors));

    ht->size = size;

    return ht;
}

/*
 * hts_put() - Allocates memory for a new node and calls the node_handler
 * function to either insert the node if the key does not exist, or update
 * a prexisting node in the case that it has the same key as that passed
 * to this function.
 * @key: The key to add to the hash table.
 * @sensor: The corresponding sensor to add to the node.
 *
 * Return: 1 if memory allocation fails, and 0 if the function returns
 * successfully.
 */
int hts_put(HashTableSensors *hashtable_sensors, const char *key, sensor *sensor)
{
    ListSensors *node;

    if (hashtable_sensors == NULL)
    {
        return 1;
    }
    node = malloc(sizeof(ListSensors));
    if (node == NULL)
    {
        return 1;
    }

    node->key = strdup(key);
    node->sensor = sensor;

    node_handler_sensors(hashtable_sensors, node);

    return 0;
}

/*
 * node_handler_sensors() - If the index item is a linked list, traverse it to ensure
 * that there is not already an item with the key passed. If there is,
 * reassign the sensor of the prexisting node to the current instead of adding
 * the new node.
 * @hashtable: The hashtable of Lists.
 * @node: The linked list to add a node to or update.
 *
 * Return: Void.
 */
void node_handler_sensors(HashTableSensors *hashtable_sensors, ListSensors *node)
{
    unsigned int i = hash(node->key, hashtable_sensors->size);

    if (hashtable_sensors->array[i] != NULL)
    {
        ListSensors *tmp = hashtable_sensors->array[i];
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
            node->next = hashtable_sensors->array[i];
            hashtable_sensors->array[i] = node;
        }
        else
        {
            free(tmp->sensor);
            tmp->sensor = node->sensor;
            free(node->sensor);
            free(node->key);
            free(node);
        }
    }
    else
    {
        node->next = NULL;
        hashtable_sensors->array[i] = node;
    }
}

/*
 * hts_get() - Traverse the list that is at the corresponding array location in
 * the hashtable. If a node with the same key is found as that passed to this
 * function, then return the sensor of that node. Otherwise, return NULL,
 * indicating there is no node with the key passed. Before returning, free
 * the copy of the string `key_cp`.
 * @hashtable: The hashtable in which to search for the data.
 * @key: The key to search the hashtable for.
 *
 * Return: The sensor that corresponds to the key if it is found, and NULL
 * otherwise. If the hashtable is NULL, return NULL.
 */
sensor *hts_get(HashTableSensors *hashtable_sensors, const char *key)
{
    char *key_cp;
    unsigned int i;
    ListSensors *tmp;

    if (hashtable_sensors == NULL)
    {
        return NULL;
    }
    key_cp = strdup(key);
    i = hash(key, hashtable_sensors->size);
    tmp = hashtable_sensors->array[i];

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
    return tmp->sensor;
}

sensor * hts_delete(HashTableSensors *hashtable_sensors, const char *key)
{
    unsigned int i;
    ListSensors *sensors;

    if (hashtable_sensors == NULL)
    {
        return NULL;
    }
    i = hash(key, hashtable_sensors->size);
    sensors = hashtable_sensors->array[i];
    
    ListSensors *tmp = sensors;
    ListSensors *prev = NULL;

    if (tmp == NULL)
    {
        return NULL;
    }

    if (strcmp(tmp->key, key) == 0)
    {
        sensor *sens = tmp->sensor;
        hashtable_sensors->array[i] = tmp->next;
        free(tmp);
        return sens;
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
    sensor *sens = tmp->sensor;
    free(tmp);
    return sens;
}

/*
 * hts_free() - Free the items in a hashtable. Iterate through the hashtable's
 * array. If it is a linked list, then traverse the list and free all the
 * node's attributes and then the node itself. When the end of the list is
 * reached, free the array item itself (i.e., `hashtable->array[i]`). Finally,
 * when all the array items are freed, free the hashtable array pointer and
 * the hashtable itself.
 * @hashtable: The hashtable to free.
 *
 * Return: If the hashtable is NULL, return because there is nothing to free.
 */
void hts_free(HashTableSensors *hashtable_sensors)
{
    ListSensors *tmp;
    unsigned int i;

    if (hashtable_sensors == NULL)
    {
        return;
    }

    for (i = 0; i < hashtable_sensors->size; ++i)
    {
        if (hashtable_sensors->array[i] != NULL)
        {
            /* Traverse the list and free the nodes. */
            while (hashtable_sensors->array[i] != NULL)
            {
                tmp = hashtable_sensors->array[i]->next;
                free(hashtable_sensors->array[i]->key);
                free(hashtable_sensors->array[i]->sensor);
                free(hashtable_sensors->array[i]);
                hashtable_sensors->array[i] = tmp;
            }
            free(hashtable_sensors->array[i]);
        }
    }
    free(hashtable_sensors->array);
    free(hashtable_sensors);
}
