#include <stdio.h>
#include <string.h>
#include <strings.h>

#include "../include/operations.h"

char *get(KeyValueStore *store, char *key)
{
    ++store->totalOperations;

    unsigned int keyIndex = hash_DJB2(key, store->capacity);

    for (int i = 0; i < store->capacity; ++i)
    {
        update_max_probe_length(store, i);

        unsigned int currentIndex = (keyIndex + i) % store->capacity;

        KeyValuePair *currEntry = &store->entries[currentIndex];

        if (currEntry->state == STATE_EMPTY)
        {
            return NULL;
        }

        if (currEntry->state == STATE_FILLED && 0 == strncmp(key, currEntry->key, sizeof(currEntry->key)))
        {
            return currEntry->value;
        }
    }

    return NULL;
}

int put(KeyValueStore *store, const char *key, const char *value, int should_append)
{
    ++store->totalOperations;

    float newUsage = ((float)store->count + 1) / (float)store->capacity * 100;

    if (newUsage > MAX_USAGE)
    {
        int result = resize_store(store);

        if (-1 == result)
        {
            return -1;
        }
    }

    if (strlen(key) + 1 > MAX_KEY_LEN)
    {
        printf("The length of key '%s' exceedes the max value of %d chars\n", key, MAX_KEY_LEN - 1);
        return -1;
    }

    if (strlen(value) + 1 > MAX_VALUE_LEN)
    {
        printf("The length of value '%s' exceedes the max value of %d chars\n", key, MAX_VALUE_LEN - 1);
        return -1;
    }

    unsigned int keyIndex = hash_DJB2(key, store->capacity);

    for (int i = 0; i < store->capacity; ++i)
    {
        update_max_probe_length(store, i);

        unsigned currentIndex = (keyIndex + i) % store->capacity;

        KeyValuePair *currEntry = &store->entries[currentIndex];

        if (currEntry->state != STATE_FILLED)
        {
            append_event(PUT_ARGS.id, key, value, should_append);

            snprintf(currEntry->key, sizeof(currEntry->key), "%s", key);
            snprintf(currEntry->value, sizeof(currEntry->value), "%s", value);
            currEntry->state = STATE_FILLED;
            ++store->count;
            return 0;
        }

        if (currEntry->state == STATE_FILLED && 0 == strncmp(key, currEntry->key, sizeof(currEntry->key)))
        {
            append_event(PUT_ARGS.id, key, value, should_append);

            snprintf(currEntry->value, sizeof(currEntry->value), "%s", value);
            return 0;
        }
    }

    printf("Store is full\n");
    return -1;
}

int delete(KeyValueStore *store, char *key, int should_append)
{
    ++store->totalOperations;

    unsigned int keyIndex = hash_DJB2(key, store->capacity);

    for (int i = 0; i < store->capacity; ++i)
    {
        update_max_probe_length(store, i);

        unsigned int currentIndex = (keyIndex + i) % store->capacity;

        KeyValuePair *currEntry = &store->entries[currentIndex];

        if (currEntry->state == STATE_EMPTY)
        {
            return -1;
        }

        if (0 == strncmp(key, currEntry->key, sizeof(currEntry->key)))
        {
            append_event(DEL_ARGS.id, key, NULL, should_append);

            currEntry->state = STATE_DELETED;
            --store->count;
            return 0;
        }
    }

    return -1;
}

void list_all(KeyValueStore *store)
{
    printf("Stored pairs:\n\n");

    for (int i = 0; i < store->capacity; ++i)
    {
        KeyValuePair *currEntry = &store->entries[i];

        if (currEntry->state != STATE_FILLED)
        {
            continue;
        }
        printf("%s:%s\n", currEntry->key, currEntry->value);
    }

    printf("\nCount: %d\n", store->count);
}

void print_menu()
{
    printf("\n");
    printf("(0) GET key\n");
    printf("(1) PUT key value\n");
    printf("(2) DELETE key\n");
    printf("(3) LIST\n");
    printf("(4) STATS\n");
    printf("(5) QUIT\n");
    printf("\n> ");
}

void update_max_probe_length(KeyValueStore *store, int index)
{
    if (index <= 0)
    {
        return;
    }

    ++store->totalCollisions;

    if (index <= store->maxProbeLength)
    {
        return;
    }

    store->maxProbeLength = index;
}

void print_stats(KeyValueStore *store)
{
    if (0 == store->totalOperations)
    {
        printf("No operations yet\n");
        return;
    }

    float usage = (float)store->count / (float)store->capacity * 100;
    float avarageProbs = (float)store->totalCollisions / (float)store->totalOperations;

    printf("Usage: %.0f%%\n", usage);
    printf("Average Probes: %.2f\n", avarageProbs);
    printf("Total Operations: %d\n", store->totalOperations);
    printf("Total Collisions: %d\n", store->totalCollisions);
    printf("Max Probe Length: %d\n", store->maxProbeLength);
}

int is_command(const char *command, const CommandDef values)
{
    return 0 == strcasecmp(command, values.id) || 0 == strcasecmp(command, values.name);
}