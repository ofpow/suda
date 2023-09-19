#pragma once

typedef struct {
    Node **nodes;
    int stmts_capacity;
    
    int program_counter;

    Variable *vars;
    int vars_index;
} Interpreter;

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

AST_Value *ast_add(AST_Value *op1, AST_Value *op2){
    if (op1->type == Value_Number && op2->type == Value_Number) {
        int op1_len = strlen(op1->value);
        int op2_len = strlen(op2->value);
        //TODO: with values over 1,000,000, %g goes to scientific notation
        return new_ast_value(Value_Number, format_str(op1_len + op2_len + 1, "%g", strtofloat(op1->value, op1_len) + strtofloat(op2->value, op2_len)));
    } else {
        int op1_len = strlen(op1->value);
        int op2_len = strlen(op2->value);
        return new_ast_value(Value_String, format_str(op1_len + op2_len + 1, "%.*s%.*s", op1_len, op1->value, op2_len, op2->value));
    }
}

AST_Value *ast_sub(AST_Value *op1, AST_Value *op2){
    if (op1->type == Value_Number && op2->type == Value_Number) {
        int op1_len = strlen(op1->value);
        int op2_len = strlen(op2->value);
        return new_ast_value(Value_Number, format_str(op1_len + op2_len + 1, "%g", strtofloat(op1->value, op1_len) - strtofloat(op2->value, op2_len)));
    } else ERR("cant subtract types `%s` and `%s`\n", find_ast_value_type(op1->type), find_ast_value_type(op2->type))
    return NULL;
}

AST_Value *ast_mult(AST_Value *op1, AST_Value *op2){
    if (op1->type == Value_Number && op2->type == Value_Number) {
        int op1_len = strlen(op1->value);
        int op2_len = strlen(op2->value);
        return new_ast_value(Value_Number, format_str(op1_len + op2_len + 1, "%g", strtofloat(op1->value, op1_len) * strtofloat(op2->value, op2_len)));
    } else ERR("cant multiply types `%s` and `%s`\n", find_ast_value_type(op1->type), find_ast_value_type(op2->type))
    return NULL;
}

AST_Value *ast_div(AST_Value *op1, AST_Value *op2){
    if (op1->type == Value_Number && op2->type == Value_Number) {
        int op1_len = strlen(op1->value);
        int op2_len = strlen(op2->value);
        return new_ast_value(Value_Number, format_str(op1_len + op2_len + 1, "%g", strtofloat(op1->value, op1_len) / strtofloat(op2->value, op2_len)));
    } else ERR("cant divide types `%s` and `%s`\n", find_ast_value_type(op1->type), find_ast_value_type(op2->type))
    return NULL;
}

AST_Value *ast_less(AST_Value *op1, AST_Value *op2) {
    if (op1->type == Value_Number && op2->type == Value_Number) {
        int op1_len = strlen(op1->value);
        int op2_len = strlen(op2->value);
        return new_ast_value(Value_Number, format_str(op1_len + op2_len, "%d", strtofloat(op1->value, op1_len) < strtofloat(op2->value, op2_len)));
    } else ERR("cant do less than on types `%s` and `%s`\n", find_ast_value_type(op1->type), find_ast_value_type(op2->type))
    return NULL;
}

AST_Value *ast_less_equal(AST_Value *op1, AST_Value *op2) {
    if (op1->type == Value_Number && op2->type == Value_Number) {
        int op1_len = strlen(op1->value);
        int op2_len = strlen(op2->value);
        return new_ast_value(Value_Number, format_str(op1_len + op2_len, "%d", strtofloat(op1->value, op1_len) <= strtofloat(op2->value, op2_len)));
    } else ERR("cant do less or equal to on types `%s` and `%s`\n", find_ast_value_type(op1->type), find_ast_value_type(op2->type))
    return NULL;
}
 
AST_Value *ast_greater(AST_Value *op1, AST_Value *op2) {
    if (op1->type == Value_Number && op2->type == Value_Number) {
        int op1_len = strlen(op1->value);
        int op2_len = strlen(op2->value);
        return new_ast_value(Value_Number, format_str(op1_len + op2_len, "%d", strtofloat(op1->value, op1_len) > strtofloat(op2->value, op2_len)));
    } else ERR("cant do greater than on types `%s` and `%s`\n", find_ast_value_type(op1->type), find_ast_value_type(op2->type))
    return NULL;
}

AST_Value *ast_greater_equal(AST_Value *op1, AST_Value *op2) {
    if (op1->type == Value_Number && op2->type == Value_Number) {
        int op1_len = strlen(op1->value);
        int op2_len = strlen(op2->value);
        return new_ast_value(Value_Number, format_str(op1_len + op2_len, "%d", strtofloat(op1->value, op1_len) >= strtofloat(op2->value, op2_len)));
    } else ERR("cant do greater or equal to on types `%s` and `%s`\n", find_ast_value_type(op1->type), find_ast_value_type(op2->type))
    return NULL;
}

AST_Value *ast_is_equal(AST_Value *op1, AST_Value *op2) {
    int op1_len = strlen(op1->value);
    int op2_len = strlen(op2->value);
    return new_ast_value(Value_Number, format_str(op1_len + op2_len, "%d", !strcmp(op1->value, op2->value)));
}

AST_Value *eval_node(Node *n, Interpreter *interpreter) {
    if (n == NULL) {
        ERR("can't evaluate null node\n")
    } else if (n->type == AST_Literal) {
        return new_ast_value(n->value->type, strdup(n->value->value));
    } else if (n->type == AST_Add) {
        AST_Value *op1 = eval_node(n->left, interpreter);
        AST_Value *op2 = eval_node(n->right, interpreter);
        AST_Value *result = ast_add(op1, op2);
        free_ast_value(op1);
        free_ast_value(op2);
        return result;
    } else if (n->type == AST_Sub) {
        AST_Value *op1 = eval_node(n->left, interpreter);
        AST_Value *op2 = eval_node(n->right, interpreter);
        AST_Value *result = ast_sub(op1, op2);
        free_ast_value(op1);
        free_ast_value(op2);
        return result;
    } else if (n->type == AST_Mult) {
        AST_Value *op1 = eval_node(n->left, interpreter);
        AST_Value *op2 = eval_node(n->right, interpreter);
        AST_Value *result = ast_mult(op1, op2);
        free_ast_value(op1);
        free_ast_value(op2);
        return result;
    } else if (n->type == AST_Div) {
        AST_Value *op1 = eval_node(n->left, interpreter);
        AST_Value *op2 = eval_node(n->right, interpreter);
        AST_Value *result = ast_div(op1, op2);
        free_ast_value(op1);
        free_ast_value(op2);
        return result;
    } else if (n->type == AST_Identifier) {
        char *var_name = strdup(n->value->value);
        Variable var = get_var(var_name, interpreter->vars, interpreter->vars_index);
        free(var_name);
        if (var.value->type == Value_Array) {
            int arr_len = (int)strtofloat(var.value->value, strlen(var.value->value));
            AST_Value *array = calloc(arr_len + 1, sizeof(var.value[0]));
            //memcpy(array, var.value, (arr_len * sizeof(var.value[0])));
            for (int i = 0; i < arr_len; i++) {
                array[i].type = var.value[i].type;
                array[i].value = strdup(var.value[i].value);
            }
            return array;
        }
        return new_ast_value(var.value->type, strdup(var.value->value));
    } else if (n->type == AST_Less) {
        AST_Value *op1 = eval_node(n->left, interpreter);
        AST_Value *op2 = eval_node(n->right, interpreter);
        AST_Value *result = ast_less(op1, op2);
        free_ast_value(op1);
        free_ast_value(op2);
        return result;
    } else if (n->type == AST_Less_Equal) {
        AST_Value *op1 = eval_node(n->left, interpreter);
        AST_Value *op2 = eval_node(n->right, interpreter);
        AST_Value *result = ast_less_equal(op1, op2);
        free_ast_value(op1);
        free_ast_value(op2);
        return result;
    } else if (n->type == AST_Greater) {
        AST_Value *op1 = eval_node(n->left, interpreter);
        AST_Value *op2 = eval_node(n->right, interpreter);
        AST_Value *result = ast_greater(op1, op2);
        free_ast_value(op1);
        free_ast_value(op2);
        return result;
    } else if (n->type == AST_Greater_Equal) {
        AST_Value *op1 = eval_node(n->left, interpreter);
        AST_Value *op2 = eval_node(n->right, interpreter);
        AST_Value *result = ast_greater_equal(op1, op2);
        free_ast_value(op1);
        free_ast_value(op2);
        return result;
    } else if (n->type == AST_Is_Equal) {
        AST_Value *op1 = eval_node(n->left, interpreter);
        AST_Value *op2 = eval_node(n->right, interpreter);
        AST_Value *result = ast_is_equal(op1, op2);
        free_ast_value(op1);
        free_ast_value(op2);
        return result;
    } else if (n->type == AST_Array) {
        int arr_len = (int)strtofloat(n->value->value, strlen(n->value->value));
        AST_Value *array = calloc(arr_len, sizeof(n->value[0]));
        //memcpy(array, n->value, (arr_len * sizeof(n->value[0])));
        for (int i = 0; i < arr_len; i++) {
            array[i].type = n->value[i].type;
            array[i].value = strdup(n->value[i].value);
        }
        return array;
    } else if (n->type == AST_At) {
        int index = 0;
        if (n->left->type != AST_Literal) {
            AST_Value *val = eval_node(n->left, interpreter);
            index = strtofloat(val->value, strlen(val->value));
            free_ast_value(val);
        } else index = (int)strtofloat(n->left->value->value, strlen(n->left->value->value));

        Variable var = get_var(n->value->value, interpreter->vars, interpreter->vars_index);

        if (var.value->type == Value_String) {
            return new_ast_value(Value_String, format_str(2, "%c", var.value->value[index - 1]));
        }
        int arr_len = (int)strtofloat(var.value[0].value, strlen(var.value[0].value));
        if (index >= arr_len) ERR("Index %d is out of bounds for array %s, length %d\n", index, var.name, arr_len)

        if (var.value[index].type == Value_String) {
            int len = strlen(var.value[index].value);
            return new_ast_value(var.value[index].type, format_str(len - 1, "%.*s", len, var.value[index].value + 1));
        } else if (var.value[index].type == Value_Number) {
            return new_ast_value(var.value[index].type, strdup(var.value[index].value));
        } else ERR("Can't evalulate %s as part of array\n", find_ast_value_type(var.value[index].type))
    } else ERR("cant evaluate node type `%s`\n", find_ast_type(n->type))
    return NULL;
}

void do_statement(Node *n, Interpreter *interpreter) {
    switch (n->type) {
        case AST_Print:;
            if (!n->left) ERR("need something to print\n")
            ASSERT((n->left->type == AST_Literal || IS_AST_MATH_OP(n->left->type) || n->left->type == AST_Identifier || n->left->type == AST_At), "Can't print `%s`\n", find_ast_type(n->left->type))
            AST_Value *print = eval_node(n->left, interpreter);

            if (print->type == Value_Array) {
                char *array = format_array(print);
                printf("%s\n", array);
                int arr_len = (int)strtofloat(print[0].value, strlen(print[0].value));
                for (int i = 0; i < arr_len; i++) {
                    if (print[i].value != NULL) free(print[i].value);
                    print[i].value = NULL;
                }
                free(print);
                free(array);
                //free_ast_value(print);
            } else {
                printf("%s\n", print->value);
                free_ast_value(print);
            }
            //printf("%s\n", print->value);

            break;
        case AST_Var_Assign:;
            char *var_name = n->value->value;
            if (check_variable(var_name, interpreter->vars, interpreter->vars_index)) 
                ERR("cant assign `%s` multiple times\n", var_name)
            AST_Value *var_val = eval_node(n->left, interpreter);
            interpreter->vars[interpreter->vars_index] = (Variable) { var_name, var_val, interpreter->vars_index };
            interpreter->vars_index++;
            break;
        case AST_If:;
            AST_Value *expr = eval_node(n->left, interpreter);
            if (!strncmp(expr->value, "0", 1)) {
                interpreter->program_counter = n->jump_index;
            }
            free(expr->value);
            free(expr);
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
            AST_Value *new_val = eval_node(n->left, interpreter);
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
            AST_Value *expr = eval_node(n->left, interpreter);
            if (!strncmp(expr->value, "0", 1)) {
                interpreter->program_counter = n->jump_index;
            }
            free_ast_value(expr);
            break;}
        case AST_At:;{
            AST_Value *new_val;
            int index;
            if (n->right) {
                new_val = eval_node(n->right, interpreter);
                index = (int)strtofloat(n->left->value->value, strlen(n->left->value->value));
            } else {
                new_val = eval_node(n->left->left, interpreter);
                AST_Value *i = eval_node(n->left, interpreter);
                index = (int)strtofloat(i->value, strlen(i->value));
                free_ast_value(i);
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
                free_ast_value(new_val);
                break;
            }
            free(var.value[index].value);
            interpreter->vars[var.index].value[index] = *new_val;
            free(new_val);

            break;}
        default: ERR("Unsupported statement type `%s`\n", find_ast_type(n->type))
    }
}

void interpret(Interpreter *interpreter) {
    while (interpreter->program_counter < interpreter->stmts_capacity) {
        do_statement(interpreter->nodes[interpreter->program_counter], interpreter);
        interpreter->program_counter++;
    }
}
