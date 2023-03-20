#pragma once

typedef enum {
    Value_String,
    Value_Number,
} AST_Value_Type;

char *find_ast_value_type(int type) {
    switch (type) {
        case Value_Number: return "Value_Number";
        case Value_String: return "Value_String";
        default: ERR("unknown ast value type `%d`", type);
    }
    return "unreachable";
}

typedef struct AST_Value {
    int type;
    char *value;
} AST_Value;

typedef enum{
    Var_Str,
    Var_Num,
} Var_Type;

typedef struct Variable {
    Var_Type type;
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
    ERR("variable `%s` not found\n", var_name);
    return (Variable) {0};
}
