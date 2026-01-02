#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/operations.h"

/**
 * String literals include an implicit null terminator
 * The longest string "DELETE" has 6 chars + \0 = 7
 * MAX_CMD_LEN should be at least 7
 */
const CommandDef GET_ARGS = {"0", "GET"};
const CommandDef PUT_ARGS = {"1", "PUT"};
const CommandDef DEL_ARGS = {"2", "DELETE"};
const CommandDef LIST_ARGS = {"3", "LIST"};
const CommandDef STATS_ARGS = {"4", "STATS"};
const CommandDef QUIT_ARGS = {"5", "QUIT"};

int init_store(KeyValueStore *store, int initialCapacity)
{
    KeyValuePair *pEntries = malloc(initialCapacity * sizeof(KeyValuePair));

    if (pEntries == NULL)
    {
        printf("Failed store entries memory allocation");
        return -1;
    }

    store->entries = pEntries;
    store->capacity = initialCapacity;
    store->count = 0;
    store->totalCollisions = 0;
    store->totalOperations = 0;
    store->maxProbeLength = 0;

    for (int i = 0; i < initialCapacity; ++i)
    {
        store->entries[i].state = STATE_EMPTY;
    }

    return 0;
}

// Resize the store (recalculate slots hash)
int resize_store(KeyValueStore *store)
{
    int oldCapacity = store->capacity;
    int newCapacity = oldCapacity * 2;

    printf("Resizing store capacity to: %d\n", newCapacity);

    KeyValuePair *pOldEntries = store->entries;
    KeyValuePair *pNewEntries = calloc(newCapacity, sizeof(KeyValuePair));

    if (pNewEntries == NULL)
    {
        printf("Failed store entries memory expansion allocation");
        return -1;
    }

    store->entries = pNewEntries;
    store->capacity = newCapacity;
    store->count = 0;

    for (int i = 0; i < newCapacity; ++i)
    {
        store->entries[i].state = STATE_EMPTY;
    }

    for (int i = 0; i < oldCapacity; ++i)
    {
        KeyValuePair *pCurrentOldEntry = &pOldEntries[i];

        if (pCurrentOldEntry->state != STATE_FILLED)
        {
            continue;
        }

        unsigned newKeyIndex = hash_DJB2(pCurrentOldEntry->key, newCapacity);

        for (int j = 0; j < newCapacity; ++j)
        {
            unsigned currentIndex = (newKeyIndex + j) % newCapacity;

            KeyValuePair *currEntry = &store->entries[currentIndex];

            if (currEntry->state != STATE_FILLED)
            {
                snprintf(currEntry->key, sizeof(currEntry->key), "%s", pCurrentOldEntry->key);
                snprintf(currEntry->value, sizeof(currEntry->value), "%s", pCurrentOldEntry->value);
                currEntry->state = STATE_FILLED;
                ++store->count;
                break;
            }
        }
    }

    free(pOldEntries);
    pOldEntries = NULL;

    return 0;
}

unsigned int hash_DJB2(const char *key, int maxEntries)
{
    // unsigned means no negative int
    unsigned int hashValue = 5381;

    for (int i = 0; key[i] != '\0'; ++i)
    {
        // bit shift: << 5 === * 2^5 (32)
        hashValue = (hashValue << 5) + hashValue + key[i];
    }

    return hashValue % maxEntries;
}

int save_store(KeyValueStore *store)
{
    FILE *file = fopen(STORE_LOCAL_PATH, "w");

    if (NULL == file)
    {
        return 0;
    }

    for (int i = 0; i < store->capacity; ++i)
    {
        KeyValuePair *pCurrEntry = &store->entries[i];

        if (STATE_FILLED != pCurrEntry->state)
        {
            continue;
        }

        fprintf(file, "%s:%s\n", pCurrEntry->key, pCurrEntry->value);
    }

    fclose(file);

    return 0;
}

int load_store(KeyValueStore *store)
{
    FILE *file = fopen(STORE_LOCAL_PATH, "r");

    if (NULL == file)
    {
        return 0;
    }

    char line[MAX_KEY_LEN + MAX_VALUE_LEN];
    char key[MAX_KEY_LEN];
    char value[MAX_VALUE_LEN];

    while (fgets(line, sizeof(line), file))
    {
        // Read until ':' for key, rest for value
        sscanf(line, "%[^:]:%s", key, value);
        put(store, key, value, 0);
    };

    fclose(file);

    return 0;
}

int append_event(const char *event, const char *key, const char *value, int should_append)
{
    if (!should_append)
    {
        return 0;
    }

    FILE *file = fopen(EVENTS_LOCAL_PATH, "a");

    if (NULL == file)
    {
        return 0;
    }

    if (0 == strncmp(PUT_ARGS.id, event, sizeof(GET_ARGS.id)))
    {
        fprintf(file, "%s:%s:%s\n", event, key, value);
    }

    if (0 == strncmp(DEL_ARGS.id, event, sizeof(GET_ARGS.id)))
    {
        fprintf(file, "%s:%s\n", event, key);
    }

    fflush(file);

    fclose(file);

    return 0;
}

int replay_events(KeyValueStore *store)
{
    FILE *file = fopen(EVENTS_LOCAL_PATH, "r");

    if (NULL == file)
    {
        return 0;
    }

    char line[MAX_CMD_LEN + MAX_KEY_LEN + MAX_VALUE_LEN];
    char event[MAX_CMD_LEN];
    char key[MAX_KEY_LEN];
    char value[MAX_VALUE_LEN];

    while (fgets(line, sizeof(line), file))
    {
        // Read until ':' for event and key, rest for value
        sscanf(line, "%[^:]:%[^:]:%s", event, key, value);

        // Remove newline character
        key[strcspn(key, "\r\n")] = '\0';
        value[strcspn(value, "\r\n")] = '\0';

        if (0 == strncmp(PUT_ARGS.id, event, sizeof(event)))
        {
            put(store, key, value, 0);
            continue;
        }

        if (0 == strncmp(DEL_ARGS.id, event, sizeof(event)))
        {
            delete(store, key, 0);
            continue;
        }
    };

    fclose(file);

    return 0;
}