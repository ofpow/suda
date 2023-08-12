#pragma once

/* 
 * SUDA EBNF
 *
 * <statement> : <print statement> | <variable assignment> | <if statement>
 * <print statement> : "print" <paren expr> | <terminal>
 * <variable assignment> : "let" var_name "=" <expr>
 * <if statement> : "if" <expr> <statements> ";"
 *
 * <paren expr> : "(" <expr> ")"
 * <expr> : <terminal> | <math_expr> | <array>
 * <array> : "[" <expr> "," <expr> "," ... "]"
 * <math_expr> : "<expr>" "+" | "-" | "*" | "/" | ">" | ">=" | "<" | "<=" | "=" "<expr>"
 * <terminal> : <literal>
 */

#define CURRENT_TOK p->tokens[p->tok_index]
#define LAST_TOK p->tokens[p->tok_index - 1]
#define IS_TOK_MATH_OP(expr) ((expr == Tok_Add) || (expr == Tok_Sub) || (expr == Tok_Mult) || (expr == Tok_Div) || (expr == Tok_Less) || (expr == Tok_Less_Equal) || (expr == Tok_Greater) || (expr == Tok_Greater_Equal))
#define IS_AST_MATH_OP(expr) ((expr == AST_Add) || (expr == AST_Sub) || (expr == AST_Mult) || (expr == AST_Div) || (expr == AST_Less) || (expr == AST_Less_Equal) || (expr == AST_Greater) || (expr == AST_Greater_Equal) || (expr == AST_Equal))


typedef enum {
    AST_End,
    AST_Literal,
    AST_Add,
    AST_Sub,
    AST_Mult,
    AST_Div,
    AST_Print,
    AST_Var_Assign,
    AST_Identifier,
    AST_Greater,
    AST_Greater_Equal,
    AST_Less,
    AST_Less_Equal,
    AST_Equal,
    AST_If,
    AST_Semicolon,
    AST_Else,
    AST_While,
    AST_Array,
    AST_At,
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
        case AST_Var_Assign: return "AST_Var_Assign";
        case AST_Identifier: return "AST_Identifier";
        case AST_Greater: return "AST_Greater";
        case AST_Greater_Equal: return "AST_Greater_Equal";
        case AST_Less: return "AST_Less";
        case AST_Less_Equal: return "AST_Less_Equal";
        case AST_Equal: return "AST_Equal";
        case AST_If: return "AST_If";
        case AST_Semicolon: return "AST_Semicolon";
        case AST_Else: return "AST_Else";
        case AST_While: return "AST_While";
        case AST_Array: return "AST_Array";
        case AST_At: return "AST_At";
        default: return "ast type not found";
    }
}

typedef struct Node {
    int type;
    AST_Value *value;

    struct Node *right;
    struct Node *left;

    int jump_index;
} Node;

typedef struct Parser {
    Token *tokens;
    int tok_index;

    Node **nodes;
    int node_index;

    int *jump_indices;
    int jumps_capacity;
    int jumps_index;
} Parser;

AST_Value *new_ast_value(int type, char *value, AST_Value *next) {
    debug("AST_VALUE ( `%s` `%s` )\n", find_ast_value_type(type), value);

    AST_Value *val = calloc(1, sizeof(struct AST_Value));
    val->type = type;
    val->value = value;
    if (next != NULL) {
        val->next = new_ast_value(next->type, strdup(next->value), next->next);
    } else {
        val->next = NULL;
    }
    return val;
}

void free_ast_value(AST_Value *value) {
    free(value->value);
    free(value);
}

Node *new_node(AST_Type type, AST_Value *value, int jump_index) {

    debug("NODE ( `%s` )\n", find_ast_type(type));

    Node *node = calloc(1, sizeof(struct Node));
    node->type = type;
    node->value = value;
    node->jump_index = jump_index;
    node->left = NULL;
    node->right = NULL;
    return node;
}

void free_array(AST_Value *array_pointer) {
    if (array_pointer->next) {free_array(array_pointer->next); array_pointer->next = NULL;}
    if (array_pointer->value) {free(array_pointer->value); array_pointer->value = NULL; }
    free(array_pointer);
    array_pointer = NULL;
}

void free_node(Node *n) {

    debug("FREE NODE `%s`\n", find_ast_type(n->type));

    if (n == NULL) return;
    if (n->value != NULL) {
        if (n->value->value != NULL) {
            free(n->value->value);
            n->value->value = NULL;
        }
        if (n->value->next) {free_array(n->value->next); n->value->next = NULL;}
        free(n->value);
        n->value = NULL;
    }
    if (n->left != NULL) {free_node(n->left); n->left = NULL;}
    if (n->right != NULL) {free_node(n->right); n->right = NULL;}
    if (n->left == NULL && n->right == NULL) free(n); else {fprintf(stderr, "not everything freed correctly\n"); exit(-1);}
}

AST_Value *parse_array(Parser *p) {
    AST_Value *value;
    switch (CURRENT_TOK.type) {
        case Tok_Number:
            p->tok_index++;
            value = new_ast_value(Value_Number, format_str(LAST_TOK.length + 1, "%.*s", LAST_TOK.length, LAST_TOK.start), NULL);
            value->next = parse_array(p);
            return value;
        case Tok_String:
            p->tok_index++;
            value = new_ast_value(Value_String, format_str(LAST_TOK.length + 1, "\"%.*s\"", LAST_TOK.length, LAST_TOK.start + 1), NULL);
            value->next = parse_array(p);
            return value;
        case Tok_Comma:
            p->tok_index++;
            value = parse_array(p);
            return value;
        case Tok_Right_Bracket:
            p->tok_index++;
            return NULL;
        case Tok_Eof:
            ERR("unclosed array\n");
            break;
        default: 
            ERR("cant parse %s as part of array\n", find_tok_type(CURRENT_TOK.type));
    }
    return NULL;
}

Node *expr(Parser *p, Node *child) {
    Node *n;
    switch (CURRENT_TOK.type) {
        case Tok_String:
            p->tok_index++; 
            if (IS_TOK_MATH_OP(CURRENT_TOK.type)) return expr(p, new_node(AST_Literal, new_ast_value(Value_String, format_str(LAST_TOK.length - 1, "%.*s", LAST_TOK.length, LAST_TOK.start + 1), NULL), -1));
            return new_node(AST_Literal, new_ast_value(Value_String, format_str(LAST_TOK.length - 1, "%.*s", LAST_TOK.length, LAST_TOK.start + 1), NULL), -1);
        case Tok_Number:
            p->tok_index++;
            if (IS_TOK_MATH_OP(CURRENT_TOK.type)) return expr(p, new_node(AST_Literal, new_ast_value(Value_Number, format_str(LAST_TOK.length + 1, "%.*s", LAST_TOK.length, LAST_TOK.start), NULL), -1));
            return new_node(AST_Literal, new_ast_value(Value_Number, format_str(LAST_TOK.length + 1, "%.*s", LAST_TOK.length, LAST_TOK.start), NULL), -1);
        case Tok_Identifier: 
            p->tok_index++; 
            if (CURRENT_TOK.type == Tok_Equal) {
                n = new_node(AST_Identifier, new_ast_value(Value_String, format_str(LAST_TOK.length + 1, "%.*s", LAST_TOK.length, LAST_TOK.start), NULL), -1);
                p->tok_index++;
                n->left = expr(p, NULL);
                return n;
            } else if (CURRENT_TOK.type == Tok_At) {
                n = new_node(AST_At, new_ast_value(Value_String, format_str(LAST_TOK.length + 1, "%.*s", LAST_TOK.length, LAST_TOK.start), NULL), -1);
                p->tok_index++;
                n->left = expr(p, NULL);
                return n;
            }
            return new_node(AST_Identifier, new_ast_value(Value_String, format_str(LAST_TOK.length + 1, "%.*s", LAST_TOK.length, LAST_TOK.start), NULL), -1);
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
            n = new_node(AST_Add, NULL, -1);
            p->tok_index++;
            if (child != NULL) n->left = child; else n->left = expr(p, child);
            n->right = expr(p, child);
            return n;
        case Tok_Sub:
            n = new_node(AST_Sub, NULL, -1);
            p->tok_index++;
            if (child != NULL) n->left = child; else n->left = expr(p, child);
            n->right = expr(p, child);
            return n;
        case Tok_Mult:
            n = new_node(AST_Mult, NULL, -1);
            p->tok_index++;
            if (child != NULL) n->left = child; else n->left = expr(p, child);
            n->right = expr(p, child);
            return n;
        case Tok_Div:
            n = new_node(AST_Div, NULL, -1);
            p->tok_index++;
            if (child != NULL) n->left = child; else n->left = expr(p, child);
            n->right = expr(p, child);
            return n;
        case Tok_Greater:
            n = new_node(AST_Greater, NULL, -1);
            p->tok_index++;
            if (child != NULL) n->left = child; else n->left = expr(p, child);
            n->right = expr(p, child);
            return n;
        case Tok_Greater_Equal:
            n = new_node(AST_Greater_Equal, NULL, -1);
            p->tok_index++;
            if (child != NULL) n->left = child; else n->left = expr(p, child);
            n->right = expr(p, child);
            return n;
        case Tok_Less:
            n = new_node(AST_Less, NULL, -1);
            p->tok_index++;
            if (child != NULL) n->left = child; else n->left = expr(p, child);
            n->right = expr(p, child);
            return n;
        case Tok_Less_Equal:
            n = new_node(AST_Less_Equal, NULL, -1);
            p->tok_index++;
            if (child != NULL) n->left = child; else n->left = expr(p, child);
            n->right = expr(p, child);
            return n;
        case Tok_Equal:
            n = new_node(AST_Equal, NULL, -1);
            p->tok_index++;
            if (child != NULL) n->left = child; else n->left = expr(p, child);
            n->right = expr(p, child);
            return n;
        case Tok_Eof:
            break;
        case Tok_Left_Bracket:
            n = new_node(AST_Array, NULL, -1);
            p->tok_index++;
            n->value = new_ast_value(Value_Array, NULL, NULL);
            n->value->next = parse_array(p);
            return n;
        default: ERR("Unsupported token type for expr %s\n", find_tok_type(CURRENT_TOK.type));
    }
    return NULL;
}

Node *statement(Parser *p) {
    Node *n;
    switch (CURRENT_TOK.type) {
        case Tok_Print:;
            n = new_node(AST_Print, NULL, -1);
            p->tok_index++;
            n->left = expr(p, NULL);
            return n;        
            break;
        case Tok_Eof:
            return new_node(AST_End, NULL, -1);
            break;
        case Tok_Let:
            n = new_node(AST_Var_Assign, NULL, -1);
            p->tok_index++;
            n->value = new_ast_value(Value_String, format_str(CURRENT_TOK.length + 1, "%.*s", CURRENT_TOK.length, CURRENT_TOK.start), NULL);
            p->tok_index++;
            ASSERT((CURRENT_TOK.type = Tok_Equal), "Require `=` to assign to variable\n");
            p->tok_index++;
            n->left = expr(p, NULL);
            return n;
        case Tok_If:
            n = new_node(AST_If, NULL, -1);
            p->tok_index++;
            n->left = expr(p, NULL);
            return n;
        case Tok_Semicolon:
            n = new_node(AST_Semicolon, NULL, -1);
            p->tok_index++;
            return n;
        case Tok_Else:
            n = new_node(AST_Else, NULL, -1);
            p->tok_index++;
            return n;
        case Tok_While:
            n = new_node(AST_While, NULL, -1);
            p->tok_index++;
            n->left = expr(p, NULL);
            return n;
        default: return expr(p, NULL); 
    }
    return NULL;
}
