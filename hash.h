#pragma once

typedef enum {
    Entry_Variable,
} Entry_Type;

typedef struct Entry {
    u_int32_t key;
    Entry_Type type;
    void *value;
} Entry;

typedef struct Map {
    int64_t capacity;
    int64_t count;
    Entry *entries;
} Map;

Entry *new_entry(u_int32_t key, void *value) {
    Entry *entry = calloc(1, sizeof(Entry));
    entry->key = key;
    entry->value = value;
    return entry;
}

void free_entry(Entry entry) {
    switch (entry.type) {
        case Entry_Variable:
            ERR("NONNONNNONONOON\n")
            break;
        default:
            ERR("cant free entry type %d\n", entry.type)
            break;
    }
}

Map *new_map(int64_t capacity) {
    Map *map = calloc(1, sizeof(Map));
    map->capacity = capacity;
    map->count = 0;

    map->entries = calloc(capacity, sizeof(Entry));

    return map;
}

void free_map(Map *map) {
    for (int i = 0; i < map->capacity; i++) {
        free_entry(map->entries[i]);
    }
    free(map->entries);
    free(map);
}

u_int32_t hash(const char* key, int64_t length) {
    u_int32_t hash = 2166136261u;
    for (int i = 0; i < length; i++) {
        hash ^= (u_int8_t)key[i];
        hash *= 16777619;
    }
    return hash;
}

Entry *get_entry(Entry *entries, int64_t capacity, u_int32_t key) {
    u_int32_t index = key % capacity;

    while (1) {
        Entry *entry = &entries[index];
        if (entry->key == key || entry->value == NULL) {
            return entry;
        }
        index = (index + 1) % capacity;
    }
}

void adjust_capacity(Map *map) {
    map->capacity *= 2;
    Entry *entries = calloc(map->capacity, sizeof(Entry));

    for (int i = 0; i < map->capacity; i++) {
        Entry *entry = &map->entries[i];
        if (entry->key == 0) continue;

        Entry *dest = get_entry(map->entries, map->capacity, entry->key);
        dest->key = entry->key;
        dest->value = entry->value;
    }
    free(map->entries);
    map->entries = entries;
    return;
}

bool insert_entry(Map *map, u_int32_t key, Entry_Type type, void *value) {
    if (map->count + 1 > map->capacity * 0.75) {
        adjust_capacity(map);
    }

    Entry *entry = get_entry(map->entries, map->capacity, key);

    bool is_new = (entry == NULL);
    if (is_new) map->count++;

    entry->key = key;
    entry->type = type;
    entry->value = value;

    return is_new;
}

bool delete_entry(Map *map, u_int32_t key) {
    if (map->count == 0) return false;

    Entry *entry = get_entry(map->entries, map->capacity, key);
    if (entry->key == 0) return false;

    entry->key = -1;
    entry->value = NULL;
    return true;
}
