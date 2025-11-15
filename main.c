#include <stdio.h>
#include <string.h>
#include <stdarg.h>

// Constants
#define MAX_INPUT_LEN 255
#define MAX_CMD_LEN 24
#define MAX_KEY_LEN 50
#define MAX_VALUE_LEN 200

#define MAX_ENTRIES 100

// Data structures
typedef struct
{
    // + 1 for string null terminator \0
    char key[MAX_KEY_LEN + 1];
    char value[MAX_VALUE_LEN + 1];
    int isFilled;
} KeyValuePair;

typedef struct
{
    KeyValuePair entries[MAX_ENTRIES];
    int count;
} KeyValueStore;

// Function prototypes
void init_store(KeyValueStore *store);
int put(KeyValueStore *store, const char *key, const char *value);
char *get(KeyValueStore *store, char *key);
int delete(KeyValueStore *store, char *key);
void list_all(KeyValueStore *store);
void print_menu();

// Initialize the store (set all slots to empty, count to 0)
void init_store(KeyValueStore *store)
{
    store->count = 0;

    for (int i = 0; i < MAX_ENTRIES; ++i)
    {
        store->entries[i].isFilled = 0;
    }
}

// Insert or update a key-value pair
// Returns: 0 on success, 1 on failure (store full, key too long, etc.)
int put(KeyValueStore *store, const char *key, const char *value)
{

    if (strnlen(key, MAX_KEY_LEN) > MAX_KEY_LEN)
    {
        printf("The length of key '%s' exceedes the max value of %d chars\n", key, MAX_KEY_LEN);
        return 1;
    }

    if (strnlen(value, MAX_KEY_LEN) > MAX_VALUE_LEN)
    {
        printf("The length of value '%s' exceedes the max value of %d chars\n", key, MAX_VALUE_LEN);
        return 1;
    }

    int foundResult = 0;
    int firstEmptyEntryIndex = -1;

    for (int i = 0; i < MAX_ENTRIES; ++i)
    {
        KeyValuePair *currEntry = &store->entries[i];

        if (firstEmptyEntryIndex < 0 && !currEntry->isFilled)
        {
            firstEmptyEntryIndex = i;
        }

        if (!currEntry->isFilled)
        {
            continue;
        }

        int hasFoundKey = strncmp(key, currEntry->key, MAX_KEY_LEN) == 0;

        if (!hasFoundKey)
        {
            continue;
        }

        foundResult = 1;
        strncpy_s(currEntry->value, sizeof(currEntry->value), value, MAX_VALUE_LEN);
        break;
    }

    if (!foundResult)
    {
        if (firstEmptyEntryIndex < 0)
        {
            printf("Store is full");
            return 1;
        }

        KeyValuePair *newEntry = &store->entries[firstEmptyEntryIndex];

        strncpy_s(newEntry->key, sizeof(newEntry->key), key, MAX_KEY_LEN);
        strncpy_s(newEntry->value, sizeof(newEntry->value), value, MAX_VALUE_LEN);
        newEntry->isFilled = 1;
        ++store->count;
        foundResult = 1;
    }

    return foundResult ? 0 : 1;
}

// Retrieve value for a given key
// Returns: pointer to value string if found, NULL if not found
char *get(KeyValueStore *store, char *key)
{
    char *foundValue = NULL;

    for (int i = 0; i < MAX_ENTRIES; ++i)
    {
        KeyValuePair *currEntry = &store->entries[i];

        if (!currEntry->isFilled)
        {
            continue;
        }

        int hasFoundKey = strncmp(key, currEntry->key, MAX_KEY_LEN) == 0;

        if (!hasFoundKey)
        {
            continue;
        }

        foundValue = currEntry->value;
        break;
    }

    return foundValue;
}

// Delete a key-value pair
// Returns: 0 if deleted, 1 if key not found
int delete(KeyValueStore *store, char *key)
{
    int foundKey = 0;

    for (int i = 0; i < MAX_ENTRIES; ++i)
    {
        KeyValuePair *currEntry = &store->entries[i];

        if (!currEntry->isFilled)
        {
            continue;
        }

        int hasFoundKey = strncmp(key, currEntry->key, MAX_KEY_LEN) == 0;

        if (!hasFoundKey)
        {
            continue;
        }

        foundKey = 1;
        currEntry->isFilled = 0;
        --store->count;
        break;
    }

    return foundKey ? 0 : 1;
}

// List all key-value pairs
void list_all(KeyValueStore *store)
{
    printf("Stored pairs:\n");

    for (int i = 0; i < MAX_ENTRIES; ++i)
    {
        KeyValuePair *currEntry = &store->entries[i];

        if (!currEntry->isFilled)
        {
            continue;
        }
        printf("Key: %s Value: %s\n", currEntry->key, currEntry->value);
    }

    printf("Count: %d\n", store->count);
}

// Print the command menu
void print_menu()
{
    printf("\n");
    printf("(1) GET key\n");
    printf("(2) PUT key value\n");
    printf("(3) DELETE key\n");
    printf("(4) LIST\n");
    printf("(5) QUIT\n");
}

int is_command(const char *command, const char values[2][MAX_CMD_LEN + 1])
{
    return stricmp(command, values[0]) == 0 || stricmp(command, values[1]) == 0;
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
    const char QUIT_ARGS[2][MAX_CMD_LEN + 1] = {"5", "QUIT"};

    init_store(&store);

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

            int res = put(&store, key, value);

            if (res == 1)
            {
                printf("Error while saving\n");
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

            int res = delete(&store, key);

            if (res == 1)
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

        if (is_command(command, QUIT_ARGS))
        {
            printf("Goodbye!\n");
            break;
        }

        printf("Invalid action\n");
    }

    return 0;
}