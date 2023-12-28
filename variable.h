#pragma once

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

typedef struct Node {
    int64_t type;
    AST_Value *value;

    struct Node *right;
    struct Node *left;

    int64_t jump_index;

    int64_t line;
    const char *file;

    struct Node **func_args;
    int64_t func_args_index;
    int64_t func_args_capacity;
} Node;

typedef struct Function {
    char *name;
    Node **nodes;
    int64_t nodes_size;

    int64_t arity;
    AST_Value **args;

    int64_t line;
} Function;

typedef struct {
    Node **nodes;
    int64_t stmts_capacity;

    int64_t program_counter;

    Map *vars;
    int64_t vars_index;
    int64_t vars_capacity;

    Map *local_vars;
    int64_t local_vars_index;
    int64_t local_vars_capacity;

    Function **funcs;
    int64_t funcs_capacity;

    int64_t auto_jump;
} Interpreter;

//TODO: variables as a hashmap
Variable *get_var(Interpreter *interpreter, char *var_name, u_int32_t key, int64_t line, const char *file) {
    if (interpreter->local_vars != NULL) {
        Entry *entry = get_entry(interpreter->local_vars->entries, interpreter->local_vars->capacity, key);
        if (entry->value != NULL) return entry->value;
    }
    Entry *entry = get_entry(interpreter->vars->entries, interpreter->vars->capacity, key);
    if (entry->key == 0) ERR("ERROR in %s on line %ld: variable %s doesnt exist\n", file, line, var_name)

    return entry->value;
}
//Variable get_var(const char *var_name, Interpreter *interpreter, int64_t line, const char *file) {
//    if (interpreter->local_vars != NULL) {
//        for (int i = 0; i < interpreter->local_vars_index; i++) {
//            if (!strcmp(var_name, interpreter->local_vars[i].name)) {
//                return interpreter->local_vars[i];
//            }
//        }
//    }
//    for (int i = 0; i < interpreter->vars_index; i++) {
//        if (!strcmp(var_name, interpreter->vars[i].name)) {
//            return interpreter->vars[i];
//        }
//    }
//    ERR("ERROR in %s on line %ld: variable `%s` not found\n", file, line, var_name)
//    return (Variable) {0};
//}

int check_variable(const char *var_name, Interpreter *interpreter) {
    return -1;
}
//    if (interpreter->local_vars != NULL) {
//        for (int i = 0; i < interpreter->local_vars_index; i++) {
//            if (!strcmp(var_name, interpreter->local_vars[i].name)) {
//                return i;
//            }
//        }
//    }
//    for (int i = 0; i < interpreter->vars_index; i++) {
//        if (!strcmp(var_name, interpreter->vars[i].name)) {
//            return i;
//        }
//    }
//    return -1;
//}
