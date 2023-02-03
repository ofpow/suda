#pragma once

#define IS_AST_MATH_OP(expr) ((expr == AST_Add) || (expr == AST_Sub) || (expr == AST_Mult) || (expr == AST_Div))

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
        char *result = malloc(op1_len + op2_len + 1);
        snprintf(result, op1_len + op2_len + 1, "%.*s%.*s", op1_len, op1, op2_len, op2);
        return result;
    } else {
        int op1_len = strlen(op1);
        int op2_len = strlen(op2);
        char *result = malloc(op1_len + op2_len + 1);
        snprintf(result, op1_len + op2_len, "%g", strtofloat(op1, op1_len) + strtofloat(op2, op2_len));
        return result;
    }
}
char *ast_sub(char *op1, char *op2){
    int op1_len = strlen(op1);
    int op2_len = strlen(op2);
    char *result = malloc(op1_len + op2_len + 1);
    snprintf(result, op1_len + op2_len, "%g", strtofloat(op1, op1_len) - strtofloat(op2, op2_len));
    return result;
}
char *ast_mult(char *op1, char *op2){
    int op1_len = strlen(op1);
    int op2_len = strlen(op2);
    char *result = malloc(op1_len + op2_len + 1);
    snprintf(result, op1_len + op2_len, "%g", strtofloat(op1, op1_len) * strtofloat(op2, op2_len));
    return result;
}
char *ast_div(char *op1, char *op2){
    int op1_len = strlen(op1);
    int op2_len = strlen(op2);
    char *result = malloc(op1_len + op2_len + 1);
    snprintf(result, op1_len + op2_len, "%g", strtofloat(op1, op1_len) / strtofloat(op2, op2_len));
    return result;
}

char *eval_node(Node *n) {
    if (n == NULL) {
        ERR("can't evaluate null node\n");
    } else if (n->type == AST_Literal) {
        if (n->value->type == Tok_String) {
            char *val = malloc(n->value->length + 1);
            snprintf(val, n->value->length, "%.*s", n->value->length - 2, n->value->start + 1);
            return val;
        } else if (n->value->type == Tok_Number) {
            char *val = malloc(n->value->length + 1);
            snprintf(val, n->value->length + 1, "%.*s", n->value->length, n->value->start);
            return val;
        } else ERR("unknown literal type `%s`\n", find_tok_type(n->value->type));
    } else if (n->type == AST_Add) {
        char *op1 = eval_node(n->left);
        char *op2 = eval_node(n->right);
        char *result = ast_add(op1, op2, (is_num(op1[0]) && is_num(op2[0])));
        free(op1);
        free(op2);
        return result;
    } else if (n->type == AST_Sub) {
        char *op1 = eval_node(n->left);
        char *op2 = eval_node(n->right);
        char *result = ast_sub(op1, op2);
        free(op1);
        free(op2);
        return result;
    } else if (n->type == AST_Mult) {
        char *op1 = eval_node(n->left);
        char *op2 = eval_node(n->right);
        char *result = ast_mult(op1, op2);
        free(op1);
        free(op2);
        return result;
    } else if (n->type == AST_Div) {
        char *op1 = eval_node(n->left);
        char *op2 = eval_node(n->right);
        char *result = ast_div(op1, op2);
        free(op1);
        free(op2);
        return result;
    } else ERR("unknown node type `%s`\n", find_ast_type(n->type));
    return "unreachable";
}

void do_statement(Node *n) {
    switch (n->type) {
        case AST_Print:;
            ASSERT((n->left->type == AST_Literal || IS_AST_MATH_OP(n->left->type)), "Can't print `%s`\n", find_ast_type(n->left->type));
            char *print = eval_node(n->left);
            printf("%s\n", print);
            free(print);
            break;
        default: ERR("Unsupported statement type `%s`\n", find_ast_type(n->type));
    }
}

void interpret(Node **nodes, int stmts_size) {
    for (int i = 0; i < stmts_size; i++) {
        do_statement(nodes[i]);
    }
}

//char *parse_math_op(Node *n) {
//    if (n->type == AST_Literal) {
//        char *s = malloc(n->value->length + 1);
//        if (n->value->type == Tok_String) snprintf(s, n->value->length, "%.*s", n->value->length - 1, n->value->start + 1);
//        else snprintf(s, n->value->length, "%.*s", n->value->length, n->value->start);
//        return s;
//    }
//    char *op1, *op2;
//    int op1_len, op2_len;
//    op1 = parse_math_op(n->left);
//    op1_len = strlen(op1);
//    op2 = parse_math_op(n->right);
//    op2_len = strlen(op2);
//    //if (n->left->type != AST_Literal) {op1 = parse_math_op(n->left); op1_len = strlen(op1);} else {op1 = strdup(n->left->value->start); op1_len = n->left->value->length;}
//    //if (n->right->type != AST_Literal) {op2 = parse_math_op(n->right); op2_len = strlen(op2);} else {op2 = strdup(n->right->value->start); op2_len = n->right->value->length;}
//    char *result = malloc(op1_len + op2_len + 1);
//    switch (n->type) {
//        case AST_Add:
//            if (is_num(op1[0]) && is_num(op2[0])) snprintf(result, op1_len + op2_len, "%g", strtofloat(op1, op1_len) + strtofloat(op2, op2_len));
//            else snprintf(result, op1_len + op2_len, "%.*s%.*s", op1_len, op1, op2_len, op2);
//            break;
//        case AST_Sub:
//            if (is_num(op1[0]) && is_num(op2[0])) snprintf(result, op1_len + op2_len, "%g", strtofloat(op1, op1_len) - strtofloat(op2, op2_len));
//            break;
//        case AST_Mult:
//            if (is_num(op1[0]) && is_num(op2[0])) snprintf(result, op1_len + op2_len, "%g", strtofloat(op1, op1_len) * strtofloat(op2, op2_len));
//            break;
//        case AST_Div:
//            if (is_num(op1[0]) && is_num(op2[0])) snprintf(result, op1_len + op2_len, "%g", strtofloat(op1, op1_len) / strtofloat(op2, op2_len));
//            break;
//        default:
//            return "unreachable";
//    }
//    free(op1);
//    free(op2);
//    return result;
//}