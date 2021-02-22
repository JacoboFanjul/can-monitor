#ifndef _HASHTABLE_COMMON_H_
#define _HASHTABLE_COMMON_H_

/*
 * @key: The key to hash
 *
 * @size: The size of the hashtable
 *
 * @return: An integer N like 0 <= N < @size
 * This integer represents the index of @key in an array of size @size
 */
unsigned int hash(const char *key, unsigned int size);

#endif