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
    char *value;
    int mutable;
} AST_Value;

typedef struct Variable {
    char *name;
    AST_Value *value;
    int index;
} Variable;

//TODO: variables as a hashmap
Variable get_var(const char *var_name, Variable *vars, int vars_index) {
    for (int i = 0; i < vars_index; i++) {
        if (!strcmp(var_name, vars[i].name)) {
            return vars[i];
        }
    }
    ERR("variable `%s` not found\n", var_name)
    return (Variable) {0};
}

int check_variable(const char *var_name, Variable *vars, int vars_index) {
    for (int i = 0; i < vars_index; i++) {
        if (!strcmp(var_name, vars[i].name)) {
            return 1;
        }
    }
    return 0;
}
