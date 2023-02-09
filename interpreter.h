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

char *ast_add(char *op1, char *op2, int is_string){
    if (!is_string) {
        int op1_len = strlen(op1);
        int op2_len = strlen(op2);
        return format_str(op1_len + op2_len + 1, "%.*s%.*s", op1_len, op1, op2_len, op2);
    } else {
        int op1_len = strlen(op1);
        int op2_len = strlen(op2);
        return format_str(op1_len + op2_len, "%g", strtofloat(op1, op1_len) + strtofloat(op2, op2_len));
    }
}

char *ast_sub(char *op1, char *op2){
    int op1_len = strlen(op1);
    int op2_len = strlen(op2);
    return format_str(op1_len + op2_len, "%g", strtofloat(op1, op1_len) - strtofloat(op2, op2_len));
}

char *ast_mult(char *op1, char *op2){
    int op1_len = strlen(op1);
    int op2_len = strlen(op2);
    return format_str(op1_len + op2_len, "%g", strtofloat(op1, op1_len) * strtofloat(op2, op2_len));
}

char *ast_div(char *op1, char *op2){
    int op1_len = strlen(op1);
    int op2_len = strlen(op2);
    return format_str(op1_len + op2_len, "%g", strtofloat(op1, op1_len) / strtofloat(op2, op2_len));
}

AST_Value *eval_node(Node *n, Interpreter *interpreter) {
    if (n == NULL) {
        ERR("can't evaluate null node\n");
    } else if (n->type == AST_Literal) {
        return n->value;
    //} else if (n->type == AST_Add) {
    //    char *op1 = eval_node(n->left, interpreter);
    //    char *op2 = eval_node(n->right, interpreter);
    //    char *result = ast_add(op1, op2, (is_num(op1[0]) && is_num(op2[0])));
    //    free(op1);
    //    free(op2);
    //    return result;
    //} else if (n->type == AST_Sub) {
    //    char *op1 = eval_node(n->left, interpreter);
    //    char *op2 = eval_node(n->right, interpreter);
    //    char *result = ast_sub(op1, op2);
    //    free(op1);
    //    free(op2);
    //    return result;
    //} else if (n->type == AST_Mult) {
    //    char *op1 = eval_node(n->left, interpreter);
    //    char *op2 = eval_node(n->right, interpreter);
    //    char *result = ast_mult(op1, op2);
    //    free(op1);
    //    free(op2);
    //    return result;
    //} else if (n->type == AST_Div) {
    //    char *op1 = eval_node(n->left, interpreter);
    //    char *op2 = eval_node(n->right, interpreter);
    //    char *result = ast_div(op1, op2);
    //    free(op1);
    //    free(op2);
    //    return result;
    //} else if (n->type == AST_Identifier) {
    //    char *var_name = format_str(n->value->length + 1, "%.*s", n->value->length, n->value->start);
    //    Variable var = get_var(var_name, interpreter->vars, interpreter->vars_index);
    //    free(var_name);
    //    return strdup(var.value);
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
        //case AST_Var_Assign:;

        //    //NODE has value with var name, left has top of value
        //    char *var_name = format_str(n->value->length + 1, "%.*s", n->value->length, n->value->start);
        //    char *var_val = eval_node(n->left, interpreter);
        //    Variable var = { (n->left->value->type == Tok_Str) ? Var_Str : Var_Num, var_name, var_val, interpreter->vars_index };
        //    interpreter->vars[interpreter->vars_index] = var;
        //    interpreter->vars_index++;
        //    break;
        default: ERR("Unsupported statement type `%s`\n", find_ast_type(n->type));
    }
}

void interpret(Interpreter *interpreter) {
    for (int i = 0; i < interpreter->stmts_size; i++) {
        do_statement(interpreter->nodes[i], interpreter);
    }
}
