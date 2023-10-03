#pragma once

#define AST_IS_EVALUATABLE(type) ((type == AST_Literal || IS_AST_MATH_OP(type) || type == AST_Identifier || type == AST_At || type == AST_Function || type == AST_Function_Call || type == AST_Len))

typedef struct {
    Node **nodes;
    int stmts_capacity;
    
    int program_counter;

    Variable *vars;
    int vars_index;
    int vars_capacity;

    Function **funcs;
    int funcs_capacity;
} Interpreter;


AST_Value *do_statement(Node *n, Interpreter *interpreter);

char *format_array(AST_Value *array) {
    int arr_size = (int)strtofloat(array[0].value, strlen(array[0].value));
    int str_len = 3 + strlen(array[1].value);
    char *array_str = format_str(str_len, "[%s", array[1].value);
    for (int i = 2; i < arr_size; i++) {
        if (!array[i].value) ERR("array has null value\n")
        str_len += (strlen(array[i].value) + 2);
        array_str = realloc(array_str, str_len);
        strcat(array_str, ", ");
        strcat(array_str, array[i].value);
    }
    strcat(array_str, "]");
    return array_str;
}

AST_Value *ast_math(AST_Value *op1, AST_Value *op2, int op) {
    int op1_len = strlen(op1->value);
    int op2_len = strlen(op2->value);
    switch (op) {
        case AST_Add:
            if (op1->type == Value_Number && op2->type == Value_Number) return new_ast_value(Value_Number, format_str(op1_len + op2_len + 1, "%g", strtofloat(op1->value, op1_len) + strtofloat(op2->value, op2_len)), 1);
            else return new_ast_value(Value_String, format_str(op1_len + op2_len + 1, "%.*s%.*s", op1_len, op1->value, op2_len, op2->value), 1);
            break;
        case AST_Sub:
            ASSERT((op1->type == Value_Number && op2->type == Value_Number), "Cant subtract type %s and type %s\n", find_ast_type(op1->type), find_ast_type(op2->type))
            return new_ast_value(Value_Number, format_str(op1_len + op2_len + 1, "%g", strtofloat(op1->value, op1_len) - strtofloat(op2->value, op2_len)), 1);
            break;
        case AST_Mult:
            ASSERT((op1->type == Value_Number && op2->type == Value_Number), "Cant multiply type %s and type %s\n", find_ast_type(op1->type), find_ast_type(op2->type))
            return new_ast_value(Value_Number, format_str(op1_len + op2_len + 1, "%g", strtofloat(op1->value, op1_len) * strtofloat(op2->value, op2_len)), 1);
            break;
        case AST_Div:
            ASSERT((op1->type == Value_Number && op2->type == Value_Number), "Cant divide type %s and type %s\n", find_ast_type(op1->type), find_ast_type(op2->type))
            return new_ast_value(Value_Number, format_str(op1_len + op2_len + 1, "%g", strtofloat(op1->value, op1_len) / strtofloat(op2->value, op2_len)), 1);
            break;
        case AST_Less:
            ASSERT((op1->type == Value_Number && op2->type == Value_Number), "Cant less than type %s and type %s\n", find_ast_type(op1->type), find_ast_type(op2->type))
            return new_ast_value(Value_Number, format_str(op1_len + op2_len, "%d", strtofloat(op1->value, op1_len) < strtofloat(op2->value, op2_len)), 1);
            break;
        case AST_Less_Equal:
            ASSERT((op1->type == Value_Number && op2->type == Value_Number), "Cant less equal type %s and type %s\n", find_ast_type(op1->type), find_ast_type(op2->type))
            return new_ast_value(Value_Number, format_str(op1_len + op2_len, "%d", strtofloat(op1->value, op1_len) <= strtofloat(op2->value, op2_len)), 1);
            break;
        case AST_Greater:
            ASSERT((op1->type == Value_Number && op2->type == Value_Number), "Cant greater than type %s and type %s\n", find_ast_type(op1->type), find_ast_type(op2->type))
            return new_ast_value(Value_Number, format_str(op1_len + op2_len, "%d", strtofloat(op1->value, op1_len) > strtofloat(op2->value, op2_len)), 1);
            break;
        case AST_Greater_Equal:
            ASSERT((op1->type == Value_Number && op2->type == Value_Number), "Cant greater equal type %s and type %s\n", find_ast_type(op1->type), find_ast_type(op2->type))
            return new_ast_value(Value_Number, format_str(op1_len + op2_len, "%d", strtofloat(op1->value, op1_len) >= strtofloat(op2->value, op2_len)), 1);
            break;
        case AST_Is_Equal:
            return new_ast_value(Value_Number, format_str(op1_len + op2_len, "%d", !strcmp(op1->value, op2->value)), 1);
            break;
    }
    return NULL;
}

AST_Value *eval_node(Node *n, Interpreter *interpreter, int mutable) {
    if (n == NULL) {
        ERR("can't evaluate null node\n")
    } else if (n->type == AST_Literal) {
        if (mutable <= 0) {
            AST_Value *new_val = n->value;
            new_val->mutable = 0;
            return new_val;
        }
        return new_ast_value(n->value->type, strdup(n->value->value), 1);
    } else if (IS_AST_MATH_OP(n->type)) {
        AST_Value *op1 = eval_node(n->left, interpreter, 0);
        AST_Value *op2 = eval_node(n->right, interpreter, 0);
        AST_Value *result = ast_math(op1, op2, n->type);
        if (op1->mutable > 0) free_ast_value(op1);
        if (op2->mutable > 0) free_ast_value(op2);
        return result;
    } else if (n->type == AST_Identifier) {
        char *var_name = strdup(n->value->value);
        Variable var = get_var(var_name, interpreter->vars, interpreter->vars_index);
        free(var_name);
        if (mutable <= 0) {
            AST_Value *new_val = var.value;
            new_val->mutable = 0;
            return new_val;
        }
        if (var.value->type == Value_Array) {
            int arr_len = (int)strtofloat(var.value->value, strlen(var.value->value));
            AST_Value *array = calloc(arr_len + 1, sizeof(var.value[0]));
            for (int i = 0; i < arr_len; i++) {
                array[i].type = var.value[i].type;
                array[i].value = strdup(var.value[i].value);
            }
            return array;
        }
        return new_ast_value(var.value->type, strdup(var.value->value), 1);
    } else if (n->type == AST_Array) {
        if (mutable <= 0) return n->value;
        int arr_len = (int)strtofloat(n->value->value, strlen(n->value->value));
        AST_Value *array = calloc(arr_len, sizeof(n->value[0]));
        for (int i = 0; i < arr_len; i++) {
            array[i].type = n->value[i].type;
            array[i].value = strdup(n->value[i].value);
        }
        return array;
    } else if (n->type == AST_At) {
        int index = 0;
        if (n->left->type != AST_Literal) {
            AST_Value *val = eval_node(n->left, interpreter, 0);
            index = (int)strtofloat(val->value, strlen(val->value));
            if (val->mutable > 0) free_ast_value(val);
        } else index = (int)strtofloat(n->left->value->value, strlen(n->left->value->value));

        Variable var = get_var(n->value->value, interpreter->vars, interpreter->vars_index);

        if (var.value->type == Value_String) {
            return new_ast_value(Value_String, format_str(2, "%c", var.value->value[index - 1]), 1);
        }
        int arr_len = (int)strtofloat(var.value[0].value, strlen(var.value[0].value));
        if (index >= arr_len) ERR("Index %d is out of bounds for array %s, length %d\n", index, var.name, arr_len)

        if (var.value[index].type == Value_String) {
            int len = strlen(var.value[index].value);
            return new_ast_value(var.value[index].type, format_str(len - 1, "%.*s", len, var.value[index].value + 1), 1);
        } else if (var.value[index].type == Value_Number) {
            if (var.value[index].mutable <= 0) return &var.value[index];
            return new_ast_value(var.value[index].type, strdup(var.value[index].value), 1);
        } else ERR("Can't evalulate %s as part of array\n", find_ast_value_type(var.value[index].type))
    } else if (n->type == AST_Function_Call) {
        Function *function = get_func(interpreter->funcs, interpreter->funcs_capacity, n->value->value);
        Function *func = dup_func(function);
        for (int i = 0; i < func->nodes_size; i++) args_replace(func->nodes[i], func->args, n->left->value, func->arity);

        Interpreter intrprtr = {
            func->nodes,
            func->nodes_size,
            0,
            interpreter->vars,
            interpreter->vars_index,
            10,
            interpreter->funcs,
            interpreter->funcs_capacity
        };

        AST_Value *rtrn;
        while (intrprtr.program_counter < intrprtr.stmts_capacity) {
            rtrn = do_statement(intrprtr.nodes[intrprtr.program_counter], &intrprtr);
            if (rtrn != NULL) {
                free_function(func);
                return rtrn;
            }
            intrprtr.program_counter++;
        }
    } else if (n->type == AST_Len) {
        ASSERT((n->left->value->type == Value_Array || n->left->value->type == Value_String || n->left->value->type == Value_Identifier), "ERROR: cant do len on value type %s\n", find_ast_value_type(n->left->value->type))
        AST_Value *op = eval_node(n->left, interpreter, 0);
        int len;
        switch (op->type) {
            case Value_String:
                len = strlen(op->value);
                return new_ast_value(Value_Number, format_str(snprintf(NULL, 0, "%d", len) + 1, "%d", len), 1);
            case Value_Array:;
                int str_len = strlen(op[0].value);
                len = (int)strtofloat(op[0].value, str_len) - 1;
                return new_ast_value(Value_Number, format_str(str_len + 1, "%d", len), 1);
            default: ERR("ERROR: cant evaluate length of value type %s\n", find_ast_value_type(op->type))
        }
    } else ERR("cant evaluate node type `%s`\n", find_ast_type(n->type))
    return NULL;
}

AST_Value *do_statement(Node *n, Interpreter *interpreter) {
    switch (n->type) {
        case AST_Print:;
            if (!n->left) ERR("need something to print\n")
            ASSERT(AST_IS_EVALUATABLE(n->left->type), "Can't print `%s`\n", find_ast_type(n->left->type))
            AST_Value *print = eval_node(n->left, interpreter, 0);
            if (print == NULL) ERR("ERROR: tried to print a node that evaluated to null\n")

            if (print->type == Value_Array) {
                char *array = format_array(print);
                printf("%s\n", array);
                if (print->mutable > 0) free(print);
                free(array);
            } else {
                printf("%s\n", print->value);
                if (print->mutable > 0) free_ast_value(print);
            }

            break;
        case AST_Var_Assign:;
            char *var_name = n->value->value;
            if (check_variable(var_name, interpreter->vars, interpreter->vars_index)) 
                ERR("cant assign `%s` multiple times\n", var_name)
            AST_Value *var_val = eval_node(n->left, interpreter, 1);

            if (interpreter->vars_index >= interpreter->vars_capacity) {
                interpreter->vars_capacity *= 2;
                interpreter->vars = realloc(interpreter->vars, sizeof(Variable) * interpreter->vars_capacity);
            }
            interpreter->vars[interpreter->vars_index] = (Variable) { var_name, var_val, interpreter->vars_index };
            interpreter->vars_index++;
            break;
        case AST_If:;
            AST_Value *expr = eval_node(n->left, interpreter, 0);
            if (!strncmp(expr->value, "0", 1)) {
                interpreter->program_counter = n->jump_index;
            }
            if (expr->mutable > 0) free_ast_value(expr);
            break;
        case AST_Else:
            interpreter->program_counter = n->jump_index;
            break;
        case AST_Semicolon:
            if (interpreter->nodes[n->jump_index]->type == AST_While) {
                interpreter->program_counter = n->jump_index - 1;
            }
            break;
        case AST_Identifier:;{
            AST_Value *new_val = eval_node(n->left, interpreter, 1);
            char *var_name = strdup(n->value->value);
            Variable var = get_var(var_name, interpreter->vars, interpreter->vars_index);
            free(var_name);
            if (new_val->type == Value_Array) {
                int arr_len = (int)strtofloat(interpreter->vars[var.index].value[0].value, strlen(interpreter->vars[var.index].value[0].value));
                for (int i = 0; i < arr_len; i++) {
                    if (interpreter->vars[var.index].value[i].value != NULL) free(interpreter->vars[var.index].value[i].value);
                    interpreter->vars[var.index].value[i].value = NULL;
                }
                free(interpreter->vars[var.index].value);
                interpreter->vars[var.index].value = NULL;
            } else {
                free_ast_value(interpreter->vars[var.index].value);
            }
            interpreter->vars[var.index].value = new_val;
            break;}
        case AST_While:;{
            AST_Value *expr = eval_node(n->left, interpreter, 0);
            if (!strncmp(expr->value, "0", 1)) {
                interpreter->program_counter = n->jump_index;
            }
            if (expr->mutable > 0) free_ast_value(expr);
            break;}
        case AST_Break:
            interpreter->program_counter = n->jump_index;
            break;
        case AST_At:;{
            AST_Value *new_val;
            int index;
            if (n->right) {
                new_val = eval_node(n->right, interpreter, 1);
                index = (int)strtofloat(n->left->value->value, strlen(n->left->value->value));
            } else {
                new_val = eval_node(n->left->left, interpreter, 1);
                AST_Value *i = eval_node(n->left, interpreter, 1);
                index = (int)strtofloat(i->value, strlen(i->value));
                if (i->mutable > 0) free_ast_value(i);
            }

            char *var_name = strdup(n->value->value);
            Variable var = get_var(var_name, interpreter->vars, interpreter->vars_index);
            free(var_name);

            //if it was assigned after array was created, add quotes around value
            if (new_val->type == Value_String && new_val->value[0] != '"' && var.value->type == Value_Array) {
                char *temp = strdup(new_val->value);
                free(new_val->value);
                new_val->value = format_str((strlen(temp) + 3), "\"%s\"", temp);
                free(temp);
            }

            if (var.value->type == Value_String) {
                var.value->value[index - 1] = new_val->value[0];
                if (new_val->mutable > 0) free_ast_value(new_val);
                break;
            }
            free(var.value[index].value);
            interpreter->vars[var.index].value[index] = *new_val;
            free(new_val);

            break;}
        case AST_Return:
            return eval_node(n->left, interpreter, 1);
        case AST_Function_Call:;
            Function *function = get_func(interpreter->funcs, interpreter->funcs_capacity, n->value->value);
            Function *func = dup_func(function);
            for (int i = 0; i < func->nodes_size; i++) args_replace(func->nodes[i], func->args, n->left->value, func->arity);

            Interpreter intrprtr = {
                func->nodes,
                func->nodes_size,
                0,
                interpreter->vars,
                interpreter->vars_index,
                10,
                interpreter->funcs,
                interpreter->funcs_capacity
            };

            AST_Value *rtrn;
            while (intrprtr.program_counter < intrprtr.stmts_capacity) {
                rtrn = do_statement(intrprtr.nodes[intrprtr.program_counter], &intrprtr);
                if (rtrn != NULL) {
                    free_function(func);
                    free_ast_value(rtrn);
                    return NULL;
                }
                intrprtr.program_counter++;
            }
            break;
        default: ERR("Unsupported statement type `%s`\n", find_ast_type(n->type))
    }
    return NULL;
}

void interpret(Interpreter *interpreter) {
    while (interpreter->program_counter < interpreter->stmts_capacity) {
        do_statement(interpreter->nodes[interpreter->program_counter], interpreter);
        interpreter->program_counter++;
    }
}
