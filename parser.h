#pragma once

/* 
 * SUDA EBNF
 *
 * <print statement> : "print" <paren expr> | <literal>
 *
 * <paren expr> : "(" <expr> ")"
 * <expr> : <literal> | <math_expr>
 * <math_expr> : "<literal>" "+" | "-" | "*" | "/" "<literal>"
 */

#define CURRENT_TOK p->tokens[p->tok_index]
#define NEXT_TOK p->tokens[p->tok_index + 1]
#define LAST_TOK p->tokens[p->tok_index - 1]
#define CURRENT_NODE p->nodes[p->node_index]
#define NO_TOK (Token) {0}

typedef enum {
    AST_End,
    AST_Literal,
    AST_Add,
    AST_Sub,
    AST_Mult,
    AST_Div,
    AST_Print,
} AST_Type;

char *find_ast_type(int type) {
    switch (type) {
        case AST_End: return "AST_End";
        case AST_Literal: return "AST_Literal";
        case AST_Add: return "AST_Add";
        case AST_Sub: return "AST_Sub";
        case AST_Mult: return "AST_Mult";
        case AST_Div: return "AST_Div";
        case AST_Print: return "AST_Print";
        default: return "unreachable";
    }
}

typedef struct Node {
    int type;
    Token *value;

    struct Node *right;
    struct Node *left;
} Node;

typedef struct Parser {
    Token *tokens;
    int tok_index;

    Node **nodes;
    int node_index;
} Parser;

Node *new_node(AST_Type type, Token *value) {

    debug("NODE ( `%s` )\n", find_ast_type(type));

    Node *node = calloc(1, sizeof(struct Node));
    node->type = type;
    if (value != NULL) node->value = value; else node->value = NULL;
    node->left = NULL;
    node->right = NULL;
    return node;
}

Node *expr(Parser *p) {
    Node *n;
    switch (CURRENT_TOK.type) {
        case Tok_String:
            p->tok_index++; return new_node(AST_Literal, &LAST_TOK);
        case Tok_Number:
            p->tok_index++; return new_node(AST_Literal, &LAST_TOK);
        case Tok_Left_Paren:
            p->tok_index++;
            n = expr(p);
            ASSERT((CURRENT_TOK.type == Tok_Right_Paren), "Require closing parenthese, got %s\n", find_tok_type(CURRENT_TOK.type));
            p->tok_index++;
            return n;
        case Tok_Add:
            n = new_node(AST_Add, &NO_TOK);
            p->tok_index++;
            n->left = expr(p);
            n->right = expr(p);
            return n;
        case Tok_Sub:
            n = new_node(AST_Sub, &NO_TOK);
            p->tok_index++;
            n->left = expr(p);
            n->right = expr(p);
            return n;
        case Tok_Mult:
            n = new_node(AST_Mult, &NO_TOK);
            p->tok_index++;
            n->left = expr(p);
            n->right = expr(p);
            return n;
        case Tok_Div:
            n = new_node(AST_Div, &NO_TOK);
            p->tok_index++;
            n->left = expr(p);
            n->right = expr(p);
            return n;
        default: ERR("Unsupported token type for expr %s\n", find_tok_type(CURRENT_TOK.type));
    }
    return (Node*) {0};
}

Node *statement(Parser *p) {
    switch (CURRENT_TOK.type) {
        case Tok_Print:;
            Node *n = new_node(AST_Print, &NO_TOK);
            p->tok_index++;
            n->left = expr(p);
            return n;        
            break;
        case Tok_Eof:;
            return new_node(AST_End, &NO_TOK);
            break;
        default: return expr(p); 
    }
    return (Node*) {0};
}
