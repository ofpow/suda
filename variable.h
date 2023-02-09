#pragma once

typedef enum{
    Var_Str,
    Var_Num,
} Var_Type;

typedef struct Variable {
    Var_Type type;
    char *name;
    char *value;
    int index;
} Variable;

Variable get_var(const char *var_name, Variable *vars, int vars_index) {
    for (int i = 0; i < vars_index; i++) {
        if (!strcmp(var_name, vars[i].name)) {
            return vars[i];
        }
    }
    ERR("variable `%s` not found\n", var_name);
    return (Variable) {0};
}
