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

typedef enum {
    Value_Number,
    Value_String,
    Value_Identifier,
    Value_Array,
    Value_Function_Args,
} Value_Type;

char *find_ast_value_type(Value_Type type) {
    switch (type) {
        case Value_Number: return "Value_Number";
        case Value_String: return "Value_String";
        case Value_Identifier: return "Value_Identifier";
        case Value_Array: return "Value_Array";
        case Value_Function_Args: return "Value_Function_Args";
        default: ERR("unknown value type `%d`", type)
    }
    return "unreachable";
}

typedef struct AST_Value {
    Value_Type type;
    void *value;
    bool mutable;
    u_int32_t hash;
} AST_Value;

typedef struct Variable {
    char *name;
    AST_Value *value;
    int64_t index;
} Variable;

void free_ast_value(AST_Value *value) {
    if (value == NULL) return;
    if (value->type == Value_Array) {
        int64_t arr_len = NUM(value[0].value);
        for (int j = 0; j < arr_len; j++) {
            if (value[j].value != NULL) free(value[j].value);
            value[j].value = NULL;
        }
    }
    free(value->value);
    free(value);
}

Entry *new_entry(u_int32_t key, void *value) {
    Entry *entry = calloc(1, sizeof(Entry));
    entry->key = key;
    entry->value = value;
    return entry;
}

void free_entry(Entry entry) {
    switch (entry.type) {
        case Entry_Variable:
            free(entry.value);
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
    Entry *tombstone = NULL;

    while (1) {
        Entry *entry = &entries[index];
        if (entry->key == 0) {
            if (entry->value == NULL) {
                return tombstone != NULL ? tombstone : entry;
            } else {
                if (tombstone == NULL) tombstone = entry;
            }
        } else if (entry->key == key) {
            return entry;
        }
        index = (index + 1) % capacity;
    }
}

void adjust_capacity(Map *map) {
    map->capacity *= 2;
    Entry *entries = calloc(map->capacity, sizeof(Entry));

    map->count = 0;
    for (int i = 0; i < map->capacity; i++) {
        Entry *entry = &map->entries[i];
        if (entry->key == 0) continue;

        Entry *dest = get_entry(map->entries, map->capacity, entry->key);
        dest->key = entry->key;
        dest->value = entry->value;
        map->count++;
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
    if (is_new && (entry->value == NULL)) map->count++;

    entry->key = key;
    entry->type = type;
    entry->value = value;

    return is_new;
}

bool delete_entry(Map *map, u_int32_t key) {
    if (map->count < 0) return false;

    Entry *entry = get_entry(map->entries, map->capacity, key);
    if (entry->key == 0) return false;
    //free_entry(*entry);
    entry->key = 0;
    entry->value = NULL;
    return true;
}

void print_map(Map *map) {
    for (int i = 0; i < map->capacity; i++) {
        if (map->entries[i].key > 0) {
            printf("at index %d, there is an entry named %s\n", i, ((Variable*)map->entries[i].value)->name);
        }
    }
}
