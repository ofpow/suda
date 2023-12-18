#pragma once

/* 
 * SUDA EBNF
 *
 * <statement> : <print statement> | <variable assignment> | <if statement> | <while statement> | <function definition> | <function call> | <append> | <include> | <for loop>
 * <print statement> : "print" <expr> | "println" <expr>
 * <variable assignment> : "let" var_name "=" <expr>
 * <if statement> : "if" <expr> <statements> ";" | "if" <expr> <statements> "else" <statements>
 * <while statement> : "while" <expr> <statements> ";"
 * <function definition> : "fn" function_name "(" <identifier> "," <identifier> "," ...")" <statements> ";"
 * <function call> : <function> "(" <expr> "," <expr> "," ... ")"
 * <append> : "append" <variable> <expr>
 * <include> : "include" <file path to include>
 * <for loop> : "for" <identifier> "in" <identifier> <statements> ";"
 *
 * <paren expr> : "(" <expr> ")"
 * <expr> : <terminal> | <math_expr> | <array> | <array access> | <function call> | <len> | <cast_num> | <cast_str>
 * <array> : "[" <expr> "," <expr> "," ... "]"
 * <math_expr> : "<expr>" "+" | "-" | "*" | "/" | ">" | ">=" | "<" | "<=" | "==" | "%" | "|" | "&" | "^" | ">>" | "<<" | "**" "<expr>"
 * <terminal> : <literal>
 * <array access> : <array variable> "@" <expr>
 * <function call> : <function> "(" <expr> "," <expr> "," ... ")"
 * <len> : "len" <string or array variable>
 * <cast_num> : num <expr>
 * <cast_str> : str <expr>
 * 
 */

#define CURRENT_TOK p->tokens[p->tok_index]
#define LAST_TOK p->tokens[p->tok_index - 1]
#define NEXT_TOK p->tokens[p->tok_index + 1]
#define IS_TOK_MATH_OP(expr) ((expr == Tok_Add) || (expr == Tok_Sub) || (expr == Tok_Mult) || (expr == Tok_Div) || (expr == Tok_Less) || (expr == Tok_Less_Equal) || (expr == Tok_Greater) || (expr == Tok_Greater_Equal) || (expr == Tok_Is_Equal) || (expr == Tok_And) || (expr == Tok_Or) || (expr == Tok_Not) || (expr == Tok_Not_Equal) || (expr == Tok_Modulo) || (expr == Tok_Bit_And) || (expr == Tok_Bit_Or) || (expr == Tok_Bit_Xor) || (expr == Tok_Bit_Not) || (expr == Tok_Lshift) || (expr == Tok_Rshift) || (expr == Tok_Power))
#define IS_AST_MATH_OP(expr) ((expr == AST_Add) || (expr == AST_Sub) || (expr == AST_Mult) || (expr == AST_Div) || (expr == AST_Less) || (expr == AST_Less_Equal) || (expr == AST_Greater) || (expr == AST_Greater_Equal) || (expr == AST_Is_Equal) || (expr == AST_And) || (expr == AST_Or) || (expr == AST_Not) || (expr == AST_Not_Equal) || (expr == AST_Modulo) || (expr == AST_Bit_And) || (expr == AST_Bit_Or) || (expr == AST_Bit_Xor) || (expr == AST_Bit_Not) || (expr == AST_Lshift) || (expr == AST_Rshift) || (expr == AST_Power))

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
    AST_Assign,
    AST_If,
    AST_Semicolon,
    AST_Else,
    AST_While,
    AST_Array,
    AST_At,
    AST_Is_Equal,
    AST_Function,
    AST_Comma,
    AST_Right_Paren,
    AST_Return,
    AST_Function_Call,
    AST_Len,
    AST_Break,
    AST_Exit,
    AST_Continue,
    AST_And,
    AST_Or,
    AST_Not,
    AST_Not_Equal,
    AST_Modulo,
    AST_Elif,
    AST_Bit_And,
    AST_Bit_Or,
    AST_Bit_Xor,
    AST_Bit_Not,
    AST_Lshift,
    AST_Rshift,
    AST_Power,
    AST_Append,
    AST_Cast_Str,
    AST_Cast_Num,
    AST_Println,
    AST_For,
    AST_In,
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
        case AST_Assign: return "AST_Assign";
        case AST_If: return "AST_If";
        case AST_Semicolon: return "AST_Semicolon";
        case AST_Else: return "AST_Else";
        case AST_While: return "AST_While";
        case AST_Array: return "AST_Array";
        case AST_At: return "AST_At";
        case AST_Is_Equal: return "AST_Is_Equal";
        case AST_Function: return "AST_Function";
        case AST_Comma: return "AST_Comma";
        case AST_Right_Paren: return "AST_Right_Paren";
        case AST_Return: return "AST_Return";
        case AST_Function_Call: return "AST_Function_Call";
        case AST_Len: return "AST_Len";
        case AST_Break: return "AST_Break";
        case AST_Exit: return "AST_Exit";
        case AST_Continue: return "AST_Continue";
        case AST_And: return "AST_And";
        case AST_Or: return "AST_Or";
        case AST_Not: return "AST_Not";
        case AST_Not_Equal: return "AST_Not_Equal";
        case AST_Modulo: return "AST_Modulo";
        case AST_Elif: return "AST_Elif";
        case AST_Bit_And: return "AST_Bit_And";
        case AST_Bit_Or: return "AST_Bit_Or";
        case AST_Bit_Xor: return "AST_Bit_Xor";
        case AST_Bit_Not: return "AST_Bit_Not";
        case AST_Lshift: return "AST_Lshift";
        case AST_Rshift: return "AST_Rshift";
        case AST_Power: return "AST_Power";
        case AST_Append: return "AST_Append";
        case AST_Cast_Str: return "AST_Cast_Str";
        case AST_Cast_Num: return "AST_Cast_Num";
        case AST_Println: return "AST_Println";
        case AST_For: return "AST_For";
        case AST_In: return "AST_In";
        default: return "ast type not found";
    }
}

AST_Type tok_to_ast(Token_Type type, int64_t line, const char *file) {
    switch (type) {
        case Tok_Add: return AST_Add;
        case Tok_Sub: return AST_Sub;
        case Tok_Mult: return AST_Mult;
        case Tok_Div: return AST_Div;
        case Tok_Modulo: return AST_Modulo;
        case Tok_Greater: return AST_Greater;
        case Tok_Greater_Equal: return AST_Greater_Equal;
        case Tok_Less: return AST_Less;
        case Tok_Less_Equal: return AST_Less_Equal;
        case Tok_Is_Equal: return AST_Is_Equal;
        case Tok_Not_Equal: return AST_Not_Equal;
        case Tok_Bit_And: return AST_Bit_And;
        case Tok_Bit_Or: return AST_Bit_Or;
        case Tok_Bit_Xor: return AST_Bit_Xor;
        case Tok_Lshift: return AST_Lshift;
        case Tok_Rshift: return AST_Rshift;
        case Tok_Power: return AST_Power;
        case Tok_And: return AST_And;
        case Tok_Or: return AST_Or;
        case Tok_Bit_Not: return AST_Bit_Not;
        case Tok_Not: return AST_Not;
        case Tok_Len: return AST_Len;
        case Tok_Cast_Num: return AST_Cast_Num;
        case Tok_Cast_Str: return AST_Cast_Str;
        case Tok_Print: return AST_Print;
        case Tok_If: return AST_If;
        case Tok_While: return AST_While;
        case Tok_Return: return AST_Return;
        case Tok_Exit: return AST_Exit;
        case Tok_Break: return AST_Break;
        case Tok_Continue: return AST_Continue;
        case Tok_Semicolon: return AST_Semicolon;
        case Tok_Else: return AST_Else;
        case Tok_Function: return AST_Function;
        case Tok_Elif: return AST_Elif;
        case Tok_Println: return AST_Println;
        case Tok_For: return AST_For;
        case Tok_In: return AST_In;
        default: ERR("ERROR in %s on line %ld: cant convert token type %s to ast\n", file, line, find_tok_type(type))
    }
    return -1;
}

typedef struct Node {
    int64_t type;
    AST_Value *value;

    struct Node *right;
    struct Node *left;

    int64_t jump_index;

    int64_t line;
    const char *file;

    struct Node **func_args;
    int64_t func_args_index;
    int64_t func_args_capacity;
} Node;

typedef struct Function {
    char *name;
    Node **nodes;
    int64_t nodes_size;

    int64_t arity;
    AST_Value **args;

    int64_t line;
} Function;

typedef struct Parser {
    Token *tokens;
    int64_t tok_index;

    Node **nodes;
    int64_t nodes_index;
    int64_t nodes_capacity;

    int64_t *jump_indices;
    int64_t jumps_capacity;
    int64_t jumps_index;

    Function **funcs;
    int64_t funcs_index;
    int64_t funcs_capacity;
    int64_t parsing_function;
} Parser;

AST_Value *new_ast_value(int type, void *value, int64_t mutable) {
    debug("AST_VALUE ( `%s` `%s` )\n", find_ast_value_type(type), value)

    AST_Value *val = calloc(1, sizeof(struct AST_Value));
    val->type = type;
    val->value = value;
    val->mutable = mutable;
    return val;
}

void free_ast_value(AST_Value *value) {
    if (value == NULL) return;
    if (value->type == Value_Array) {
        int64_t arr_len = NUM(value[0].value);
        for (int j = 0; j < arr_len; j++) {
            if (value[j].value != NULL) free(value[j].value);
            value[j].value = NULL;
        }
    }
    free(value->value);
    free(value);
}

Node *new_node(AST_Type type, AST_Value *value, int64_t jump_index, int64_t line, const char *file) {

    debug("NODE ( `%s` )\n", find_ast_type(type))

    Node *node = calloc(1, sizeof(struct Node));
    node->type = type;
    node->value = value;
    node->jump_index = jump_index;
    node->line = line;
    node->left = NULL;
    node->right = NULL;
    node->file = file;
    node->func_args = NULL;
    return node;
}

void free_node(Node *n) {

    debug("FREE NODE `%s`\n", find_ast_type(n->type))

    if (n == NULL) return;

    if (n->type == AST_Array) {
        int64_t size = NUM(n->value->value);
        for (int i = 0; i < size; i++) {
            if (n->value[i].value != 0) free(n->value[i].value);
            n->value[i].value = NULL;
        }
    }
    
    if (n->value != NULL) {
        if (n->value->type == Value_Function_Args) {
            int64_t len = NUM(n->value[0].value);
            for (int i = 0; i < len; i++) {
                if (n->value[i].value) free(n->value[i].value);
            }
        } else if (n->value->value != NULL) {
            free(n->value->value);
            n->value->value = NULL;
        }
        free(n->value);
        n->value = NULL;
    }
    if (n->left != NULL) {
        free_node(n->left);
        n->left = NULL;
    }
    if (n->right != NULL) {
        free_node(n->right);
        n->right = NULL;
    }

    if (n->func_args != NULL) {
        for (int i = 0; i < n->func_args_index; i++)
            free_node(n->func_args[i]);
    }
    free(n->func_args);

    if (n->left == NULL && n->right == NULL) 
        free(n);
    else
        ERR("ERROR in %s on line %ld: not everything freed correctly\n", n->file, n->line)
}

void free_function(Function *func) {
    for (int i = 0; i < func->nodes_size; i++) if (func->nodes) free_node(func->nodes[i]);
    free(func->nodes);
    for (int i = 0; i < func->arity; i++) if (func->args[i]) free_ast_value(func->args[i]);
    free(func->args);
    free(func->name);
    free(func);
}

AST_Value *parse_list(Parser *p) {
    int64_t list_capacity = 2;
    int64_t list_index = 1;
    AST_Value *list = calloc(list_capacity, sizeof(struct AST_Value));
    while (1) {
        switch (CURRENT_TOK.type) {
            case Tok_Number:
                p->tok_index++;
                append(list, ((AST_Value){ Value_Number, dup_int(strtoint(LAST_TOK.start, LAST_TOK.length)), 1 }), list_index, list_capacity)
                //append(list, ((AST_Value){ Value_Number, format_str(LAST_TOK.length + 1, "%.*s", LAST_TOK.length, LAST_TOK.start), 1 }), list_index, list_capacity)
                break;
            case Tok_String:
                p->tok_index++;
                append(list, ((AST_Value){ Value_String, format_str(LAST_TOK.length + 1, "\"%.*s\"", LAST_TOK.length, LAST_TOK.start + 1), 1 }), list_index, list_capacity)
                break;
            case Tok_Identifier:
                p->tok_index++;
                append(list, ((AST_Value){ Value_Identifier, format_str(LAST_TOK.length + 1, "%.*s", LAST_TOK.length, LAST_TOK.start), 1 }), list_index, list_capacity)
                break;
            case Tok_Comma:
                p->tok_index++;
                continue;
            case Tok_Right_Bracket:
                p->tok_index++;
                list[0].type = Value_Array;
                list[0].value = dup_int(list_index);
                return list;
            case Tok_Right_Paren:
                p->tok_index++;
                list[0].type = Value_Function_Args;
                list[0].value = dup_int(list_index);
                //list[0].value = format_str(num_len(list_index) + 1, "%ld", list_index);
                return list;
            case Tok_Eof:
                ERR("ERROR in %s on line %ld: unclosed list\n", CURRENT_TOK.file, CURRENT_TOK.line)
                break;
            default: 
                ERR("ERROR in %s on line %ld: cant parse %s as part of list\n", CURRENT_TOK.file, CURRENT_TOK.line, find_tok_type(CURRENT_TOK.type))
        }
    }
    return NULL;
}

Node *expr(Parser *p, Node *child) {
    Node *n;
    switch (CURRENT_TOK.type) {
        case Tok_String:
            p->tok_index++; 
            if (IS_TOK_MATH_OP(CURRENT_TOK.type))
                return expr(p, new_node(AST_Literal, new_ast_value(Value_String, format_str(LAST_TOK.length - 1, "%.*s", LAST_TOK.length, LAST_TOK.start + 1), 1), -1, CURRENT_TOK.line, CURRENT_TOK.file));
            return new_node(AST_Literal, new_ast_value(Value_String, format_str(LAST_TOK.length - 1, "%.*s", LAST_TOK.length, LAST_TOK.start + 1), 1), -1, CURRENT_TOK.line, CURRENT_TOK.file);
        case Tok_Number:
            p->tok_index++;
            int64_t *val = calloc(1, sizeof(int64_t));
            *val = strtoint(LAST_TOK.start, LAST_TOK.length);
            if (IS_TOK_MATH_OP(CURRENT_TOK.type))
                return expr(p, new_node(AST_Literal, new_ast_value(Value_Number, val, 1), -1, CURRENT_TOK.line, CURRENT_TOK.file));
            return new_node(AST_Literal, new_ast_value(Value_Number, val, 1), -1, CURRENT_TOK.line, CURRENT_TOK.file);
        case Tok_Identifier: 
            p->tok_index++; 
            if (CURRENT_TOK.type == Tok_Assign) {
                n = new_node(AST_Identifier, new_ast_value(Value_Identifier, format_str(LAST_TOK.length + 1, "%.*s", LAST_TOK.length, LAST_TOK.start), 1), -1, CURRENT_TOK.line, CURRENT_TOK.file);
                p->tok_index++;
                n->left = expr(p, NULL);
                return n;
            } else if (CURRENT_TOK.type == Tok_At) {
                n = new_node(AST_At, new_ast_value(Value_String, format_str(LAST_TOK.length + 1, "%.*s", LAST_TOK.length, LAST_TOK.start), 1), -1, CURRENT_TOK.line, CURRENT_TOK.file);
                p->tok_index++;
                n->left = expr(p, NULL);
                if (CURRENT_TOK.type == Tok_Assign) {
                    p->tok_index++;
                    n->right = expr(p, NULL);
                }
                return n;
            } else if (CURRENT_TOK.type == Tok_Left_Paren) {
                n = new_node(AST_Function_Call, NULL, -1, CURRENT_TOK.line, CURRENT_TOK.file);
                n->value = new_ast_value(Value_String, format_str(LAST_TOK.length + 1, "%.*s", LAST_TOK.length, LAST_TOK.start), 1);
                n->func_args = calloc(10, sizeof(Node*));
                n->func_args_index = 0;
                n->func_args_capacity = 10;
                p->tok_index++;
                Node *arg;
                while ((arg = expr(p, NULL)) != NULL) {
                    if (arg->type == AST_Right_Paren) { free_node(arg); break; }
                    else if (arg->type == AST_Comma) free_node(arg);
                    else append(n->func_args, arg, n->func_args_index, n->func_args_capacity)
                }
                return n;
            } else if (IS_TOK_MATH_OP(CURRENT_TOK.type)) {
                return expr(p, new_node(AST_Identifier, new_ast_value(Value_Identifier, format_str(LAST_TOK.length + 1, "%.*s", LAST_TOK.length, LAST_TOK.start), 1), -1, CURRENT_TOK.line, CURRENT_TOK.file));
            }
            return new_node(AST_Identifier, new_ast_value(Value_Identifier, format_str(LAST_TOK.length + 1, "%.*s", LAST_TOK.length, LAST_TOK.start), 1), -1, CURRENT_TOK.line, CURRENT_TOK.file);
        case Tok_Left_Paren:
            p->tok_index++;
            n = expr(p, NULL);
            while (CURRENT_TOK.type != Tok_Right_Paren) {
                if (CURRENT_TOK.type == Tok_Eof) ERR("ERROR in %s on line %ld: Require closing parenthese\n", CURRENT_TOK.file, CURRENT_TOK.line)
                n = expr(p, n);
            }
            p->tok_index++;
            return n;
        case Tok_Add: // nodes with 2 arguments
        case Tok_Sub:
        case Tok_Mult:
        case Tok_Div:
        case Tok_Modulo:
        case Tok_Greater:
        case Tok_Greater_Equal:
        case Tok_Less:
        case Tok_Less_Equal:
        case Tok_Is_Equal:
        case Tok_Not_Equal:
        case Tok_Bit_And:
        case Tok_Bit_Or:
        case Tok_Bit_Xor:
        case Tok_Lshift:
        case Tok_Rshift:
        case Tok_Power:
        case Tok_And:
        case Tok_Or:
            n = new_node(tok_to_ast(CURRENT_TOK.type, CURRENT_TOK.line, CURRENT_TOK.file), NULL, -1, CURRENT_TOK.line, CURRENT_TOK.file);
            p->tok_index++;
            if (child != NULL) n->left = child; else n->left = expr(p, child);
            n->right = expr(p, child);
            return n;

        case Tok_Bit_Not: // nodes with 1 argument
        case Tok_Not:
        case Tok_Len:
        case Tok_Cast_Num:
        case Tok_Cast_Str:
            n = new_node(tok_to_ast(CURRENT_TOK.type, CURRENT_TOK.line, CURRENT_TOK.file), NULL, -1, CURRENT_TOK.line, CURRENT_TOK.file);
            p->tok_index++;
            n->left = expr(p, NULL);
            return n;

        case Tok_Comma:
            p->tok_index++;
            return new_node(AST_Comma, NULL, -1, CURRENT_TOK.line, CURRENT_TOK.file);
        case Tok_Right_Paren:
            p->tok_index++;
            return new_node(AST_Right_Paren, NULL, -1, CURRENT_TOK.line, CURRENT_TOK.file);
        case Tok_Eof:
            break;
        case Tok_Left_Bracket:
            n = new_node(AST_Array, NULL, -1, CURRENT_TOK.line, CURRENT_TOK.file);
            p->tok_index++;
            n->value = parse_list(p);
            return n;
        default: ERR("ERROR in %s on line %ld: Unsupported token type for expr %s\n", CURRENT_TOK.file, CURRENT_TOK.line, find_tok_type(CURRENT_TOK.type))
    }
    return NULL;
}

Node *statement(Parser *p) {
    Node *n;
    switch (CURRENT_TOK.type) {
        case Tok_Eof:
            return new_node(AST_End, NULL, -1, CURRENT_TOK.line, CURRENT_TOK.file);
        case Tok_Let:
            n = new_node(AST_Var_Assign, NULL, -1, CURRENT_TOK.line, CURRENT_TOK.file);
            p->tok_index++;
            n->value = new_ast_value(Value_String, format_str(CURRENT_TOK.length + 1, "%.*s", CURRENT_TOK.length, CURRENT_TOK.start), 1);
            p->tok_index++;
            ASSERT((CURRENT_TOK.type = Tok_Assign), "Require `=` to assign to variable\n")
            p->tok_index++;
            n->left = expr(p, NULL);
            return n;
        case Tok_Print: //nodes with 1 arg
        case Tok_Println:
        case Tok_If:
        case Tok_While:
        case Tok_Return:
        case Tok_Exit:
        case Tok_Elif:
            n = new_node(tok_to_ast(CURRENT_TOK.type, CURRENT_TOK.line, CURRENT_TOK.file), NULL, -1, CURRENT_TOK.line, CURRENT_TOK.file);
            p->tok_index++;
            n->left = expr(p, NULL);
            return n;

        case Tok_Break: //nodes with no args
        case Tok_Continue:
        case Tok_Semicolon:
        case Tok_Else:
        case Tok_Function:
            n = new_node(tok_to_ast(CURRENT_TOK.type, CURRENT_TOK.line, CURRENT_TOK.file), NULL, -1, CURRENT_TOK.line, CURRENT_TOK.file);
            p->tok_index++;
            return n;

        case Tok_Append:
            n = new_node(AST_Append, NULL, -1, CURRENT_TOK.line, CURRENT_TOK.file);
            p->tok_index++;
            n->value = new_ast_value(Value_String, format_str(CURRENT_TOK.length + 1, "%.*s", CURRENT_TOK.length, CURRENT_TOK.start), 1);
            p->tok_index++;
            n->left = expr(p, NULL);
            return n;
        case Tok_For:
            n = new_node(AST_For, NULL, -1, CURRENT_TOK.line, CURRENT_TOK.file);
            p->tok_index++;

            ASSERT((CURRENT_TOK.type == Tok_Identifier), "ERROR in %s on line %ld: need identifier to follow for\n", CURRENT_TOK.file, CURRENT_TOK.line)
            n->left = expr(p, NULL);

            ASSERT((CURRENT_TOK.type == Tok_In), "ERROR in %s on line %ld: need `in` to follow identifier\n", CURRENT_TOK.file, CURRENT_TOK.line)
            p->tok_index++;
            n->right = expr(p, NULL);
            n->value = new_ast_value(Value_Number, dup_int(0), 1);
            return n;

        default: return expr(p, NULL); 
    }
    return NULL;
}
