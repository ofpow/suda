#pragma once

/* 
 * SUDA EBNF
 *
 * <print statement> : "print" <paren expr> | <terminal>
 *
 * <paren expr> : "(" <expr> ")"
 * <expr> : <terminal> | <math_expr>
 * <math_expr> : "<terminal>" "+" | "-" | "*" | "/" "<terminal>"
 * <terminal> : <literal>
 */

#define CURRENT_TOK p->tokens[p->tok_index]
#define NEXT_TOK p->tokens[p->tok_index + 1]
#define LAST_TOK p->tokens[p->tok_index - 1]
#define CURRENT_NODE p->nodes[p->node_index]
#define LAST_NODE p->nodes[p->node_index - 1]
#define NO_TOK (Token) {0}
#define NO_NODE (Node) {0}
#define IS_TOK_MATH_OP(expr) ((expr == Tok_Add) || (expr == Tok_Sub) || (expr == Tok_Mult) || (expr == Tok_Div))
#define IS_TERMINAL(expr) ((expr == AST_Literal))

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

Node *dup_node(Node *n) {
    debug("DUP NODE ( `%s` )\n", find_ast_type(n->type));

    Node *new_n = new_node(n->type, NULL);
    if (n->value != NULL) new_n->value = n->value;
    if (n->left != NULL) new_n->left = dup_node(n->left);
    if (n->right != NULL) new_n->right = dup_node(n->right);
    return new_n;    
}

void free_node(Node *n) {

    debug("FREE NODE `%s`\n", find_ast_type(n->type));

    if (n == NULL) return;
    if (n->value != NULL) n->value = NULL;
    if (n->left != NULL) {free_node(n->left); n->left = NULL;}
    if (n->right != NULL) {free_node(n->right); n->right = NULL;}
    if (n->left == NULL && n->right == NULL && n->value == NULL) free(n); else {fprintf(stderr, "not everything freed correctly\n"); exit(-1);}
}

Node *expr(Parser *p, Node *child) {
    Node *n;
    switch (CURRENT_TOK.type) {
        case Tok_String:
            p->tok_index++; 
            if (IS_TOK_MATH_OP(CURRENT_TOK.type)) return expr(p, new_node(AST_Literal, &LAST_TOK));
            return new_node(AST_Literal, &LAST_TOK);
        case Tok_Number:
            p->tok_index++;
            if (IS_TOK_MATH_OP(CURRENT_TOK.type)) return expr(p, new_node(AST_Literal, &LAST_TOK));
            return new_node(AST_Literal, &LAST_TOK);
        case Tok_Left_Paren:
            p->tok_index++;
            n = expr(p, NULL);
            while (CURRENT_TOK.type != Tok_Right_Paren) {
                if (CURRENT_TOK.type == Tok_Eof) ERR("Require closing parenthese\n");
                n = expr(p, n);
            }
            p->tok_index++;
            return n;
        case Tok_Add:
            n = new_node(AST_Add, NULL);
            p->tok_index++;
            if (child != NULL) n->left = child; else n->left = expr(p, child);
            n->right = expr(p, child);
            return n;
        case Tok_Sub:
            n = new_node(AST_Sub, NULL);
            p->tok_index++;
            if (child != NULL) n->left = child; else n->left = expr(p, child);
            n->right = expr(p, child);
            return n;
        case Tok_Mult:
            n = new_node(AST_Mult, NULL);
            p->tok_index++;
            if (child != NULL) n->left = child; else n->left = expr(p, child);
            n->right = expr(p, child);
            return n;
        case Tok_Div:
            n = new_node(AST_Div, NULL);
            p->tok_index++;
            if (child != NULL) n->left = child; else n->left = expr(p, child);
            n->right = expr(p, child);
            return n;
        //case Tok_Sub:
        //    n = new_node(AST_Sub, NULL);
        //    p->tok_index++;
        //    n->left = expr(p, child);
        //    n->right = expr(p, child);
        //    return n;
        //case Tok_Mult:
        //    n = new_node(AST_Mult, NULL);
        //    p->tok_index++;
        //    n->left = expr(p, child);
        //    n->right = expr(p, child);
        //    return n;
        //case Tok_Div:
        //    n = new_node(AST_Div, NULL);
        //    p->tok_index++;
        //    n->left = expr(p, child);
        //    n->right = expr(p, child);
        //    return n;
        //case Tok_Right_Paren:
        //    break;
        default: ERR("Unsupported token type for expr %s\n", find_tok_type(CURRENT_TOK.type));
    }
    return (Node*) {0};
}

Node *statement(Parser *p) {
    switch (CURRENT_TOK.type) {
        case Tok_Print:;
            Node *n = new_node(AST_Print, NULL);
            p->tok_index++;
            n->left = expr(p, NULL);
            return n;        
            break;
        case Tok_Eof:
            return new_node(AST_End, NULL);
            break;
        default: return expr(p, NULL); 
    }
    return (Node*) {0};
}
