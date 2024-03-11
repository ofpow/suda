#pragma once

AST_Function *get_func(AST_Function_Array *funcs, char *func_name, int64_t line) {
    for (int i = 0; i < funcs->index; i++) {
        if (!strcmp(funcs->data[i]->name, func_name)) return funcs->data[i];
    }
    ERR("ERROR on line %ld: function %s not found\n", line, func_name);
    return NULL;
}

int check_func(AST_Function_Array *funcs, char *func_name) {
    for (int i = 0; i < funcs->index; i++) {
        if (!strcmp(funcs->data[i]->name, func_name)) return i;
    }
    return -1;
}
