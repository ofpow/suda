#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <limits.h>

//#define DEBUG
#ifdef DEBUG
    #define debug(...) printf(__VA_ARGS__);
#else
    #define debug(...)
#endif

//#define PROFILE
#ifdef PROFILE
u_int64_t *instr_profiler = 0;
double *time_profiler = 0;
#endif

void free_mem(int exit_val);

#define ERR(...) do {fprintf (stderr, __VA_ARGS__); free_mem(1);} while (0);
#define ASSERT(expr, ...) do {if (!(expr)) {fprintf (stderr, __VA_ARGS__); free_mem(1);}} while (0);
#define NUM(_val) (*((int64_t*)(_val)))
#define STR(_val) ((char*)(_val))
#define append_verbose(_array, _element, _index, _capacity) do {         \
    if (_index >= _capacity) {                                   \
        _capacity *= 2;                                          \
        _array = realloc(_array, _capacity * sizeof(_array[0])); \
    }                                                            \
                                                                 \
    _array[_index++] = _element;                                 \
} while (0);                                                     \

#define append(_array, _element) do {                                               \
    if (_array.index >= _array.capacity) {                                              \
        _array.capacity *= 2;                                                           \
        _array.data = realloc(_array.data, _array.capacity * sizeof(_array.data[0]));   \
    }                                                                                   \
                                                                                        \
    _array.data[_array.index++] = _element;                                             \
} while (0)                                                                             \

#define free_array(_array, _free) do {        \
    for (int i = 0; i < _array.index; i++) {  \
        _free(_array.data[i]);                \
    }                                         \
    free(_array.data);                        \
} while (0)                                   \

#define define_array(_name, _type) \
    typedef struct _name {         \
        _type *data;               \
        int64_t index;             \
        int64_t capacity;          \
    } _name                        \

#define report_time(_msg) do {                                      \
    if (time) {                                                     \
        clock_gettime(CLOCK_MONOTONIC, &tend);                      \
        printf(_msg,                                                \
               ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) -        \
               ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec));    \
        tstart = tend;                                              \
    }                                                               \
} while (0)                                                         \

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

define_array(String_Array, char*);

char **call_stack = NULL;
int call_stack_index;
int call_stack_capacity;

#include "str.h"
#include "hash.h"
#include "lexer.h"
#include "parser.h"
#include "native.h"
#include "compiler.h"
#include "vm.h"
#include "assembler.h"

//global variables for access to freeing from anywhere
Token *tokens;
String_Array programs = {0};
String_Array include_paths = {0};
VM vm = {0};
Compiler c = {0};
bool disassemble_bytecode = false;
bool instructions = false;
bool assemble = false;

void free_mem(int exit_val) {

    debug("\n----------\nFREEING\n")

    if (vm.call_stack_count > 1 && exit_val) {
        printf("Stack trace:\n");
        for (int i = vm.call_stack_count - 1; i > 0; i--)
            printf("%s:%ld\n", vm.call_stack[i].func->name, vm.call_stack[i].loc.line);
    }
    free(c.if_indices.data);
    free(c.while_indices.data);
    free_array(vm.funcs, free_func);
    free_array(p->funcs, free_ast_function);
    free_map(vm.vars);
    free(tokens);
    free(p->jumps.data);
    for (int i = 0; i < p->nodes.index; i++) free_node(p->nodes.data[i]);
    free(p->nodes.data);
    free(p);
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
    free_array(programs, free);
    free_array(include_paths, free);
    exit(exit_val);
}

int main(int argc, char *argv[]) {

    srand(time(NULL));

    //read input file
    if (argc < 2) {
        fprintf(stderr, "ERROR: invalid arguments\nUsage: suda <args> [script]\n");
        exit(1);
    }

    bool time = false;
    struct timespec tstart, tend, tfinal;
    char *file_path;

    // suda's argc and argv
    AST_Value *suda_argc = NULL;
    AST_Value *suda_argv = NULL;

    //TODO: make debug a cli flag
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-t")) {
            time = true;
        } else if (!strcmp(argv[i], "-d")) {
            disassemble_bytecode = true;
        } else if (!strcmp(argv[i], "-i")) {
            instructions = true;
        } else if (!strcmp(argv[i], "-a")) {
            assemble = true;
        } else if (!strcmp(argv[i], "-h")) {
            printf("Usage: suda <args> [file] \n");
            printf("Arguments:\n");
            printf("  -h: print this message\n");
            printf("  -t: print timing info\n");
            printf("  -d: disassemble bytecode instead of running\n");
            printf("  -i: print instructions when profiling\n");
            printf("  -a: assemble bytecode to machine code\n");
            return 0;
        } else {
            file_path = argv[i];
            if ((argc - 1) > i) {
                suda_argc = new_ast_value(Value_Number, dup_int((argc - 1) - i), 1, hash("argc", 4));
                i++;
                int index = 1;
                int capacity = 2;
                suda_argv = calloc(2, sizeof(struct AST_Value));
                while (i < argc) {
                    int len = strlen(argv[i]);
                    append_verbose(suda_argv, ((AST_Value){ Value_String, format_str(len + 3, "\"%s\"", strdup(argv[i])), 1, 0 }), index, capacity)
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

    programs.data = calloc(2, sizeof(char*));
    programs.index = 0;
    programs.capacity = 2;

    include_paths.data = calloc(2, sizeof(char*));
    include_paths.index = 0;
    include_paths.capacity = 2;

    tokens = lex_file(file_path, &programs, &include_paths);

    report_time("LEXING       time: %f seconds\n");


    debug("\n----------\nPARSING\n")

    //parse tokens
    p = calloc(1, sizeof(Parser));
    p->tokens = tokens;
    p->tok_index = 0;

    p->jumps = (Jump_Indices){
        calloc(10, sizeof(int64_t)),
        0,
        10,
    };

    p->nodes = (Node_Array){
        calloc(10, sizeof(Node*)),
        0,
        10,
    };
    
    p->funcs = (AST_Function_Array){
        calloc(10, sizeof(AST_Function*)),
        0,
        10,
    };

    //variables to store main state when parsing a function
    AST_Function *func;
    int64_t temp_nodes_capacity;
    int64_t temp_nodes_index;
    Node **temp_nodes = NULL;

    while (1) {
        Node *n = statement(p);
        if (n->type == AST_End) {
            free_node(n);
            if (p->jumps.index > 0) {
                ERR("unclosed block\n")
            }
            break;
        } else if (n->type == AST_If) {
            debug("IF: push index %ld\n", p->nodes.index)
            append(p->jumps, p->nodes.index);
            append(p->nodes, n);
        } else if (n->type == AST_While) {
            debug("WHILE: push index %ld\n", p->nodes.index)
            append(p->jumps, p->nodes.index);
            append(p->nodes, n);
        } else if (n->type == AST_For) {
            debug("FOR: push index %ld\n", p->nodes.index)
            append(p->jumps, p->nodes.index);
            append(p->nodes, n);
        } else if (n->type == AST_Enumerate) {
            debug("ENUMERATE: push index %ld\n", p->nodes.index)
            append(p->jumps, p->nodes.index);
            append(p->nodes, n);
        } else if (n->type == AST_Else) {
            debug("ELSE: change %ld to %ld\n", p->jumps.data[p->jumps.index - 1], p->nodes.index)
            p->nodes.data[p->jumps.data[p->jumps.index - 1]]->jump_index = p->nodes.index;
            n->jump_index = p->jumps.data[p->jumps.index - 1];
            p->jumps.index--;
            append(p->jumps, p->nodes.index);
            append(p->nodes, n);
        } else if (n->type == AST_Elif) {
            debug("ELIF: change %ld to %ld\n", p->jumps.data[p->jumps.index - 1], p->nodes.index)
            p->nodes.data[p->jumps.data[p->jumps.index - 1]]->jump_index = p->nodes.index - 1;
            p->jumps.data[p->jumps.index - 1] = p->nodes.index;
            append(p->nodes, n);
        } else if (n->type == AST_Semicolon) {
            if (p->parsing_function && p->jumps.index <= 0) {
                free_node(n);
                func->nodes = p->nodes;
                append(p->funcs, func);

                p->nodes.data = temp_nodes;
                p->nodes.index = temp_nodes_index;
                p->nodes.capacity = temp_nodes_capacity;
                temp_nodes = NULL;
                continue;
            }
            p->jumps.index--;
            if (p->jumps.index < 0) ERR("ERROR in %s on line %ld: extra semicolon\n", n->file, n->line)
            debug("SEMICOLON: pop index %ld\n", p->jumps.data[p->jumps.index])
            if (p->nodes.data[p->jumps.data[p->jumps.index]]->type == AST_Break) {
                n->jump_index = p->nodes.data[p->jumps.data[p->jumps.index]]->jump_index;
                p->nodes.data[p->jumps.data[p->jumps.index]]->jump_index = p->nodes.index;
            } else {
                p->nodes.data[p->jumps.data[p->jumps.index]]->jump_index = p->nodes.index;
                n->jump_index = p->jumps.data[p->jumps.index];
            }
            append(p->nodes, n);
        } else if (n->type == AST_Break) {
            for (int i = p->jumps.index - 1; i > -1; i--) {
                if (p->nodes.data[p->jumps.data[i]]->type == AST_While || p->nodes.data[p->jumps.data[i]]->type == AST_Break) {
                    n->jump_index = p->jumps.data[i];
                    p->jumps.data[i] = p->nodes.index;
                    append(p->nodes, n);
                }
            }
            if (p->nodes.data[p->nodes.index - 1]->jump_index < 0) ERR("ERROR in %s on line %ld: tried to use break outside a while loop\n", n->file, n->line)
        } else if (n->type == AST_Continue) {
            for (int i = p->jumps.index - 1; i > -1; i--) {
                if (p->nodes.data[p->jumps.data[i]]->type == AST_While) {
                    n->jump_index = p->jumps.data[i];
                    append(p->nodes, n);
                }
            }
            if (n->jump_index < 0) ERR("ERROR in %s on line %ld: tried to use continue outside a while loop\n", n->file, n->line)
        } else if (n->type == AST_Fn) {
            free_node(n);
            func = calloc(1, sizeof(AST_Function));
            func->name = format_str(CURRENT_TOK.length + 1, "%.*s", CURRENT_TOK.length, CURRENT_TOK.start);
            func->line = CURRENT_TOK.line;
            for (int i = 0; i < p->funcs.index; i++)
                if (!strcmp(p->funcs.data[i]->name, func->name))
                    ERR("ERROR in %s on line %ld: cant define function %s multiple times\n", CURRENT_TOK.file, CURRENT_TOK.line, func->name)

            p->tok_index++;
            ASSERT((p->jumps.index >= 0), "ERROR in %s on line %ld: unclosed block before function %s\n",  CURRENT_TOK.file,CURRENT_TOK.line, func->name)
            ASSERT((CURRENT_TOK.type == Tok_Left_Paren), "ERROR in %s on line %ld: need left paren to open function arguments\n",  CURRENT_TOK.file,CURRENT_TOK.line)
            p->tok_index++;

            //switch parser to parse the function
            if (temp_nodes != NULL) ERR("ERROR in %s on line %ld: cant define functions inside other functions\n", CURRENT_TOK.file, LAST_TOK.line)
            temp_nodes = p->nodes.data;
            temp_nodes_index = p->nodes.index;
            temp_nodes_capacity = p->nodes.capacity;
            p->nodes.data = calloc(10, sizeof(Node*));
            p->nodes.index = 0;
            p->nodes.capacity = 10;
            p->parsing_function = true;

            //parse arguments into func->args
            int64_t args_capacity = 1;
            func->arity = 0;
            func->args = calloc(args_capacity, sizeof(AST_Value*));
            while (1) {
                Node *n = expr(p, NULL, false);
                if (n->type == AST_Identifier) {
                    append_verbose(func->args, n->value, func->arity, args_capacity)
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
            append(p->nodes, n);
        }
    }

    report_time("PARSING      time: %f seconds\n");

    debug("\n----------\nINTERPRETING\n")
    Functions funcs = {calloc(10, sizeof(Function)), 0, 10};

    append(funcs, compile_func(&((AST_Function){NULL, p->nodes, 0, NULL, 0})));
    funcs.data[0].name = "MAIN";

    append(funcs.data[0].code, OP_DONE);
    append(funcs.data[0].locs, ((Location){"DONE", 0}));


    for (int i = 0; i < p->funcs.index; i++) {
        append(funcs, compile_func(p->funcs.data[i]));
    }
#ifdef PROFILE
    instr_profiler = calloc(funcs.index, sizeof(u_int64_t));
    time_profiler = calloc(funcs.index, sizeof(double));
#endif

    report_time("COMPILING    time: %f seconds\n");

    vm.stack_top = vm.stack;
    vm.funcs = funcs;

    vm.vars = new_map(8);

    Variable *argcc = calloc(1, sizeof(Variable));
    Variable *argvv = calloc(1, sizeof(Variable));

    if (suda_argc != NULL) {
        argcc->value = (Value){ Value_Number, .val.num=NUM(suda_argc->value), true, hash("argc", 4) };
        argvv->value = (Value){ Value_Array, .val.array=NULL, true, hash("argv", 4) };

        Value *x = calloc(argcc->value.val.num + 1, sizeof(Value));
        x[0].val.num = argcc->value.val.num + 1;
        x[0].type = Value_Array;
        for (int i = 1; i < argcc->value.val.num + 1; i++) {
            size_t len = strlen(suda_argv[i].value);
            x[i] = (Value){ Value_String, .val.str={format_str(len - 1, "%.*s", len - 2, STR(suda_argv[i].value) + 1), len - 2}, true, 0 };
        }

        argvv->value.val.array = x;
    } else {
        argcc->value = (Value){ Value_Number, .val.num=0, true, hash("argc", 4) };
        argvv->value = (Value){ Value_Array, .val.array=NULL, true, hash("argv", 4) };
        Value *x = calloc(1, sizeof(Value));
        x[0].val.num = 1;
        x[0].type = Value_Array;

        argvv->value.val.array = x;
    }

    insert_entry(vm.vars, argcc->value.hash, Entry_Variable, argcc);
    insert_entry(vm.vars, argvv->value.hash, Entry_Variable, argvv);

    vm.call_stack[0] = (Call_Frame){
        &vm.funcs.data[0],
        vm.stack,
        0,
        ((Location){file_path, 0}),
#ifdef PROFILE
        0
#endif
    };
    vm.call_stack_count++;
    vm.func = &vm.funcs.data[0];

    if (disassemble_bytecode) disassemble(&vm);
    else if (assemble) {
        emit_asm(&vm);
        report_time("CODEGEN      time: %f seconds\n");
    }
    else {
        run(&vm);
        report_time("RUNNING      time: %f seconds\n");
    }

#ifdef PROFILE
    printf("FUNCTION NAME         INSTR COUNT    %% OF TOTAL      TIME     %% OF TOTAL\n");

    u_int64_t instr_total = 0;
    double time_total = 0;
    for (int i = 0; i < vm.funcs.index; i++) {
        instr_total += instr_profiler[i];
        time_total += time_profiler[i];
    }

    printf("OVERALL             : %-15ld: 100%%       : %9.5f: 100%%\n", instr_total, time_total);

    u_int64_t highest = 0;
    int current_index = 0;

    while (1) {
        for (int i = 0; i < vm.funcs.index; i++) {
            if (instr_profiler[i] > highest) {
                highest = instr_profiler[i];
                current_index = i;
            }
        }
        if (highest == 0) break;
        printf("%-20s: %-15ld: %9.6f%% : %9.6f: %9.6f%%\n", vm.funcs.data[current_index].name, 
            instr_profiler[current_index], ((double)instr_profiler[current_index] / instr_total) * 100,
            time_profiler[current_index], ((double)time_profiler[current_index] / time_total) * 100);
        instr_profiler[current_index] = 0;
        highest = 0;
    }
    if (instructions) { 
        printf("\nOPS:\n");
#define X(x) if (op_profiler[x]) printf("%-26s: %-15ld: %9.6f%%\n", find_op_code(x), op_profiler[x], ((double)op_profiler[x] / instr_total) * 100);
        ops
#undef X
    }

    free(instr_profiler);
    free(time_profiler);
#endif


    if (time) {
        printf("OVERALL      time: %f seconds\n",
            ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - 
            ((double)tfinal.tv_sec + 1.0e-9*tfinal.tv_nsec));
    }

    free_mem(0);
}
