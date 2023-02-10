#pragma once

#define IS_AST_MATH_OP(expr) ((expr == AST_Add) || (expr == AST_Sub) || (expr == AST_Mult) || (expr == AST_Div))
#define VARS interpreter->vars, interpreter->vars_index

typedef struct {
    Node **nodes;
    int stmts_size;

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
        return new_ast_value(Value_Number, format_str(op1_len + op2_len, "%g", strtofloat(op1->value, op1_len) + strtofloat(op2->value, op2_len)));
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
        return new_ast_value(Value_Number, format_str(op1_len + op2_len, "%g", strtofloat(op1->value, op1_len) - strtofloat(op2->value, op2_len)));
    } else ERR("cant add types `%s` and `%s`\n", find_ast_value_type(op1->type), find_ast_value_type(op2->type));
    return NULL;
}

AST_Value *ast_mult(AST_Value *op1, AST_Value *op2){
    if (op1->type == Value_Number && op2->type == Value_Number) {
        int op1_len = strlen(op1->value);
        int op2_len = strlen(op2->value);
        return new_ast_value(Value_Number, format_str(op1_len + op2_len, "%g", strtofloat(op1->value, op1_len) * strtofloat(op2->value, op2_len)));
    } else ERR("cant add types `%s` and `%s`\n", find_ast_value_type(op1->type), find_ast_value_type(op2->type));
    return NULL;
}

AST_Value *ast_div(AST_Value *op1, AST_Value *op2){
    if (op1->type == Value_Number && op2->type == Value_Number) {
        int op1_len = strlen(op1->value);
        int op2_len = strlen(op2->value);
        return new_ast_value(Value_Number, format_str(op1_len + op2_len, "%g", strtofloat(op1->value, op1_len) / strtofloat(op2->value, op2_len)));
    } else ERR("cant add types `%s` and `%s`\n", find_ast_value_type(op1->type), find_ast_value_type(op2->type));
    return NULL;
}

AST_Value *eval_node(Node *n, Interpreter *interpreter) {
    if (n == NULL) {
        ERR("can't evaluate null node\n");
    } else if (n->type == AST_Literal) {
        return n->value;
    } else if (n->type == AST_Add) {
        AST_Value *op1 = eval_node(n->left, interpreter);
        AST_Value *op2 = eval_node(n->right, interpreter);
        AST_Value *result = ast_add(op1, op2);
        free(op1->value);
        free(op2->value);
        free(op1);
        free(op2);
        return result;
    } else if (n->type == AST_Sub) {
        AST_Value *op1 = eval_node(n->left, interpreter);
        AST_Value *op2 = eval_node(n->right, interpreter);
        AST_Value *result = ast_sub(op1, op2);
        free(op1->value);
        free(op2->value);
        free(op1);
        free(op2);
        return result;
    } else if (n->type == AST_Mult) {
        AST_Value *op1 = eval_node(n->left, interpreter);
        AST_Value *op2 = eval_node(n->right, interpreter);
        AST_Value *result = ast_mult(op1, op2);
        free(op1->value);
        free(op2->value);
        free(op1);
        free(op2);
        return result;
    } else if (n->type == AST_Div) {
        AST_Value *op1 = eval_node(n->left, interpreter);
        AST_Value *op2 = eval_node(n->right, interpreter);
        AST_Value *result = ast_div(op1, op2);
        free(op1->value);
        free(op2->value);
        free(op1);
        free(op2);
        return result;
    } else if (n->type == AST_Identifier) {
        char *var_name = strdup(n->value->value);
        Variable var = get_var(var_name, interpreter->vars, interpreter->vars_index);
        free(var_name);
        return new_ast_value(var.value->type, strdup(var.value->value));
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
            AST_Value *var_val = eval_node(n->left, interpreter);
            Variable var = { (n->left->value->type == Tok_Str) ? Var_Str : Var_Num, var_name, var_val, interpreter->vars_index };
            interpreter->vars[interpreter->vars_index] = var;
            interpreter->vars_index++;
            break;
        default: ERR("Unsupported statement type `%s`\n", find_ast_type(n->type));
    }
}

void interpret(Interpreter *interpreter) {
    for (int i = 0; i < interpreter->stmts_size; i++) {
        do_statement(interpreter->nodes[i], interpreter);
    }
}
