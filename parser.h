#pragma once

/* 
 * SUDA EBNF
 *
 * <print statement> : "print" <paren expr> | <literal>
 *
 * <paren expr> : "(" <expr> ")"
 * <expr> : <literal>
 */

#define CURRENT_TOK p->tokens[p->index]
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
    int index;

    Node *nodes;
} Parser;

Node *new_node(AST_Type type, Token *value) {
    Node *node = calloc(1, sizeof(struct Node));
    node->type = type;
    if (value->type) node->value = value;
    return node;
}

Node *expr(Parser *p) {
    switch (CURRENT_TOK.type) {
        case Tok_String:
            p->index++; return new_node(AST_Literal, &CURRENT_TOK);
        case Tok_Number:
            p->index++; return new_node(AST_Literal, &CURRENT_TOK);
        case Tok_Left_Paren:
            p->index++;
            Node *n = expr(p);
            ASSERT((CURRENT_TOK.type == Tok_Right_Paren), "Require closing parenthese, got %s\n", find_tok_type(CURRENT_TOK.type));
            p->index++;
            return n;
        default: ERR("Unsupported token type %s\n", find_tok_type(CURRENT_TOK.type));
    }
    return (Node*) {0};
}

Node *statement(Parser *p) {
    switch (CURRENT_TOK.type) {
        case Tok_Print:;
            Node *n = new_node(AST_Print, &NO_TOK);
            p->index++;
            n->left = expr(p);
            return n;        
        case Tok_Eof:;
            return new_node(AST_End, &NO_TOK);
        default: ERR("Unsupported token type %s\n", find_tok_type(CURRENT_TOK.type));
    }
    return (Node*) {0};
}

void parse(Parser *p) {
    p->nodes = statement(p);
}