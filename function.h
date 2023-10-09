#pragma once

Function *get_func(Function **funcs, int funcs_size, char *func_name) {
    for (int i = 0; i < funcs_size; i++) {
        if (!strcmp(funcs[i]->name, func_name)) return funcs[i];
    }
    ERR("ERROR: function %s not found\n", func_name);
    return NULL;
}

int check_func(Function **funcs, int funcs_size, char *func_name) {
    for (int i = 0; i < funcs_size; i++) {
        if (!strcmp(funcs[i]->name, func_name)) return i;
    }
    return -1;
}
