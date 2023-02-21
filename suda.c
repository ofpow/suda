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
    for (int i = 0; i < interpreter.vars_index; i++) free(interpreter.vars[i].value);
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
    size_t file_size = ftell(file);
    rewind(file);

    program = malloc(file_size + 1);
    program[fread(program, 1, file_size, file)] = '\0';

    fclose(file);
    
    //tokenize program
    Lexer lexer = { program, program, 1 };
    int tokens_index = 0;
    int tokens_size = 10;

    tokens = calloc(tokens_size, sizeof(struct Token));

    debug("LEXING\n")
    while (1) {
        if (tokens_index >= tokens_size) {
            tokens_size *= 2;
            tokens = realloc(tokens, tokens_size * sizeof(struct Token));
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

    nodes_index = 0;
    int nodes_size = 10;

    nodes = calloc(nodes_size, sizeof(Node*));

    while (1) {
        if (nodes_index >= nodes_size) {
            nodes_size *= 2;
            nodes = realloc(nodes, nodes_size * sizeof(Node*));
        }
        Node *n = statement(&p);
        if (n->type == Tok_Eof) {
            free_node(n);
            break;
        } else {
            nodes[nodes_index] = n;
            nodes_index++;
        }
    }

    debug("\n----------\nINTERPRETING\n");

    interpreter.nodes = nodes;
    interpreter.stmts_size = nodes_index;
    interpreter.vars_index = 0;
    interpreter.vars = calloc(10, sizeof(struct Variable));
    interpret(&interpreter);

    free_mem(0);
}
