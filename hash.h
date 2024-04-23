#pragma once

#define ARRAY_LEN(_x) ((u_int32_t)_x)
#define ARRAY_SIZE(_x) ((u_int32_t)(_x >> 32))
#define MAKE_ARRAY_INFO(_capacity, _index) ((((u_int64_t)_capacity) << 32) | ((u_int64_t)_index))

typedef enum {
    Entry_Empty,
    Entry_Tombstone,
    Entry_AST_Variable,
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
    Value_Hash,
} Value_Type;

char *find_value_type(Value_Type type) {
    switch (type) {
        case Value_Number: return "Value_Number";
        case Value_String: return "Value_String";
        case Value_Identifier: return "Value_Identifier";
        case Value_Array: return "Value_Array";
        case Value_Function_Args: return "Value_Function_Args";
        case Value_Hash: return "Value_Hash";
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

typedef struct AST_Variable {
    char *name;
    AST_Value *value;
    int64_t index;
} AST_Variable;

typedef struct Value {
    Value_Type type;
    union {
        struct Value *array;
        string str;
        int64_t num;
    } val;
    bool mutable;
    u_int32_t hash;
} Value;

void free_value_array(Value *val);

typedef struct Variable {
    char *name;
    Value value;
} Variable;

void free_ast_value(AST_Value *value) {
    if (value == NULL) return;
    if (value->type == Value_Array) {
        int64_t arr_len = NUM(value[0].value);
        for (int i = 0; i < arr_len; i++) {
            if (value[i].value != NULL) free(value[i].value);
            value[i].value = NULL;
        }
    }
    free(value->value);
    value->value = NULL;
    free(value);
    value = NULL;
}

void free_entry(Entry entry) {
    switch (entry.type) {
        case Entry_Empty:
        case Entry_Tombstone:
            break;
        case Entry_Variable:;{
            Variable *var = entry.value;

            if (var->value.mutable == true && var->value.type == Value_String) 
                free(var->value.val.str.chars);
            if (var->value.type == Value_Array)
                free_value_array(var->value.val.array); 

            free(var);
            break;}
        case Entry_AST_Variable:;{
            AST_Variable *var = (AST_Variable*)entry.value;
            if (var->value != NULL) free_ast_value(var->value);
            free(var);
            break;}
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

void print_map(Map *map) {
    for (int i = 0; i < map->capacity; i++) {
        if (map->entries[i].key != 0) 
            printf("index %d not empty\n", i);
        else
            printf("index %d empty\n", i);
    }
}

Entry *get_entry(Entry *entries, int64_t capacity, u_int32_t key) {
    u_int32_t index = key & (capacity - 1);
    Entry *tombstone = NULL;

    while (1) {
        Entry *entry = &entries[index];
        if (entry->key == 0) {
            if (entry->type != Entry_Tombstone) {
                return tombstone != NULL ? tombstone : entry;
            } else {
                if (tombstone == NULL) tombstone = entry;
            }
        } else if (entry->key == key) {
            return entry;
        }
        index = (index + 1) & (capacity - 1);
    }
}

void adjust_capacity(Map *map, int64_t capacity) {
    Entry *entries = calloc(capacity, sizeof(Entry));

    map->count = 0;
    for (int i = 0; i < map->capacity; i++) {
        Entry *entry = &map->entries[i];
        if (entry->key == 0) continue;

        Entry *dest = get_entry(entries, capacity, entry->key);
        dest->key = entry->key;
        dest->type = entry->type;
        dest->value = entry->value;
        map->count++;
    }
    free(map->entries);
    map->entries = entries;
    map->capacity = capacity;
}

bool insert_entry(Map *map, u_int32_t key, Entry_Type type, void *value) {
    if ((int64_t)(map->count + 1) > (int64_t)(map->capacity * 0.75)) {
        adjust_capacity(map, map->capacity * 2);
    }

    Entry *entry = get_entry(map->entries, map->capacity, key);

    bool is_new = (entry->key == 0);
    if (is_new) map->count++;

    entry->key = key;
    entry->type = type;
    entry->value = value;

    return is_new;
}

bool delete_entry(Map *map, u_int32_t key) {
    if (map->count < 0) return false;

    Entry *entry = get_entry(map->entries, map->capacity, key);
    if (entry->key == 0) return false;
    free_entry(*entry);
    entry->key = 0;
    entry->value = NULL;
    entry->type = Entry_Tombstone;
    return true;
}
