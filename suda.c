#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define ERR(...) fprintf (stderr, __VA_ARGS__);
#define ASSERT(expr, ...) if (!expr) {fprintf (stderr, __VA_ARGS__); exit(1);}

int strtoi(const char *str, int len) {
    int total = 0;
    for (int i = 0; i < len - 1; i++) {
        total += str[i] - '0';
        total *= 10;
    }
    total += str[len - 1] - '0';
    return total;
}

#include "lexer.h"

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

    char *program = malloc(file_size + 1);
    program[fread(program, 1, file_size, file)] = '\0';

    fclose(file);
    
    //tokenize program
    Lexer lexer = { program, program, 1 };
    int tokens_index = 0;
    int tokens_size = 100;

    Token *tokens = calloc(tokens_size, sizeof(struct Token));

    int line = -1;

    int jumps_size = 10;
    int jumps_index = 0;
    int *jump_indices = calloc(jumps_size, sizeof(int));

    while (1) {
        if (tokens_index >= tokens_size) {
            tokens_size *= 2;
            tokens = realloc(tokens, tokens_size);
        }
        Token tok = scan_token(&lexer);
        if (tok.type == Eof) {
            break;
        } else if (tok.type == Comment) {
        } else if (tok.type == If || tok.type == While) {
            if (jumps_index >= jumps_size) {
                jumps_size *= 2;
                jump_indices = realloc(jump_indices, jumps_size);
            }
            jump_indices[jumps_index] = tokens_index;
            jumps_index++;
            tokens[tokens_index] = tok;
            tokens_index++;
        } else if (tok.type == Else) {
            if (tokens[jump_indices[jumps_index - 1]].type != If) {
                ERR("ERROR: else can only be used in if blocks\n");
                goto clean_up;
            } else {
                tok.jump_index = jump_indices[jumps_index - 1];
                jumps_index--;
                tokens[tokens_index] = tok;
                tokens_index++;
            }
        } else if (tok.type == Semicolon) {
            if (jumps_index - 1 < 0) {
                ERR("ERROR: semicolon without block opener on line %d\n", tok.line);
                goto clean_up;
            }
            tok.jump_index = jump_indices[jumps_index - 1];
            jumps_index--;
            tokens[tokens_index] = tok;
            tokens_index++;
        } else {
            tokens[tokens_index] = tok;
            tokens_index++;
        }
    }
clean_up:
    free(jump_indices);

    free(tokens);
    free(program);
    return 0;
}
