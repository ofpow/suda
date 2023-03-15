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

char *format_str(int strlen, const char *format, ...) {
    char *result = malloc(strlen + 1);
    va_list args;
    va_start(args, format);
    vsnprintf(result, strlen, format, args);
    va_end(args);
    return result;
}

#define ERR(...) do {fprintf (stderr, __VA_ARGS__); free_mem(1);} while (0);
#define ASSERT(expr, ...) do {if (!expr) {fprintf (stderr, __VA_ARGS__); free_mem(1);}} while (0)
#define append(array, element, index, capacity) do {            \
    if (index >= capacity) {                                    \
        capacity *= 2;                                          \
        array = realloc(array, capacity * sizeof(array[0]));    \
    }                                                           \
                                                                \
    array[index++] = element;                                   \
} while (0);                                                    \

#include "variable.h"
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"

//global variables for access to freeing from anywhere
Token *tokens;
char *program;
Parser p;
Node **nodes;
int nodes_index;
Interpreter interpreter;

void free_mem(int exit_val) {

    debug("\n----------\nFREEING\n");

    free(tokens);
    for (int i = 0; i < nodes_index; i++) free_node(nodes[i]);
    free(nodes);
    free(p.jump_indices);
    for (int i = 0; i < interpreter.vars_index; i++) { free(interpreter.vars[i].value->value); free(interpreter.vars[i].value); }
    free(interpreter.vars);
    free(program);
    exit(exit_val);
}

int main(int argc, char *argv[]) {
    //read input file
    if (argc != 2) {
        ERR("ERROR: invalid arguments\nUsage: suda [script]\n");
        exit(1);
    }

    FILE* file = fopen(argv[1], "rb");
    if (!file) {
        ERR("ERROR: could not open file %s\n", argv[1]);
        exit(1);
    }
    fseek(file, 0L, SEEK_END);
    size_t file_capacity = ftell(file);
    rewind(file);

    program = malloc(file_capacity + 1);
    program[fread(program, 1, file_capacity, file)] = '\0';

    fclose(file);
    
    //tokenize program
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
        } else {
            tokens[tokens_index] = tok;
            tokens_index++;
        }
        debug("TOKEN ( `%s` | '%.*s' )\n", find_tok_type(tok.type), tok.length, tok.start);
    }

    debug("\n----------\nPARSING\n");

    //parse tokens
    p.tokens = tokens;
    p.tok_index = 0;
    p.jumps_index = 0;
    p.jumps_capacity = 10;
    p.jump_indices = calloc(10, sizeof(int));

    nodes_index = 0;
    int nodes_capacity = 10;

    nodes = calloc(nodes_capacity, sizeof(Node*));

    while (1) {
        if (nodes_index >= nodes_capacity) {
            nodes_capacity *= 2;
            nodes = realloc(nodes, nodes_capacity * sizeof(Node*));
        }
        Node *n = statement(&p);
        if (n->type == AST_End) {
            free_node(n);
            break;
        } else if (n->type == AST_If) {
            append(p.jump_indices, nodes_index, p.jumps_index, p.jumps_capacity);
            append(nodes, n, nodes_index, nodes_capacity);
        } else if (n->type == AST_Semicolon) {
            nodes[p.jump_indices[p.jumps_index - 1]]->jump_index = nodes_index;
            n->jump_index = p.jump_indices[p.jumps_index - 1];
            p.jumps_index--;
            append(nodes, n, nodes_index, nodes_capacity);
        } else {
            append(nodes, n, nodes_index, nodes_capacity);
        }
    }

    debug("\n----------\nINTERPRETING\n");

    interpreter.nodes = nodes;
    interpreter.stmts_capacity = nodes_index;
    interpreter.vars_index = 0;
    interpreter.vars = calloc(10, sizeof(struct Variable));
    interpret(&interpreter);

    free_mem(0);
}
