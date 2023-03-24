#pragma once

typedef struct {
    Node **nodes;
    int stmts_capacity;
    
    int program_counter;

    Variable *vars;
    int vars_index;
} Interpreter;

float strtofloat(const char *str, int len) {
    float total = 0;
    for (int i = 0; i < len - 1; i++) {
        total += str[i] - '0';
        total *= 10;
    }
    total += str[len - 1] - '0';
    return total;
}

AST_Value *ast_add(AST_Value *op1, AST_Value *op2){
    if (op1->type == Value_Number && op2->type == Value_Number) {
        int op1_len = strlen(op1->value);
        int op2_len = strlen(op2->value);
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
    } else ERR("cant subtract types `%s` and `%s`\n", find_ast_value_type(op1->type), find_ast_value_type(op2->type));
    return NULL;
}

AST_Value *ast_mult(AST_Value *op1, AST_Value *op2){
    if (op1->type == Value_Number && op2->type == Value_Number) {
        int op1_len = strlen(op1->value);
        int op2_len = strlen(op2->value);
        return new_ast_value(Value_Number, format_str(op1_len + op2_len + 1, "%g", strtofloat(op1->value, op1_len) * strtofloat(op2->value, op2_len)));
    } else ERR("cant multiply types `%s` and `%s`\n", find_ast_value_type(op1->type), find_ast_value_type(op2->type));
    return NULL;
}

AST_Value *ast_div(AST_Value *op1, AST_Value *op2){
    if (op1->type == Value_Number && op2->type == Value_Number) {
        int op1_len = strlen(op1->value);
        int op2_len = strlen(op2->value);
        return new_ast_value(Value_Number, format_str(op1_len + op2_len + 1, "%g", strtofloat(op1->value, op1_len) / strtofloat(op2->value, op2_len)));
    } else ERR("cant divide types `%s` and `%s`\n", find_ast_value_type(op1->type), find_ast_value_type(op2->type));
    return NULL;
}

AST_Value *ast_less(AST_Value *op1, AST_Value *op2) {
    if (op1->type == Value_Number && op2->type == Value_Number) {
        int op1_len = strlen(op1->value);
        int op2_len = strlen(op2->value);
        return new_ast_value(Value_Number, format_str(op1_len + op2_len, "%d", strtofloat(op1->value, op1_len) < strtofloat(op2->value, op2_len)));
    } else ERR("cant do less than on types `%s` and `%s`\n", find_ast_value_type(op1->type), find_ast_value_type(op2->type));
    return NULL;
}

AST_Value *ast_less_equal(AST_Value *op1, AST_Value *op2) {
    if (op1->type == Value_Number && op2->type == Value_Number) {
        int op1_len = strlen(op1->value);
        int op2_len = strlen(op2->value);
        return new_ast_value(Value_Number, format_str(op1_len + op2_len, "%d", strtofloat(op1->value, op1_len) <= strtofloat(op2->value, op2_len)));
    } else ERR("cant do less or equal to on types `%s` and `%s`\n", find_ast_value_type(op1->type), find_ast_value_type(op2->type));
    return NULL;
}
 
AST_Value *ast_greater(AST_Value *op1, AST_Value *op2) {
    if (op1->type == Value_Number && op2->type == Value_Number) {
        int op1_len = strlen(op1->value);
        int op2_len = strlen(op2->value);
        return new_ast_value(Value_Number, format_str(op1_len + op2_len, "%d", strtofloat(op1->value, op1_len) > strtofloat(op2->value, op2_len)));
    } else ERR("cant do greater than on types `%s` and `%s`\n", find_ast_value_type(op1->type), find_ast_value_type(op2->type));
    return NULL;
}

AST_Value *ast_greater_equal(AST_Value *op1, AST_Value *op2) {
    if (op1->type == Value_Number && op2->type == Value_Number) {
        int op1_len = strlen(op1->value);
        int op2_len = strlen(op2->value);
        return new_ast_value(Value_Number, format_str(op1_len + op2_len, "%d", strtofloat(op1->value, op1_len) >= strtofloat(op2->value, op2_len)));
    } else ERR("cant do greater or equal to on types `%s` and `%s`\n", find_ast_value_type(op1->type), find_ast_value_type(op2->type));
    return NULL;
}

AST_Value *ast_equal(AST_Value *op1, AST_Value *op2) {
    int op1_len = strlen(op1->value);
    int op2_len = strlen(op2->value);
    return new_ast_value(Value_Number, format_str(op1_len + op2_len, "%d", !strcmp(op1->value, op2->value)));
}

AST_Value *eval_node(Node *n, Interpreter *interpreter) {
    if (n == NULL) {
        ERR("can't evaluate null node\n");
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
    } else if (n->type == AST_Equal) {
        AST_Value *op1 = eval_node(n->left, interpreter);
        AST_Value *op2 = eval_node(n->right, interpreter);
        AST_Value *result = ast_equal(op1, op2);
        free_ast_value(op1);
        free_ast_value(op2);
        return result;
    } else ERR("cant evaluate node type `%s`\n", find_ast_type(n->type));
    return NULL;
}

void do_statement(Node *n, Interpreter *interpreter) {
    switch (n->type) {
        case AST_Print:;
            ASSERT((n->left->type == AST_Literal || IS_AST_MATH_OP(n->left->type) || n->left->type == AST_Identifier), "Can't print `%s`\n", find_ast_type(n->left->type));
            AST_Value *print = eval_node(n->left, interpreter);
            printf("%s\n", print->value);
            free(print->value);
            free(print);
            break;
        case AST_Var_Assign:;
            char *var_name = n->value->value;
            if (check_variable(var_name, interpreter->vars, interpreter->vars_index)) ERR("cant assign `%s` multiple times\n", var_name);
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
            free_ast_value(interpreter->vars[var.index].value);
            interpreter->vars[var.index].value = new_val;
            break;}
        case AST_While:;{
            AST_Value *expr = eval_node(n->left, interpreter);
            if (!strncmp(expr->value, "0", 1)) {
                interpreter->program_counter = n->jump_index;
            }
            free_ast_value(expr);
            break;}
        default: ERR("Unsupported statement type `%s`\n", find_ast_type(n->type));
    }
}

void interpret(Interpreter *interpreter) {
    while (interpreter->program_counter < interpreter->stmts_capacity) {
        do_statement(interpreter->nodes[interpreter->program_counter], interpreter);
        interpreter->program_counter++;
    }
}
