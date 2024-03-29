#pragma once

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

define_array(Node_Array, Node*);

typedef struct AST_Function {
    char *name;

    Node_Array nodes;

    int64_t arity;
    AST_Value **args;

    int64_t line;
} AST_Function;

define_array(AST_Function_Array, AST_Function*);

typedef struct Interpreter {
    Node_Array nodes;

    int64_t program_counter;

    Map *vars;

    Map *local_vars;

    AST_Function_Array funcs;

    bool auto_jump;
} Interpreter;

AST_Variable *get_var(Interpreter *interpreter, char *var_name, u_int32_t key, int64_t line, const char *file) {
    if (interpreter->local_vars != NULL) {
        Entry *entry = get_entry(interpreter->local_vars->entries, interpreter->local_vars->capacity, key);
        if (entry->value != NULL) return entry->value;
    }
    Entry *entry = get_entry(interpreter->vars->entries, interpreter->vars->capacity, key);
    if (entry->key == 0) ERR("ERROR in %s on line %ld: variable %s doesnt exist\n", file, line, var_name)

    return entry->value;
}

bool check_variable(Interpreter *interpreter, u_int32_t key) {
    if (interpreter->local_vars != NULL) {
        Entry *entry = get_entry(interpreter->local_vars->entries, interpreter->local_vars->capacity, key);
        if (entry->value != NULL) return true;
    }
    Entry *entry = get_entry(interpreter->vars->entries, interpreter->vars->capacity, key);
    if (entry->key == 0) return false;

    return true;
}
