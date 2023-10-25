//#define DEBUG
#ifdef DEBUG
    #define debug(...) printf(__VA_ARGS__);
#else
    #define debug(...)
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

void free_mem(int exit_val);

#define ERR(...) do {fprintf (stderr, __VA_ARGS__); free_mem(1);} while (0);
#define ASSERT(expr, ...) do {if (!expr) {fprintf (stderr, __VA_ARGS__); free_mem(1);}} while (0);
#define num_len(num) snprintf(NULL, 0, "%d", num)
#define append(array, element, index, capacity) do {            \
    if (index >= capacity) {                                    \
        capacity *= 2;                                          \
        array = realloc(array, capacity * sizeof(array[0]));    \
    }                                                           \
                                                                \
    array[index++] = element;                                   \
} while (0);                                                    \

char *format_str(int strlen, const char *format, ...) {
    char *result = malloc(strlen + 1);
    va_list args;
    va_start(args, format);
    vsnprintf(result, strlen, format, args);
    va_end(args);
    return result;
}

int strtoint(const char *str, int len) {
    if (!str) ERR("need something to convert\n")
    if (!len) ERR("need length to convert\n")
    int total = 0;
    if (str[0] == '-') {
        for (int i = 1; i < len - 1; i++) {
            total -= str[i] - '0';
            total *= 10;
        }
    } else {
        for (int i = 0; i < len - 1; i++) {
            total += str[i] - '0';
            total *= 10;
        }
        total += str[len - 1] - '0';
    }
    return total;
}

char *read_file(const char *file_path) {
    FILE* file = fopen(file_path, "rb");
    if (!file) {
        ERR("ERROR: could not open file %s\n", file_path)
        exit(1);
    }
    fseek(file, 0L, SEEK_END);
    size_t file_capacity = ftell(file);
    rewind(file);

    char *program = malloc(file_capacity + 1);
    program[fread(program, 1, file_capacity, file)] = '\0';

    fclose(file);
    return program;
}

#include "variable.h"
#include "lexer.h"
#include "parser.h"
#include "function.h"
#include "interpreter.h"

//global variables for access to freeing from anywhere
Token *tokens;
char *program;
Parser *p;
Interpreter interpreter;

char **include_paths;
int include_paths_index;
int include_paths_capacity;

void free_mem(int exit_val) {

    debug("\n----------\nFREEING\n")

    free(tokens);
    free(p->jump_indices);
    for (int i = 0; i < interpreter.vars_index; i++) free_ast_value(interpreter.vars[i].value);
    free(interpreter.vars);
    for (int i = 0; i < interpreter.funcs_capacity; i++) free_function(interpreter.funcs[i]);
    free(interpreter.funcs);
    for (int i = 0; i < p->nodes_index; i++) free_node(p->nodes[i]);
    free(p->nodes);
    free(p);
    free(program);
    for (int i = 0; i < include_paths_index; i++) free(include_paths[i]);
    free(include_paths);
    exit(exit_val);
}

int main(int argc, char *argv[]) {
    //read input file
    if (argc != 2) {
        fprintf(stderr, "ERROR: invalid arguments\nUsage: suda [script]\n");
        exit(1);
    }
    
    //tokenize program
    include_paths = calloc(10, sizeof(char*));
    include_paths_index = 0;
    include_paths_capacity = 10;
    
    program = read_file(argv[1]);
    Lexer lexer = { program, program, 1 };
    int tokens_index = 0;
    int tokens_capacity = 10;

    tokens = calloc(tokens_capacity, sizeof(struct Token));

    debug("LEXING\n")
    while (1) {
        if (tokens_index >= tokens_capacity) {
            tokens_capacity *= 2;
            tokens = realloc(tokens, tokens_capacity * sizeof(struct Token));
        }
        Token tok = scan_token(&lexer);
        if (tok.type == Tok_Eof) {
            tokens[tokens_index] = tok;
            tokens_index++;
            break;
        } else if (tok.type == Tok_Comment) {
        } else if (tok.type == Tok_Include) {
            //TODO: including currently counts lines of included files, not just main file
            // offset line based on number of lines in included file
            tok = scan_token(&lexer);
            char *include_path = format_str(tok.length - 1, "%.*s", tok.length, tok.start + 1);
            int included = 0;
            for (int i = 0; i < include_paths_index; i++) {
                if (!strcmp(include_path, include_paths[i])) {
                    included = 1;
                }
            }
            if (included) {
                free(include_path);
                continue;
            }
            append(include_paths, include_path, include_paths_index, include_paths_capacity);
            char *include = read_file(include_path);
            
            //concatenate included file and current program
            int new_program_len = strlen(program) + strlen(include) + 3;
            char *new_program = calloc(new_program_len, sizeof(char));
            strcat(new_program, include);
            strcat(new_program, "\n\n");
            strcat(new_program, program);

            free(program);
            free(include);
            program = new_program;
            lexer.start = program;
            lexer.current = program;
            lexer.line = 1;
            
            //start lexing again from beginning of new program
            free(tokens);
            tokens = calloc(10, sizeof(struct Token));
            tokens_index = 0;
            tokens_capacity = 10;
        } else {
            tokens[tokens_index] = tok;
            tokens_index++;
        }
        debug("TOKEN ( `%s` | '%.*s' )\n", find_tok_type(tok.type), tok.length, tok.start)
    }

    debug("\n----------\nPARSING\n")

    //parse tokens
    p = calloc(1, sizeof(Parser));
    p->tokens = tokens;
    p->tok_index = 0;
    p->jumps_index = 0;
    p->jumps_capacity = 10;
    p->jump_indices = calloc(10, sizeof(int));

    p->nodes_index = 0;
    p->nodes_capacity = 10;

    p->nodes = calloc(p->nodes_capacity, sizeof(Node*));
    
    p->funcs_index = 0;
    p->funcs_capacity = 10;
    p->funcs = calloc(p->funcs_capacity, sizeof(Function*));

    //variables to store main state when parsing a function
    Function *func;
    int temp_nodes_capacity;
    int temp_nodes_index;
    Node **temp_nodes = NULL;

    while (1) {
        if (p->nodes_index >= p->nodes_capacity) {
            p->nodes_capacity *= 2;
            p->nodes = realloc(p->nodes, p->nodes_capacity * sizeof(Node*));
        }
        Node *n = statement(p);
        if (n->type == AST_End) {
            free_node(n);
            if (p->jumps_index > 0) {
                ERR("unclosed block\n")
            }
            break;
        } else if (n->type == AST_If) {
            append(p->jump_indices, p->nodes_index, p->jumps_index, p->jumps_capacity)
            append(p->nodes, n, p->nodes_index, p->nodes_capacity)
        } else if (n->type == AST_While) {
            append(p->jump_indices, p->nodes_index, p->jumps_index, p->jumps_capacity)
            append(p->nodes, n, p->nodes_index, p->nodes_capacity)
        } else if (n->type == AST_Else) {
            p->nodes[p->jump_indices[p->jumps_index - 1]]->jump_index = p->nodes_index;
            n->jump_index = p->jump_indices[p->jumps_index - 1];
            p->jumps_index--;
            append(p->jump_indices, p->nodes_index, p->jumps_index, p->jumps_capacity)
            append(p->nodes, n, p->nodes_index, p->nodes_capacity)
        } else if (n->type == AST_Elif) {
            p->nodes[p->jump_indices[p->jumps_index - 1]]->jump_index = p->nodes_index - 1;
            p->jump_indices[p->jumps_index - 1] = p->nodes_index;
            append(p->nodes, n, p->nodes_index, p->nodes_capacity)
        } else if (n->type == AST_Semicolon) {
            if (p->parsing_function && p->jumps_index <= 0) {
                free_node(n);
                func->nodes = p->nodes;
                func->nodes_size = p->nodes_index;
                append(p->funcs, func, p->funcs_index, p->funcs_capacity);

                p->nodes = temp_nodes;
                p->nodes_index = temp_nodes_index;
                p->nodes_capacity = temp_nodes_capacity;
                temp_nodes = NULL;
                continue;
            }
            p->jumps_index--;
            if (p->jumps_index < 0) ERR("ERROR on line %d: extra semicolon\n", n->line)
            if (p->nodes[p->jump_indices[p->jumps_index]]->type == AST_Break) {
                n->jump_index = p->nodes[p->jump_indices[p->jumps_index]]->jump_index;
                p->nodes[p->jump_indices[p->jumps_index]]->jump_index = p->nodes_index;
            } else {
                p->nodes[p->jump_indices[p->jumps_index]]->jump_index = p->nodes_index;
                n->jump_index = p->jump_indices[p->jumps_index];
            }
            append(p->nodes, n, p->nodes_index, p->nodes_capacity)
        } else if (n->type == AST_Break) {
            for (int i = p->jumps_index - 1; i > -1; i--) {
                if (p->nodes[p->jump_indices[i]]->type == AST_While) {
                    n->jump_index = p->jump_indices[i];
                    p->jump_indices[i] = p->nodes_index;
                    append(p->nodes, n, p->nodes_index, p->nodes_capacity)
                }
            }
            if (n->jump_index < 0) ERR("ERROR: tried to use break outside a while loop\n")
        } else if (n->type == AST_Continue) {
            for (int i = p->jumps_index - 1; i > -1; i--) {
                if (p->nodes[p->jump_indices[i]]->type == AST_While) {
                    n->jump_index = p->jump_indices[i];
                    append(p->nodes, n, p->nodes_index, p->nodes_capacity)
                }
            }
            if (n->jump_index < 0) ERR("ERROR: tried to use continue outside a while loop\n")
        } else if (n->type == AST_Function) {
            free_node(n);
            func = calloc(1, sizeof(Function));
            func->name = format_str(CURRENT_TOK.length + 1, "%.*s", CURRENT_TOK.length, CURRENT_TOK.start);
            if (check_func(p->funcs, p->funcs_index, func->name) > 0) ERR("ERROR on line %d: cant define function %s multiple times\n", CURRENT_TOK.line, func->name)

            p->tok_index++;
            ASSERT((p->jumps_index >= 0), "ERROR: unclosed block before function %s\n", func->name)
            ASSERT((CURRENT_TOK.type == Tok_Left_Paren), "ERROR on line %d: need left paren to open function arguments\n", CURRENT_TOK.line)
            p->tok_index++;

            //switch parser to parse the function
            if (temp_nodes != NULL) ERR("ERROR cant define functions inside other functions\n")
            temp_nodes = p->nodes;
            temp_nodes_index = p->nodes_index;
            temp_nodes_capacity = p->nodes_capacity;
            p->nodes = calloc(10, sizeof(Node*));
            p->nodes_index = 0;
            p->nodes_capacity = 10;
            p->parsing_function = 1;

            //parse arguments into func->args
            int args_capacity = 1;
            func->arity = 0;
            func->args = calloc(args_capacity, sizeof(AST_Value*));
            while (1) {
                Node *n = expr(p, NULL);
                if (n->type == AST_Identifier) {
                    append(func->args, n->value, func->arity, args_capacity)
                    n->value = NULL;
                    free_node(n);
                } else if (n->type == AST_Comma) {
                    free_node(n);
                    continue;
                } else if (n->type == AST_Right_Paren) {
                    free_node(n);
                    break;
                } else ERR("ERROR on line %d: cant parse token type %s as part of function arguments\n", CURRENT_TOK.line, find_tok_type(CURRENT_TOK.type))
            }
        } else if (n->type == AST_Function_Call) {
            Function *func = get_func(p->funcs, p->funcs_index, n->value->value, n->line);
            int call_args_len = strtoint(n->left->value->value, strlen(n->left->value->value)) - 1;
            ASSERT((func->arity == call_args_len), "ERROR on line %d: cant call function %s with %d arguments\n", n->line, func->name, call_args_len)
            append(p->nodes, n, p->nodes_index, p->nodes_capacity)
        } else {
            append(p->nodes, n, p->nodes_index, p->nodes_capacity)
        }
    }

    debug("\n----------\nINTERPRETING\n")

    interpreter.nodes = p->nodes;
    interpreter.stmts_capacity = p->nodes_index;

    interpreter.vars = calloc(10, sizeof(struct Variable));
    interpreter.vars_index = 0;
    interpreter.vars_capacity = 10;

    interpreter.local_vars = NULL;
    interpreter.local_vars_index = -1;
    interpreter.local_vars_capacity = -1;

    interpreter.funcs_capacity = p->funcs_index;
    interpreter.funcs = p->funcs;

    interpreter.auto_jump = 0;

    interpret(&interpreter);

    free_mem(0);
}
