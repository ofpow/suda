#pragma once

Function *get_func(Function **funcs, int funcs_size, char *func_name) {
    for (int i = 0; i < funcs_size; i++) {
        if (!strcmp(funcs[i]->name, func_name)) return funcs[i];
    }
    ERR("ERROR: function %s not found\n", func_name);
    return NULL;
}

Function *dup_func(Function *func) {
    Function *function = calloc(1, sizeof(Function));
    function->nodes = calloc(func->nodes_size, sizeof(Node*));
    function->args = calloc(func->arity, sizeof(AST_Value*));
    function->name = strdup(func->name);
    function->nodes_size = 0;
    function->arity = 0;
    for (int i = 0; i < func->nodes_size; i++) append(function->nodes, dup_node(func->nodes[i]), function->nodes_size, func->nodes_size)
    for (int i = 0; i < func->arity; i++) append(function->args, new_ast_value(func->args[i]->type, strdup(func->args[i]->value), 1), function->arity, func->arity)
    return function;
}

void args_replace(Node *node, AST_Value **args, AST_Value *call_args, int args_size) {
    if (node == NULL) return;
    if (node->left != NULL) args_replace(node->left, args, call_args, args_size);
    if (node->right != NULL) args_replace(node->right, args, call_args, args_size);
    if (node->value == NULL) return;

    for (int i = 0; i < args_size; i++) {
        if (!strcmp(node->value->value, args[i]->value)) {
            if (node->value->mutable > 0) free_ast_value(node->value);
            node->value = new_ast_value(call_args[i + 1].type, strdup(call_args[i + 1].value), 1);
            if (call_args[i + 1].type == Value_Identifier) {
                node->type = AST_Identifier;
            } else if (call_args[i + 1].type == Value_String || call_args[i + 1].type == Value_Number) {
                node->type = AST_Literal;
            } else ERR("ERROR: cant replace arg type %s\n", find_ast_value_type(call_args[i + 1].type))
            return;
        }
        //} else printf("not %s and %s\n", node->value->value, args[i]->value);
        //printf("value %s\n", node->value->value);
        //printf("type %s value %s\n", find_ast_value_type(call_args[i].type), call_args[i].value);
        //printf("type %s value %s\n", find_ast_value_type(args[i - 1]->type), args[i - 1]->value);
    }
}
