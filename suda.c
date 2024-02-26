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
#include <time.h>
#include <limits.h>

void free_mem(int exit_val);

#define ERR(...) do {fprintf (stderr, __VA_ARGS__); free_mem(1);} while (0);
#define ASSERT(expr, ...) do {if (!(expr)) {fprintf (stderr, __VA_ARGS__); free_mem(1);}} while (0);
#define NUM(_val) (*((int64_t*)(_val)))
#define STR(_val) ((char*)(_val))
#define append(_array, _element, _index, _capacity) do {         \
    if (_index >= _capacity) {                                   \
        _capacity *= 2;                                          \
        _array = realloc(_array, _capacity * sizeof(_array[0])); \
    }                                                            \
                                                                 \
    _array[_index++] = _element;                                 \
} while (0);                                                     \

int64_t num_len(int64_t n) {
    if (n < 0) n = (n == LONG_MIN) ? LONG_MAX : -n;
    if (n < 10) return 1;
    if (n < 100) return 2;
    if (n < 1000) return 3;
    if (n < 10000) return 4;
    if (n < 100000) return 5;
    if (n < 1000000) return 6;
    if (n < 10000000) return 7;
    if (n < 100000000) return 8;
    if (n < 1000000000) return 9;
    if (n < 10000000000) return 10;
    if (n < 100000000000) return 11;
    if (n < 1000000000000) return 12;
    if (n < 10000000000000) return 13;
    if (n < 100000000000000) return 14;
    if (n < 1000000000000000) return 15;
    if (n < 10000000000000000) return 16;
    if (n < 100000000000000000) return 17;
    if (n < 1000000000000000000) return 18;
    return 19;
}

char *format_str(int strlen, const char *format, ...) {
    char *result = malloc(strlen + 1);
    va_list args;
    va_start(args, format);
    vsnprintf(result, strlen, format, args);
    va_end(args);
    return result;
}

int64_t *dup_int(int64_t x) {
    int64_t *rtrn = calloc(1, sizeof(int64_t));
    *rtrn = x;
    return rtrn;
}

int64_t strtoint(const char *str, int64_t len) {
    if (!str) ERR("need something to convert\n")
    if (!len) ERR("need length to convert\n")
    int64_t total = 0;
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

typedef struct Programs {
    char **progs;
    int64_t progs_index;
    int64_t progs_capacity;
} Programs;

char **call_stack = NULL;
int call_stack_index;
int call_stack_capacity;

#include "hash.h"
#include "variable.h"
#include "lexer.h"
#include "parser.h"
#include "function.h"
#include "interpreter.h"
#include "vm.h"

//global variables for access to freeing from anywhere
Token *tokens;
Programs *programs;
Parser *p;
Interpreter interpreter;

char **include_paths;
int include_paths_index;
int include_paths_capacity;

void free_mem(int exit_val) {

    debug("\n----------\nFREEING\n")

    free(tokens);
    free(p->jump_indices);
    free_map(interpreter.vars);
    for (int i = 0; i < interpreter.funcs_capacity; i++) free_function(interpreter.funcs[i]);
    free(interpreter.funcs);
    for (int i = 0; i < p->nodes_index; i++) free_node(p->nodes[i]);
    free(p->nodes);
    free(p);
    for (int i = 0; i < programs->progs_index; i++) free(programs->progs[i]);
    free(programs->progs);
    free(programs);
    for (int i = 0; i < include_paths_index; i++) free(include_paths[i]);
    if (call_stack != NULL) {
        if (exit_val > 0 && call_stack_index > 0) {
            printf("Stack trace:\n");
            for (int i = call_stack_index - 1; i >= 0; i--) {
                printf("%s\n", call_stack[i]);
                free(call_stack[i]);
            }
        } else {
            for (int i = call_stack_index - 1; i >= 0; i--) {
                free(call_stack[i]);
            }
        }
        free(call_stack);
    }
    free(include_paths);
    exit(exit_val);
}

int main(int argc, char *argv[]) {
    //read input file
    if (argc < 2) {
        fprintf(stderr, "ERROR: invalid arguments\nUsage: suda <args> [script]\n");
        exit(1);
    }

    bool time = false;
    bool bytecode = false;
    struct timespec tstart, tend, tfinal;
    char *file_path;

    // suda's argc and argv
    AST_Value *suda_argc = NULL;
    AST_Value *suda_argv = NULL;

    //TODO: make debug a cli flag
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-t")) {
            time = true;
        } else if (!strcmp(argv[i], "-b")) {
            bytecode = true;
        } else if (!strcmp(argv[i], "-c")) {
            call_stack = calloc(10, sizeof(char*));
            call_stack_index = 0;
            call_stack_capacity = 10;
        } else if (!strcmp(argv[i], "-h")) {
            printf("Usage: suda <args> [file] \n");
            printf("Arguments:\n");
            printf("  -h: print this message\n");
            printf("  -t: print timing info\n");
            printf("  -c: print call stack on crash\n");
            return 0;
        } else {
            file_path = argv[i];
            if ((argc - 1) > i) {
                suda_argc = new_ast_value(Value_Number, dup_int((argc - 1) - i), 1, hash("argc", 4));
                i++;
                int index = 1;
                int capacity = 10;
                suda_argv = calloc(2, sizeof(struct AST_Value));
                while (i < argc) {
                    int len = strlen(argv[i]);
                    append(suda_argv, ((AST_Value){ Value_String, format_str(len + 3, "\"%s\"", strdup(argv[i])), 1, 0 }), index, capacity)
                    i++;
                }
                suda_argv->hash = hash("argv", 4);
                suda_argv->mutable = true;
                suda_argv[0].type = Value_Array;
                suda_argv[0].value = dup_int(index);
            }
            break;
        }
    }

    if (time) {
        tstart=(struct timespec){0,0};
        tend=(struct timespec){0,0};
        tfinal=(struct timespec){0,0};
        clock_gettime(CLOCK_MONOTONIC, &tstart);
        tfinal = tstart;
    }
    
    //tokenize program
    debug("\n----------\nLEXING\n")
    programs = calloc(1, sizeof(Programs));
    programs->progs = calloc(2, sizeof(char*));
    programs->progs_index = 0;
    programs->progs_capacity = 2;
    tokens = lex_file(file_path, programs);

    if (time) {
        clock_gettime(CLOCK_MONOTONIC, &tend);
        printf("LEXING       time: %f seconds\n",
               ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - 
               ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec));
        tstart = tend;
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
    int64_t temp_nodes_capacity;
    int64_t temp_nodes_index;
    Node **temp_nodes = NULL;

    while (1) {
        Node *n = statement(p);
        if (n->type == AST_End) {
            free_node(n);
            if (p->jumps_index > 0) {
                ERR("unclosed block\n")
            }
            break;
        } else if (n->type == AST_If) {
            debug("IF: push index %ld\n", p->nodes_index)
            append(p->jump_indices, p->nodes_index, p->jumps_index, p->jumps_capacity)
            append(p->nodes, n, p->nodes_index, p->nodes_capacity)
        } else if (n->type == AST_While) {
            debug("WHILE: push index %ld\n", p->nodes_index)
            append(p->jump_indices, p->nodes_index, p->jumps_index, p->jumps_capacity)
            append(p->nodes, n, p->nodes_index, p->nodes_capacity)
        } else if (n->type == AST_For) {
            debug("For: push index %ld\n", p->nodes_index)
            append(p->jump_indices, p->nodes_index, p->jumps_index, p->jumps_capacity)
            append(p->nodes, n, p->nodes_index, p->nodes_capacity)
        } else if (n->type == AST_Else) {
            debug("ELSE: change %ld to %ld\n", p->jump_indices[p->jumps_index - 1], p->nodes_index)
            p->nodes[p->jump_indices[p->jumps_index - 1]]->jump_index = p->nodes_index;
            n->jump_index = p->jump_indices[p->jumps_index - 1];
            p->jumps_index--;
            append(p->jump_indices, p->nodes_index, p->jumps_index, p->jumps_capacity)
            append(p->nodes, n, p->nodes_index, p->nodes_capacity)
        } else if (n->type == AST_Elif) {
            debug("ELIF: change %ld to %ld\n", p->jump_indices[p->jumps_index - 1], p->nodes_index)
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
            if (p->jumps_index < 0) ERR("ERROR in %s on line %ld: extra semicolon\n", n->file, n->line)
            debug("SEMICOLON: pop index %ld\n", p->jump_indices[p->jumps_index])
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
                if (p->nodes[p->jump_indices[i]]->type == AST_While || p->nodes[p->jump_indices[i]]->type == AST_Break) {
                    n->jump_index = p->jump_indices[i];
                    p->jump_indices[i] = p->nodes_index;
                    append(p->nodes, n, p->nodes_index, p->nodes_capacity)
                }
            }
            if (p->nodes[p->nodes_index - 1]->jump_index < 0) ERR("ERROR in %s on line %ld: tried to use break outside a while loop\n", n->file, n->line)
        } else if (n->type == AST_Continue) {
            for (int i = p->jumps_index - 1; i > -1; i--) {
                if (p->nodes[p->jump_indices[i]]->type == AST_While) {
                    n->jump_index = p->jump_indices[i];
                    append(p->nodes, n, p->nodes_index, p->nodes_capacity)
                }
            }
            if (n->jump_index < 0) ERR("ERROR in %s on line %ld: tried to use continue outside a while loop\n", n->file, n->line)
        } else if (n->type == AST_Function) {
            free_node(n);
            func = calloc(1, sizeof(Function));
            func->name = format_str(CURRENT_TOK.length + 1, "%.*s", CURRENT_TOK.length, CURRENT_TOK.start);
            func->line = CURRENT_TOK.line;
            if (check_func(p->funcs, p->funcs_index, func->name) > 0) ERR("ERROR in %s on line %ld: cant define function %s multiple times\n", CURRENT_TOK.file, CURRENT_TOK.line, func->name)

            p->tok_index++;
            ASSERT((p->jumps_index >= 0), "ERROR in %s on line %ld: unclosed block before function %s\n",  CURRENT_TOK.file,CURRENT_TOK.line, func->name)
            ASSERT((CURRENT_TOK.type == Tok_Left_Paren), "ERROR in %s on line %ld: need left paren to open function arguments\n",  CURRENT_TOK.file,CURRENT_TOK.line)
            p->tok_index++;

            //switch parser to parse the function
            if (temp_nodes != NULL) ERR("ERROR in %s on line %ld: cant define functions inside other functions\n", CURRENT_TOK.file, LAST_TOK.line)
            temp_nodes = p->nodes;
            temp_nodes_index = p->nodes_index;
            temp_nodes_capacity = p->nodes_capacity;
            p->nodes = calloc(10, sizeof(Node*));
            p->nodes_index = 0;
            p->nodes_capacity = 10;
            p->parsing_function = 1;

            //parse arguments into func->args
            int64_t args_capacity = 1;
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
                } else ERR("ERROR in %s on line %ld: cant parse token type %s as part of function arguments\n", CURRENT_TOK.file, CURRENT_TOK.line, find_tok_type(CURRENT_TOK.type))
            }
        } else {
            append(p->nodes, n, p->nodes_index, p->nodes_capacity)
        }
    }

    if (time) {
        clock_gettime(CLOCK_MONOTONIC, &tend);
        printf("PARSING      time: %f seconds\n",
               ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - 
               ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec));
        tstart = tend;
    }

    debug("\n----------\nINTERPRETING\n")
    if (bytecode) {
        VM vm = {0};
        vm.code = calloc(10, sizeof(u_int8_t));
        vm.code_capacity = 10;

        vm.constants = calloc(10, sizeof(Value));
        vm.constants_capacity = 10;

        vm.stack_top = vm.stack;

        compile(p->nodes, p->nodes_index, &vm);

        run(&vm);
    } else {
        interpreter.nodes = p->nodes;
        interpreter.stmts_capacity = p->nodes_index;

        interpreter.vars = new_map(8);
        interpreter.local_vars = NULL;
        if (suda_argc == NULL) {
            suda_argc = new_ast_value(Value_Number, dup_int(0), 1, hash("argc", 4));
            suda_argv = calloc(1, sizeof(AST_Value));
            suda_argv->hash = hash("argv", 4);
            suda_argv->mutable = true;
            suda_argv[0].value = dup_int(1);
            suda_argv[0].type = Value_Array;
        }
        assign_variable(&interpreter, "argc", suda_argc->hash, suda_argc, -1, file_path);
        assign_variable(&interpreter, "argv", suda_argv->hash, suda_argv, -1, file_path);

        interpreter.funcs_capacity = p->funcs_index;
        interpreter.funcs = p->funcs;

        interpreter.auto_jump = 0;

        interpret(&interpreter);

        if (time) {
            clock_gettime(CLOCK_MONOTONIC, &tend);
            printf("INTERPRETING time: %f seconds\n",
                ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - 
                ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec));
            printf("OVERALL      time: %f seconds\n",
                ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - 
                ((double)tfinal.tv_sec + 1.0e-9*tfinal.tv_nsec));
        }

        free_mem(0);
    }
}
