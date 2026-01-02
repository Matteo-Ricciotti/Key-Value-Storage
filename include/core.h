#ifndef CORE_H
#define CORE_H

#define MAX_CMD_LEN 8
#define MAX_KEY_LEN 128
#define MAX_VALUE_LEN 256

#define STATE_EMPTY 0
#define STATE_FILLED 1
#define STATE_DELETED 2

#define STORE_LOCAL_PATH "store.db"
#define EVENTS_LOCAL_PATH "events.log"

typedef struct
{
    char key[MAX_KEY_LEN];
    char value[MAX_VALUE_LEN];
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

typedef struct
{
    char id[2];
    char name[MAX_CMD_LEN];
} CommandDef;

extern const CommandDef GET_ARGS;
extern const CommandDef PUT_ARGS;
extern const CommandDef DEL_ARGS;
extern const CommandDef LIST_ARGS;
extern const CommandDef STATS_ARGS;
extern const CommandDef QUIT_ARGS;

int init_store(KeyValueStore *store, int initialCapacity);
int resize_store(KeyValueStore *store);
int save_store(KeyValueStore *store);
int load_store(KeyValueStore *store);
int append_event(const char *event, const char *key, const char *value, int should_append);
int replay_events(KeyValueStore *store);

unsigned int hash_DJB2(const char *key, int maxEntries);

#endif