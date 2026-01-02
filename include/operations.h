#ifndef OPERATIONS_H
#define OPERATIONS_H

#include "core.h"

#define MAX_USAGE 70

char *get(KeyValueStore *store, char *key);
int put(KeyValueStore *store, const char *key, const char *value, int should_append);
int delete (KeyValueStore *store, char *key, int should_append);
void list_all(KeyValueStore *store);

void print_menu();
void update_max_probe_length(KeyValueStore *store, int index);
void print_stats(KeyValueStore *store);

int is_command(const char *command, const CommandDef values);

#endif