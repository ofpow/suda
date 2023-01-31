#pragma once

float strtofloat(const char *str, int len) {
    float total = 0;
    for (int i = 0; i < len - 1; i++) {
        total += str[i] - '0';
        total *= 10;
    }
    total += str[len - 1] - '0';
    return total;
}

#define IS_MATH_OP(expr) ((expr == AST_Add) || (expr == AST_Sub) || (expr == AST_Mult) || (expr == AST_Div))

float parse_math_op(Node *n) {
    float f1, f2;
    if (n->left->type != AST_Literal) f1 = parse_math_op(n->left); else f1 = strtofloat(n->left->value->start, n->left->value->length);
    if (n->right->type != AST_Literal) f2 = parse_math_op(n->right); else f2 = strtofloat(n->right->value->start, n->right->value->length);
    switch (n->type) {
        case AST_Add: return f1 + f2;
        case AST_Sub: return f1 - f2;
        case AST_Mult: return f1 * f2;
        case AST_Div: return f1 / f2;
    }
    return -1;
}

void interpret(Node **nodes, int stmts_size) {
    for (int i = 0; i < stmts_size; i++) {
        switch (nodes[i]->type) {
            case AST_Print:;
                ASSERT((nodes[i]->left->type == AST_Literal || IS_MATH_OP(nodes[i]->left->type)), "Can't print %s\n", find_ast_type(nodes[i]->left->type));
                if (nodes[i]->left->value->type == Tok_String) {
                    printf("%.*s\n", nodes[i]->left->value->length - 2, nodes[i]->left->value->start + 1);
                } else if (nodes[i]->left->value->type == Tok_Number) {
                    printf("%.*s\n", nodes[i]->left->value->length, nodes[i]->left->value->start);
                } else if (IS_MATH_OP(nodes[i]->left->type)) {
                    float print = parse_math_op(nodes[i]->left);
                    printf("%g\n", print);
                } else {
                    ERR("Unknown type `%s`\n", find_tok_type(nodes[i]->left->value->type));
                }
                break;
            case AST_End:
                return;
            default: ERR("Unsupported statement type `%s`\n", find_ast_type(nodes[i]->type));
        }
    }
}
