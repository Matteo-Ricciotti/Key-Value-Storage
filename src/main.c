#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "../include/operations.h"

#define MAX_INPUT_LEN 512
#define MAX_ENTRIES 5

int main()
{
    KeyValueStore store;

    char input[MAX_INPUT_LEN];

    char command[MAX_CMD_LEN];
    char key[MAX_KEY_LEN];
    char value[MAX_VALUE_LEN];

    int result = init_store(&store, MAX_ENTRIES);

    if (-1 == result)
    {
        return 1;
    }

    load_store(&store);
    replay_events(&store);

    printf("\nWelcome to the Key-Value Store!\n");

    // Main command loop
    while (1)
    {
        print_menu();

        fgets(input, sizeof(input), stdin);

        sscanf(input, "%s", command);

        if (is_command(command, GET_ARGS))
        {
            int inputWords = sscanf(input, "%s %s", command, key);

            if (inputWords != 2)
            {
                printf("Usage: GET <key>\n");
                continue;
            }

            char *res = get(&store, key);

            if (NULL == res)
            {
                printf("Key not found\n");
                continue;
            }

            printf("%s\n", res);

            continue;
        }

        if (is_command(command, PUT_ARGS))
        {
            int inputWords = sscanf(input, "%s %s %s", command, key, value);

            if (inputWords != 3)
            {
                printf("Usage: PUT <key> <value>\n");
                continue;
            }

            int result = put(&store, key, value, 1);

            if (-1 == result)
            {
                continue;
            }

            printf("Value saved!\n");
            continue;
        }

        if (is_command(command, DEL_ARGS))
        {
            int inputWords = sscanf(input, "%s %s", command, key);

            if (inputWords != 2)
            {
                printf("Usage: DELETE <key>\n");
                continue;
            }

            int result = delete(&store, key, 1);

            if (-1 == result)
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
            printf("\nGoodbye!\n");
            break;
        }

        printf("Invalid action\n");
    }

    save_store(&store);
    remove(EVENTS_LOCAL_PATH);

    free(store.entries);
    store.entries = NULL;

    return 0;
}