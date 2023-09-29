#pragma once

Function *get_func(Function **funcs, int funcs_size, char *func_name) {
    for (int i = 0; i < funcs_size; i++) {
        if (!strcmp(funcs[i]->name, func_name)) return funcs[i];
    }
    ERR("ERROR: function %s not found\n", func_name);
    return NULL;
}

Function *dup_function(Function *func) {
    Function *function = calloc(1, sizeof(Function));
    function->nodes = calloc(func->nodes_size, sizeof(Node*));
    function->args = calloc(func->arity, sizeof(AST_Value*));
    function->name = strdup(func->name);
    function->nodes_size = 0;
    function->arity = 0;
    for (int i = 0; i < func->nodes_size; i++) append(function->nodes, dup_node(func->nodes[i]), function->nodes_size, func->nodes_size)
    for (int i = 0; i < func->arity; i++) append(function->args, new_ast_value(func->args[i]->type, func->args[i]->value, func->args[i]->mutable), function->arity, func->arity)
    return function;
}
