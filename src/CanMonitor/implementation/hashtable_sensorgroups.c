#include "hashtable_sensorgroups.h"
#include "hashtable_common.h"

/*
 * htsg_create() - If the size is a positive integer, allocate the requisite
 * memory for a new hashtable and its indexed array. Assign the size of the
 * array in ht->size.
 * size: The size of the hashtable's array.
 *
 * Return: A pointer to the newly allocated Hashtable. If size is zero or a
 * negative number, return NULL. If a memory allocation fails, return NULL.
 */
HashTableSensorgroups *htsg_create(unsigned int size)
{
    HashTableSensorgroups *ht;

    if (size < 1)
    {
        return NULL;
    }

    ht = malloc(sizeof(HashTableSensorgroups));
    if (ht == NULL)
    {
        return (NULL);
    }

    ht->array = (ListSensorgroups **)malloc(size * sizeof(ListSensorgroups));
    if (ht->array == NULL)
    {
        return (NULL);
    }
    memset(ht->array, 0, size * sizeof(ListSensorgroups));

    ht->size = size;

    return ht;
}

/*
 * htsg_put() - Allocates memory for a new node and calls the node_handler
 * function to either insert the node if the key does not exist, or update
 * a prexisting node in the case that it has the same key as that passed
 * to this function.
 * @key: The key to add to the hash table.
 * @sensorgroup: The corresponding sensorgroup to add to the node.
 *
 * Return: 1 if memory allocation fails, and 0 if the function returns
 * successfully.
 */
int htsg_put(HashTableSensorgroups *hashtable_sensorgroups, const char *key, sensorgroup *sensorgroup)
{
    ListSensorgroups *node;

    if (hashtable_sensorgroups == NULL)
    {
        return 1;
    }
    node = malloc(sizeof(ListSensorgroups));
    if (node == NULL)
    {
        return 1;
    }

    node->key = strdup(key);
    node->sensorgroup = sensorgroup;

    node_handler_sensorgroups(hashtable_sensorgroups, node);

    return 0;
}

/*
 * node_handler_sensorgroups() - If the index item is a linked list, traverse it to ensure
 * that there is not already an item with the key passed. If there is,
 * reassign the sensorgroup of the prexisting node to the current instead of adding
 * the new node.
 * @hashtable: The hashtable of Lists.
 * @node: The linked list to add a node to or update.
 *
 * Return: Void.
 */
void node_handler_sensorgroups(HashTableSensorgroups *hashtable_sensorgroups, ListSensorgroups *node)
{
    unsigned int i = hash(node->key, hashtable_sensorgroups->size);
    ListSensorgroups *tmp = hashtable_sensorgroups->array[i];

    if (hashtable_sensorgroups->array[i] != NULL)
    {
        tmp = hashtable_sensorgroups->array[i];
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
            node->next = hashtable_sensorgroups->array[i];
            hashtable_sensorgroups->array[i] = node;
        }
        else
        {
            free(tmp->sensorgroup);
            tmp->sensorgroup = node->sensorgroup;
            free(node->sensorgroup);
            free(node->key);
            free(node);
        }
    }
    else
    {
        node->next = NULL;
        hashtable_sensorgroups->array[i] = node;
    }
}

/*
 * htsg_get() - Traverse the list that is at the corresponding array location in
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
sensorgroup *htsg_get(HashTableSensorgroups *hashtable_sensorgroups, const char *key)
{
    char *key_cp;
    unsigned int i;
    ListSensorgroups *tmp;

    if (hashtable_sensorgroups == NULL)
    {
        return NULL;
    }
    key_cp = strdup(key);
    i = hash(key, hashtable_sensorgroups->size);
    tmp = hashtable_sensorgroups->array[i];

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
    return tmp->sensorgroup;
}

sensorgroup * htsg_delete(HashTableSensorgroups *hashtable_sensorgroups, const char *key)
{
    unsigned int i;
    ListSensorgroups *sensorgroups;

    if (hashtable_sensorgroups == NULL)
    {
        return NULL;
    }
    i = hash(key, hashtable_sensorgroups->size);
    sensorgroups = hashtable_sensorgroups->array[i];
    
    ListSensorgroups *tmp = sensorgroups;
    ListSensorgroups *prev = NULL;

    if (tmp == NULL)
    {
        return NULL;
    }

    if (strcmp(tmp->key, key) == 0)
    {
        sensorgroup *sensgroup = tmp->sensorgroup;
        hashtable_sensorgroups->array[i] = tmp->next;
        free(tmp);
        return sensgroup;
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
    sensorgroup *sensgroup = tmp->sensorgroup;
    free(tmp);
    return sensgroup;
}

/*
 * htsg_free() - Free the items in a hashtable. Iterate through the hashtable's
 * array. If it is a linked list, then traverse the list and free all the
 * node's attributes and then the node itself. When the end of the list is
 * reached, free the array item itself (i.e., `hashtable->array[i]`). Finally,
 * when all the array items are freed, free the hashtable array pointer and
 * the hashtable itself.
 * @hashtable: The hashtable to free.
 *
 * Return: If the hashtable is NULL, return because there is nothing to free.
 */
void htsg_free(HashTableSensorgroups *hashtable_sensorgroups)
{
    ListSensorgroups *tmp;
    unsigned int i;

    if (hashtable_sensorgroups == NULL)
    {
        return;
    }

    for (i = 0; i < hashtable_sensorgroups->size; ++i)
    {
        if (hashtable_sensorgroups->array[i] != NULL)
        {
            /* Traverse the list and free the nodes. */
            while (hashtable_sensorgroups->array[i] != NULL)
            {
                tmp = hashtable_sensorgroups->array[i]->next;
                free(hashtable_sensorgroups->array[i]->key);
                free(hashtable_sensorgroups->array[i]->sensorgroup);
                free(hashtable_sensorgroups->array[i]);
                hashtable_sensorgroups->array[i] = tmp;
            }
            free(hashtable_sensorgroups->array[i]);
        }
    }
    free(hashtable_sensorgroups->array);
    free(hashtable_sensorgroups);
}
