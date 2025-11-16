#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

// Constants
#define MAX_INPUT_LEN 255
#define MAX_CMD_LEN 12
#define MAX_KEY_LEN 24
#define MAX_VALUE_LEN 255

#define MAX_ENTRIES 5
#define MAX_USAGE 70
#define STATE_EMPTY 0
#define STATE_FILLED 1
#define STATE_DELETED 2

// Data structures
typedef struct
{
    // + 1 for string null terminator \0
    char key[MAX_KEY_LEN + 1];
    char value[MAX_VALUE_LEN + 1];
    int state;
} KeyValuePair;

typedef struct
{
    KeyValuePair *entries;
    int capacity;
    int count;
    int totalCollisions; // Total times we had to probe
    int totalOperations; // Total get/put/delete operations
    int maxProbeLength;
} KeyValueStore;

// Function prototypes
int init_store(KeyValueStore *store, int initialCapacity);
int resize_store(KeyValueStore *store);
int put(KeyValueStore *store, const char *key, const char *value);
char *get(KeyValueStore *store, char *key);
int delete(KeyValueStore *store, char *key);
void list_all(KeyValueStore *store);
void print_menu();
unsigned int hash_DJB2(const char *key, int maxEntries);
void update_max_probe_length(KeyValueStore *store, int index);
void print_stats(KeyValueStore *store);

// Initialize the store (set all slots to empty, count to 0)
int init_store(KeyValueStore *store, int initialCapacity)
{
    KeyValuePair *pEntries = malloc(initialCapacity * sizeof(KeyValuePair));

    if (pEntries == NULL)
    {
        printf("Failed store entries memory allocation");
        return 0;
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

    return 1;
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
        return 0;
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
                strncpy_s(currEntry->key, sizeof(currEntry->key), pCurrentOldEntry->key, sizeof(pCurrentOldEntry->key));
                strncpy_s(currEntry->value, sizeof(currEntry->value), pCurrentOldEntry->value, sizeof(pCurrentOldEntry->value));
                currEntry->state = STATE_FILLED;
                ++store->count;
                break;
            }
        }
    }

    free(pOldEntries);
    pOldEntries = NULL;

    return 1;
}

// Insert or update a key-value pair
int put(KeyValueStore *store, const char *key, const char *value)
{
    ++store->totalOperations;

    float newUsage = ((float)store->count + 1) / (float)store->capacity * 100;

    if (newUsage > MAX_USAGE)
    {
        int success = resize_store(store);

        if (!success)
        {
            return 0;
        }
    }

    if (strnlen(key, MAX_KEY_LEN) > MAX_KEY_LEN)
    {
        printf("The length of key '%s' exceedes the max value of %d chars\n", key, MAX_KEY_LEN);
        return 0;
    }

    if (strnlen(value, MAX_KEY_LEN) > MAX_VALUE_LEN)
    {
        printf("The length of value '%s' exceedes the max value of %d chars\n", key, MAX_VALUE_LEN);
        return 0;
    }

    unsigned int keyIndex = hash_DJB2(key, store->capacity);

    for (int i = 0; i < store->capacity; ++i)
    {
        update_max_probe_length(store, i);

        unsigned currentIndex = (keyIndex + i) % store->capacity;

        KeyValuePair *currEntry = &store->entries[currentIndex];

        if (currEntry->state != STATE_FILLED)
        {
            strncpy_s(currEntry->key, sizeof(currEntry->key), key, sizeof(key));
            strncpy_s(currEntry->value, sizeof(currEntry->value), value, sizeof(value));
            currEntry->state = STATE_FILLED;
            ++store->count;
            return 1;
        }

        if (currEntry->state == STATE_FILLED && strncmp(key, currEntry->key, MAX_KEY_LEN) == 0)
        {
            strncpy_s(currEntry->value, sizeof(currEntry->value), value, sizeof(value));
            return 1;
        }
    }

    printf("Store is full\n");
    return 0;
}

// Retrieve value for a given key
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

        if (currEntry->state == STATE_FILLED && strncmp(key, currEntry->key, sizeof(currEntry->key)) == 0)
        {
            return currEntry->value;
        }
    }

    return NULL;
}

// Delete a key-value pair
int delete(KeyValueStore *store, char *key)
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
            return 0;
        }

        if (strncmp(key, currEntry->key, sizeof(currEntry->key)) == 0)
        {
            currEntry->state = STATE_DELETED;
            --store->count;
            return 1;
        }
    }

    return 0;
}

// Print all key-value pairs
void list_all(KeyValueStore *store)
{
    printf("Stored pairs:\n");

    for (int i = 0; i < store->capacity; ++i)
    {
        KeyValuePair *currEntry = &store->entries[i];

        if (currEntry->state != STATE_FILLED)
        {
            continue;
        }
        printf("Key: %s Value: %s\n", currEntry->key, currEntry->value);
    }

    printf("Count: %d\n", store->count);
}

void print_menu()
{
    printf("\n");
    printf("(1) GET key\n");
    printf("(2) PUT key value\n");
    printf("(3) DELETE key\n");
    printf("(4) LIST\n");
    printf("(5) STATS\n");
    printf("(6) QUIT\n");
}

int is_command(const char *command, const char values[2][MAX_CMD_LEN + 1])
{
    return stricmp(command, values[0]) == 0 || stricmp(command, values[1]) == 0;
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

int main()
{
    KeyValueStore store;
    char input[MAX_INPUT_LEN + 1];
    char command[MAX_CMD_LEN + 1];
    char key[MAX_KEY_LEN + 1];
    char value[MAX_VALUE_LEN + 1];

    const char GET_ARGS[2][MAX_CMD_LEN + 1] = {"1", "GET"};
    const char PUT_ARGS[2][MAX_CMD_LEN + 1] = {"2", "PUT"};
    const char DEL_ARGS[2][MAX_CMD_LEN + 1] = {"3", "DELETE"};
    const char LIST_ARGS[2][MAX_CMD_LEN + 1] = {"4", "LIST"};
    const char STATS_ARGS[2][MAX_CMD_LEN + 1] = {"5", "STATS"};
    const char QUIT_ARGS[2][MAX_CMD_LEN + 1] = {"6", "QUIT"};

    int success = init_store(&store, MAX_ENTRIES);

    if (!success)
    {
        return 1;
    }

    printf("Welcome to the Key-Value Store!\n");

    // Main command loop
    while (1)
    {
        print_menu();
        printf("\n> ");

        fgets(input, sizeof(input), stdin);

        sscanf_s(input, "%s", command, sizeof(command));

        if (is_command(command, GET_ARGS))
        {
            int inputWords = sscanf_s(input, "%s %s", command, sizeof(command), key, sizeof(key));

            if (inputWords != 2)
            {
                printf("Usage: GET <key>\n");
                continue;
            }

            char *res = get(&store, key);

            if (res == NULL)
            {
                printf("Key not found\n");
                continue;
            }

            printf("%s\n", res);

            continue;
        }

        if (is_command(command, PUT_ARGS))
        {
            int inputWords = sscanf_s(input, "%s %s %s", command, sizeof(command), key, sizeof(key), value, sizeof(value));

            if (inputWords != 3)
            {
                printf("Usage: PUT <key> <value>\n");
                continue;
            }

            int success = put(&store, key, value);

            if (!success)
            {
                continue;
            }

            printf("Value saved!\n");
            continue;
        }

        if (is_command(command, DEL_ARGS))
        {
            int inputWords = sscanf_s(input, "%s %s", command, sizeof(command), key, sizeof(key));

            if (inputWords != 2)
            {
                printf("Usage: DELETE <key>\n");
                continue;
            }

            int success = delete(&store, key);

            if (!success)
            {
                printf("Key not found\n");
                continue;
            }

            printf("Value deleted!\n");
            continue;
        }

        if (is_command(command, LIST_ARGS))
        {
            list_all(&store);
            continue;
        }

        if (is_command(command, STATS_ARGS))
        {
            print_stats(&store);
            continue;
        }

        if (is_command(command, QUIT_ARGS))
        {
            printf("Goodbye!\n");
            break;
        }

        printf("Invalid action\n");
    }

    free(store.entries);
    store.entries = NULL;

    return 0;
}