#pragma once

Function *get_func(Function **funcs, int64_t funcs_size, char *func_name, int64_t line) {
    for (int i = 0; i < funcs_size; i++) {
        if (!strcmp(funcs[i]->name, func_name)) return funcs[i];
    }
    ERR("ERROR on line %ld: function %s not found\n", line, func_name);
    return NULL;
}

int check_func(Function **funcs, int64_t funcs_size, char *func_name) {
    for (int i = 0; i < funcs_size; i++) {
        if (!strcmp(funcs[i]->name, func_name)) return i;
    }
    return -1;
}
