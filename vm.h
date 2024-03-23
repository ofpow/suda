#pragma once

#define STACK_SIZE 16384
#define LOCALS_MAX 256
#define CALL_STACK_SIZE 64

#define FIRST_BYTE(_val) (u_int8_t)((_val) >> 8)
#define SECOND_BYTE(_val) (u_int8_t)((_val) & 0xFF)
#define COMBYTE(_byte1, _byte2) (((_byte1) << 8) | (_byte2))
#define read_index (COMBYTE(vm->func->code.data[i + 1], vm->func->code.data[i + 2]))
#define INVALID_LOC ((Location){ "INVALID LOCATION", -1 })
#define make_loc(_file, _line) ((Location){ _file, _line })
#define append_code(_code, _loc) append(c->func.code, _code); append(c->func.locs, _loc)
#define get_loc vm->func->locs.data[i].file, vm->func->locs.data[i].line
#define stack_pop (*--vm->stack_top)
#define stack_push(_val) do { \
    *vm->stack_top = (_val);  \
    vm->stack_top++;          \
} while (0)                   \

#define binary_op(op, msg) do {                                                     \
    Value op2 = stack_pop;                                                          \
    Value op1 = stack_pop;                                                          \
    if (op1.type == Value_Number && op2.type == Value_Number) {                     \
        stack_push(((Value) {                                                       \
            Value_Number,                                                           \
            .val.num=(op1.val.num op op2.val.num),                                  \
            false,                                                                  \
            0                                                                       \
        }));                                                                        \
    } else ERR(msg, get_loc, find_value_type(op1.type), find_value_type(op2.type))  \
} while (0);                                                                        \
break                                                                               \

#define ops \
    X(OP_CONSTANT)\
    X(OP_PRINTLN)\
    X(OP_DEFINE_GLOBAL)\
    X(OP_SET_GLOBAL)\
    X(OP_GET_GLOBAL)\
    X(OP_SET_LOCAL)\
    X(OP_GET_LOCAL)\
    X(OP_ADD)\
    X(OP_SUBTRACT)\
    X(OP_MULTIPLY)\
    X(OP_DIVIDE)\
    X(OP_LESS)\
    X(OP_LESS_EQUAL)\
    X(OP_GREATER)\
    X(OP_GREATER_EQUAL)\
    X(OP_IS_EQUAL)\
    X(OP_AND)\
    X(OP_OR)\
    X(OP_NOT)\
    X(OP_NOT_EQUAL)\
    X(OP_JUMP_IF_FALSE)\
    X(OP_JUMP)\
    X(OP_ARRAY)\
    X(OP_GET_ELEMENT)\
    X(OP_SET_ELEMENT)\
    X(OP_START_IF)\
    X(OP_POP)\
    X(OP_LEN)\
    X(OP_CAST_STR)\
    X(OP_CAST_NUM)\
    X(OP_CALL)\

typedef enum {
#define X(x) x,
    ops
#undef X
} Op_Code;

char *find_op_code(Op_Code code) {
    switch (code) {
#define X(x) case x: return #x;
        ops
#undef X
    }
    return "unreachable";
}

Op_Code ast_to_op_code(AST_Type type) {
    switch (type) {
        case AST_Literal: return OP_CONSTANT;
        case AST_Println: return OP_PRINTLN;
        case AST_Var_Assign: return OP_DEFINE_GLOBAL;
        case AST_Add: return OP_ADD;
        case AST_Sub: return OP_SUBTRACT;
        case AST_Mult: return OP_MULTIPLY;
        case AST_Div: return OP_DIVIDE;
        case AST_Less: return OP_LESS;
        case AST_Less_Equal: return OP_LESS_EQUAL;
        case AST_Greater: return OP_GREATER;
        case AST_Greater_Equal: return OP_GREATER_EQUAL;
        case AST_Is_Equal: return OP_IS_EQUAL;
        case AST_And: return OP_AND;
        case AST_Or: return OP_OR;
        case AST_Not: return OP_NOT;
        case AST_Not_Equal: return OP_NOT_EQUAL;
        default: ERR("no op code for ast type %d\n", type);
    }
    return 0;
}

define_array(Code, u_int8_t);
define_array(Constants, Value);
define_array(Arrays, Value*);

typedef struct Location {
    const char *file;
    int64_t line;
} Location;

define_array(Locations, Location);

typedef struct Function {
    Code code;
    Locations locs;

    Constants constants;

    Arrays arrays;

    int arity;
    char *name;
} Function;

typedef struct Call_Frame {
    Function *func;
    Value *slots;
} Call_Frame;

define_array(Functions, Function);

typedef struct Local {
    AST_Value *name;
    int depth;
} Local;

typedef struct Compiler {
    Jump_Indices if_indices;
    Jump_Indices while_indices;

    Function func;

    Local locals[LOCALS_MAX];
    u_int8_t locals_count;
    int64_t depth;
} Compiler;

typedef struct VM {
    Function *func;

    Map *vars;

    Value stack[STACK_SIZE];
    Value *stack_top;

    Functions funcs;
    Call_Frame call_stack[STACK_SIZE];
    int call_stack_count;
} VM;

u_int8_t resolve_local(AST_Value *name, Compiler *c) {
    for (int i = c->locals_count - 1; i >= 0; i--) {
        if (ast_value_equal(name, c->locals[i].name)) return i;
    }
    return 0;
}

bool is_local(AST_Value *name, Compiler *c) {
    for (int i = c->locals_count - 1; i >= 0; i--) {
        if (ast_value_equal(name, c->locals[i].name)) return true;
    }
    return false;
}

u_int16_t resolve_func(AST_Value *func) {
    for (u_int16_t i = 0; i < p->funcs.index; i++) {
        if (!strcmp(STR(func->value), p->funcs.data[i]->name)) return i + 1;
    }
    ERR("cant resolve func %s\n", STR(func->value))
    return 0;
}

void print_array(VM *vm, Value *val) {
    Value *array = vm->func->arrays.data[val->val.num];
    if (array[0].val.num < 2) printf("[]\n");

    int64_t str_len = 2;
    char *str = format_str(str_len, "[");

    for (int i = 1; i < array[0].val.num; i++) {
        if (array[i].type == Value_Number) {
            int64_t len = num_len(array[i].val.num) + 2;
            char *num = format_str(len, "%d", array[i].val.num);
            str_len += len;
            str = realloc(str, str_len);
            strcat(str, num);
            strcat(str, ", ");
            free(num);
        } else if (array[i].type == Value_Identifier) {
            str_len += (strlen(array[i].val.str) + 2);
            str = realloc(str, str_len);
            strcat(str, array[i].val.str);
            strcat(str, ", ");
        } else if (array[i].type == Value_String) {
            int len = strlen(array[i].val.str) + 4;
            str_len += len;
            str = realloc(str, str_len);
            char *x = format_str(len + 1, "\"%s\"", array[i].val.str);
            strcat(str, x);
            strcat(str, ", ");
            free(x);
        } else ERR("cant print %s as part of array\n", find_value_type(array[i].type))
    }
    str[str_len - 3] = ']';
    str[str_len - 2] = 0;
    printf("%s\n", str);
    free(str);
}

void disassemble(VM *vm) {
    for (int i = 0; i < vm->func->code.index; i++) {
        switch(vm->func->code.data[i]) {
            case OP_CONSTANT:
                printf("%-6d OP_CONSTANT:      index %d\n", i, COMBYTE(vm->func->code.data[i + 1], vm->func->code.data[i + 2]));
                i += 2;
                break;
            case OP_ARRAY:
                printf("%-6d OP_ARRAY:         index %d\n", i, COMBYTE(vm->func->code.data[i + 1], vm->func->code.data[i + 2]));
                i += 2;
                break;
            case OP_GET_ELEMENT:
                printf("%-6d OP_GET_ELEMENT\n", i);
                break;
            case OP_SET_ELEMENT:
                printf("%-6d OP_SET_ELEMENT\n", i);
                break;
            case OP_PRINTLN:
                printf("%-6d OP_PRINTLN\n", i);
                break;
            case OP_DEFINE_GLOBAL:
                printf("%-6d OP_DEFINE_GLOBAL\n", i);
                i += 2;
                break;
            case OP_ADD:
                printf("%-6d OP_ADD\n", i);
                break;
            case OP_SUBTRACT:
                printf("%-6d OP_SUBTRACT\n", i);
                break;
            case OP_MULTIPLY:
                printf("%-6d OP_MULTIPLY\n", i);
                break;
            case OP_DIVIDE:
                printf("%-6d OP_DIVIDE\n", i);
                break;
            case OP_LESS:
                printf("%-6d OP_LESS\n", i);
                break;
            case OP_LESS_EQUAL:
                printf("%-6d OP_LESS_EQUAL\n", i);
                break;
            case OP_GREATER:
                printf("%-6d OP_GREATER\n", i);
                break;
            case OP_GREATER_EQUAL:
                printf("%-6d OP_GREATER_EQUAL\n", i);
                break;
            case OP_IS_EQUAL:
                printf("%-6d OP_IS_EQUAL\n", i);
                break;
            case OP_AND:
                printf("%-6d OP_AND\n", i);
                break;
            case OP_OR:
                printf("%-6d OP_OR\n", i);
                break;
            case OP_NOT:
                printf("%-6d OP_NOT\n", i);
                break;
            case OP_NOT_EQUAL:
                printf("%-6d OP_NOT_EQUAL\n", i);
                break;
            case OP_SET_GLOBAL:
                printf("%-6d OP_SET_GLOBAL \n", i);
                i += 2;
                break;
            case OP_GET_GLOBAL:
                printf("%-6d OP_GET_GLOBAL \n", i);
                i += 2;
                break;
            case OP_JUMP_IF_FALSE:
                printf("%-6d OP_JUMP_IF_FALSE: offset %d\n", i, COMBYTE(vm->func->code.data[i + 1], vm->func->code.data[i + 2]));
                i += 2;
                break;
            case OP_START_IF:
                printf("%-6d OP_START_IF:      offset %d\n", i, COMBYTE(vm->func->code.data[i + 1], vm->func->code.data[i + 2]));
                i += 2;
                break;
            case OP_JUMP:
                printf("%-6d OP_JUMP:          index %d\n", i, COMBYTE(vm->func->code.data[i + 1], vm->func->code.data[i + 2]));
                i += 2;
                break;
            case OP_SET_LOCAL:
                printf("%-6d OP_SET_LOCAL:     index %d\n", i, COMBYTE(vm->func->code.data[i + 1], vm->func->code.data[i + 2]));
                i += 2;
                break;
            case OP_GET_LOCAL:
                printf("%-6d OP_GET_LOCAL:     index %d\n", i, COMBYTE(vm->func->code.data[i + 1], vm->func->code.data[i + 2]));
                i += 2;
                break;
            case OP_POP:
                printf("%-6d OP_POP:           amount: %d\n", i, COMBYTE(vm->func->code.data[i + 1], vm->func->code.data[i + 2])); 
                i += 2;
                break;
            case OP_LEN:
                printf("%-6d OP_LEN\n", i);
                break;
            case OP_CAST_STR:
                printf("%-6d OP_CAST_STR\n", i);
                break;
            case OP_CAST_NUM:
                printf("%-6d OP_CAST_NUM\n", i);
                break;
            case OP_CALL:
                printf("%-6d OP_CALL\n", i);
                i += 2;
                break;
            default:
                ERR("ERROR in %s on line %ld: cant disassemble op type %d\n", get_loc,  vm->func->code.data[i]);
        }
    }
}

void compile_constant(Node *n, Compiler *c) {
    if (n->value->type == Value_Number)
        append(c->func.constants, ((Value){n->value->type, .val.num=NUM(n->value->value), false, 0}));
    else if (n->value->type == Value_String)
        append(c->func.constants, ((Value){n->value->type, .val.str=n->value->value, false, 0}));
    else if (n->value->type == Value_Identifier)
        append(c->func.constants, ((Value){n->value->type, .val.str=n->value->value, false, n->value->hash}));
    else
        ERR("ERROR in %s on line %ld: cant compile constant of type %d\n", n->file, n->line, n->value->type)

    u_int16_t index = c->func.constants.index - 1;
    append_code(OP_CONSTANT, make_loc(n->file, n->line));
    append_code(FIRST_BYTE(index), INVALID_LOC);
    append_code(SECOND_BYTE(index), INVALID_LOC);
}

void compile_expr(Node *n, Compiler *c) {
    switch (n->type) {
        case AST_Literal:
            compile_constant(n, c);
            break;
        case AST_Identifier:
            if (is_local(n->value, c)) {
                append_code(OP_GET_LOCAL, make_loc(n->file, n->line));
                append_code(resolve_local(n->value, c), INVALID_LOC);
            } else {
                append(c->func.constants, ((Value){Value_Identifier, .val.str=n->value->value, false, n->value->hash})); // name
                u_int16_t index = c->func.constants.index - 1;
                append_code(OP_GET_GLOBAL, make_loc(n->file, n->line));
                append_code(FIRST_BYTE(index), INVALID_LOC);
                append_code(SECOND_BYTE(index), INVALID_LOC);
            }
            break;
        case AST_Add:
        case AST_Sub:
        case AST_Mult:
        case AST_Div:
        case AST_Less:
        case AST_Less_Equal:
        case AST_Greater:
        case AST_Greater_Equal:
        case AST_Is_Equal:
        case AST_And:
        case AST_Or:
        case AST_Not_Equal:
            compile_expr(n->left, c);
            compile_expr(n->right, c);
            append_code(ast_to_op_code(n->type), make_loc(n->file, n->line));
            break;
        case AST_Not:
            compile_expr(n->left, c);
            append_code(OP_NOT, make_loc(n->file, n->line));
            break;
        case AST_Array:{
            int64_t array_len = NUM(n->value[0].value);
            Value *array = calloc(array_len, sizeof(Value));
            for (int i = 0; i < array_len; i++) {
                switch (n->value[i].type) {
                    case Value_Array:
                        array[i].type = Value_Array;
                        array[i].val.num = NUM(n->value[i].value);
                        break;
                    case Value_Number:
                        array[i].type = Value_Number;
                        array[i].val.num = NUM(n->value[i].value);
                        break;
                    case Value_String:
                        array[i].type = Value_String;
                        char *s = format_str(strlen(n->value[i].value), "%.*s", strlen(n->value[i].value) - 2, STR(n->value[i].value) + 1);
                        array[i].val.str = s;
                        array[i].mutable = true;
                        free(n->value[i].value);
                        n->value[i].value = NULL;
                        break;
                    case Value_Identifier:
                        array[i].type = Value_Identifier;
                        array[i].val.str = STR(n->value[i].value);
                        array[i].mutable = true;
                        array[i].hash = n->value[i].hash;
                        n->value[i].value = NULL;
                        break;
                    default: ERR("ERROR in %s on line %ld: cant do array type %s\n", n->file, n->line, find_value_type(n->value[i].type))
                }
            }

            append(c->func.arrays, array);
            append_code(OP_ARRAY, make_loc(n->file, n->line));
            u_int16_t index = c->func.arrays.index - 1;
            append_code(FIRST_BYTE(index), INVALID_LOC);
            append_code(SECOND_BYTE(index), INVALID_LOC);
            break;}
        case AST_At:{
            compile_expr(n->left, c); // the index
            compile_constant(n, c); //var name
            append_code(OP_GET_ELEMENT, make_loc(n->file, n->line));
            break;}
        case AST_Len:
            compile_expr(n->left, c);
            append_code(OP_LEN, make_loc(n->file, n->line));
            break;
        case AST_Cast_Str:
            compile_expr(n->left, c);
            append_code(OP_CAST_STR, make_loc(n->file, n->line));
            break;
        case AST_Cast_Num:
            compile_expr(n->left, c);
            append_code(OP_CAST_NUM, make_loc(n->file, n->line));
            break;
        default: ERR("ERROR in %s on line %ld: cant compile node type %s as expr\n", n->file, n->line, find_ast_type(n->type));
    }
}

void compile(Node **nodes, int64_t nodes_size, Compiler *c) {
    for (int i = 0; i < nodes_size; i++) {
        switch (nodes[i]->type) {
            case AST_Println:
                compile_expr(nodes[i]->left, c);
                append_code(OP_PRINTLN, make_loc(nodes[i]->file, nodes[i]->line));
                break;
            case AST_Var_Assign:
                compile_expr(nodes[i]->left, c); // value
                
                if (c->depth > 0) {
                    if (c->locals_count >= LOCALS_MAX - 1) ERR("ERROR in %s on line %ld: too many local vars\n", nodes[i]->file, nodes[i]->line)
                    
                    for (int i = c->locals_count - 1; i >= 0; i--) {
                        if (c->locals[i].depth != -1 && c->locals[i].depth < c->depth) break;

                        if (ast_value_equal(c->locals[i].name, nodes[i]->value)) 
                            ERR("ERROR in %s on line %ld: cant redefine local variable %s\n", nodes[i]->file, nodes[i]->line, STR(nodes[i]->value->value))
                    }

                    c->locals[c->locals_count].name = nodes[i]->value;
                    c->locals[c->locals_count].depth = c->depth;
                    c->locals_count++;
                } else {
                    append(c->func.constants, ((Value){Value_Identifier, .val.str=nodes[i]->value->value, false, nodes[i]->value->hash})); // name
                    u_int16_t index = c->func.constants.index - 1;
                    append_code(OP_DEFINE_GLOBAL, make_loc(nodes[i]->file, nodes[i]->line));
                    append_code(FIRST_BYTE(index), INVALID_LOC);
                    append_code(SECOND_BYTE(index), INVALID_LOC);
                }
                break;
            case AST_Identifier:
                compile_expr(nodes[i]->left, c); // value
                
                if (is_local(nodes[i]->value, c)) {
                    append_code(OP_SET_LOCAL, make_loc(nodes[i]->file, nodes[i]->line));
                    append_code(resolve_local(nodes[i]->value, c), INVALID_LOC);
                } else {
                    append(c->func.constants, ((Value){Value_Identifier, .val.str=nodes[i]->value->value, false, nodes[i]->value->hash})); // name
                    u_int16_t index = c->func.constants.index - 1;
                    append_code(OP_SET_GLOBAL, make_loc(nodes[i]->file, nodes[i]->line));
                    append_code(FIRST_BYTE(index), INVALID_LOC);
                    append_code(SECOND_BYTE(index), INVALID_LOC);
                }
                break;
            case AST_If:
                compile_expr(nodes[i]->left, c);

                append(c->if_indices, c->func.code.index);
                append_code(OP_START_IF, make_loc(nodes[i]->file, nodes[i]->line));
                append_code(0, INVALID_LOC);
                append_code(0, INVALID_LOC);

                c->depth++;
                break;
            case AST_While:
                append(c->while_indices, c->func.code.index);
                compile_expr(nodes[i]->left, c);

                append(c->while_indices, c->func.code.index);
                append_code(OP_JUMP_IF_FALSE, make_loc(nodes[i]->file, nodes[i]->line));
                append_code(0, INVALID_LOC);
                append_code(0, INVALID_LOC);
                c->depth++;
                break;
            case AST_Semicolon:;
                c->depth--;

                u_int16_t amount = 0;
                while(c->locals_count > 0 && c->locals[c->locals_count - 1].depth > c->depth) { amount++; c->locals_count--; }

                if (amount > 0) {
                    append_code(OP_POP, make_loc(nodes[i]->file, nodes[i]->line));
                    append_code(FIRST_BYTE(amount), INVALID_LOC);
                    append_code(SECOND_BYTE(amount), INVALID_LOC);
                }

                if (nodes[nodes[i]->jump_index]->type == AST_While) {
                    u_int16_t index = c->while_indices.data[--c->while_indices.index];
                    u_int16_t x = c->func.code.index + 3 - index;
                    c->func.code.data[index + 1] = FIRST_BYTE(x);
                    c->func.code.data[index + 2] = SECOND_BYTE(x);

                    index = c->while_indices.data[--c->while_indices.index];
                    append_code(OP_JUMP, make_loc(nodes[i]->file, nodes[i]->line));
                    append_code(FIRST_BYTE(index), INVALID_LOC);
                    append_code(SECOND_BYTE(index), INVALID_LOC);
                } else if (nodes[nodes[i]->jump_index]->type == AST_If) {
                    u_int16_t index = c->if_indices.data[--c->if_indices.index];

                    u_int16_t offset = c->func.code.index - index;
                    c->func.code.data[index + 1] = FIRST_BYTE(offset);
                    c->func.code.data[index + 2] = SECOND_BYTE(offset);
                } else {
                    while (1) {
                        if (c->if_indices.index < 1) break;
                        u_int16_t index = c->if_indices.data[--c->if_indices.index];
                        if (c->func.code.data[index] == OP_START_IF) break;
                        else if (c->func.code.data[index] == OP_JUMP_IF_FALSE) continue;
                        c->func.code.data[index + 1] = FIRST_BYTE(c->func.code.index);
                        c->func.code.data[index + 2] = SECOND_BYTE(c->func.code.index);
                    }
                }
                break;
            case AST_Else:;
                u_int16_t index = c->if_indices.data[--c->if_indices.index];
                u_int16_t offset = c->func.code.index - index + 3;
                c->func.code.data[index + 1] = FIRST_BYTE(offset);
                c->func.code.data[index + 2] = SECOND_BYTE(offset);
                append(c->if_indices, c->func.code.index);
                append_code(OP_JUMP, make_loc(nodes[i]->file, nodes[i]->line));
                append_code(0, INVALID_LOC);
                append_code(0, INVALID_LOC);
                break;
            case AST_Elif:{
                u_int16_t index = c->if_indices.data[--c->if_indices.index];
                u_int16_t offset = c->func.code.index - index + 3;
                c->func.code.data[index + 1] = FIRST_BYTE(offset);
                c->func.code.data[index + 2] = SECOND_BYTE(offset);

                append(c->if_indices, c->func.code.index);
                append_code(OP_JUMP, make_loc(nodes[i]->file, nodes[i]->line));
                append_code(0, INVALID_LOC);
                append_code(0, INVALID_LOC);
                
                compile_expr(nodes[i]->left, c);

                append(c->if_indices, c->func.code.index);
                append_code(OP_JUMP_IF_FALSE, make_loc(nodes[i]->file, nodes[i]->line));
                append_code(0, INVALID_LOC);
                append_code(0, INVALID_LOC);
                break;}
            case AST_At:
                compile_constant(nodes[i], c); // var name
                compile_expr(nodes[i]->left, c); // index
                if (nodes[i]->right) {// new value
                    compile_expr(nodes[i]->right, c); 
                } else {
                    compile_expr(nodes[i]->left->left, c);
                }
                append_code(OP_SET_ELEMENT, make_loc(nodes[i]->file, nodes[i]->line));
                break;
            case AST_Fn_Call:{
                u_int16_t index = resolve_func(nodes[i]->value);
                append_code(OP_CALL, make_loc(nodes[i]->file, nodes[i]->line));
                append_code(FIRST_BYTE(index), INVALID_LOC);
                append_code(SECOND_BYTE(index), INVALID_LOC);
                break;}
            default: ERR("ERROR in %s on line %ld: cant compile node type %s\n", nodes[i]->file, nodes[i]->line, find_ast_type(nodes[i]->type))
        }
    }
}

Function compile_func(AST_Function *func){
    Compiler c = {0};
    c.if_indices = (Jump_Indices){calloc(10, sizeof(int64_t)), 0, 10};
    c.while_indices = (Jump_Indices){calloc(10, sizeof(int64_t)), 0, 10};
    c.func.code = (Code){calloc(10, sizeof(u_int8_t)), 0, 10};
    c.func.constants = (Constants){calloc(10, sizeof(Value)), 0, 10};
    c.func.arrays = (Arrays){calloc(10, sizeof(Value*)), 0, 10};
    c.func.locs = (Locations){calloc(10, sizeof(Location)), 0, 10};

    compile(func->nodes.data, func->nodes.index, &c);

    free(c.if_indices.data);
    free(c.while_indices.data);
    return c.func;
}

void run(VM *vm) {
    for (int i = 0; i < vm->func->code.index; i++) {
        debug("%-6d %s\n", i, find_op_code(vm->func->code.data[i]));
        switch (vm->func->code.data[i]) {
            case OP_CONSTANT:;
                u_int16_t index = read_index;
                stack_push(vm->func->constants.data[index]);
                i += 2;
                break;
            case OP_PRINTLN:;
                Value print = stack_pop;
                if (print.type == Value_Number) {
                    printf("%ld\n", print.val.num);
                } else if (print.type == Value_String) {
                    printf("%s\n", print.val.str);
                    if (print.mutable == true) free(print.val.str);
                } else if (print.type == Value_Array) {
                    print_array(vm, &print);
                } else
                    ERR("ERROR in %s on line %ld: cant print type %s\n", get_loc, find_value_type(print.type))
                break;
            case OP_DEFINE_GLOBAL:;
                Value name = vm->func->constants.data[read_index];
                i += 2;
                Value value = stack_pop;
                Variable *var = calloc(1, sizeof(Variable));
                var->name = name.val.str;
                var->value = value;
                var->index = -1;
                insert_entry(vm->vars, name.hash, Entry_Variable, var);
                break;
            case OP_ADD:;
                Value op2 = stack_pop;
                Value op1 = stack_pop;

                if (op1.type == Value_Number && op2.type == Value_Number) {
                    stack_push(((Value) {
                        Value_Number,
                        .val.num=(op1.val.num + op2.val.num),
                        false,
                        0
                    }));
                } else {
                    char *str1;
                    char *str2;
                    int64_t op1_len = 0;
                    int64_t op2_len = 0;
                    bool free1 = false;
                    bool free2 = false;

                    if (op1.type == Value_String) {
                        op1_len = strlen(op1.val.str);
                        str1 = op1.val.str;
                        if (op1.mutable) free1 = true;
                    } else if (op1.type == Value_Number) {
                        op1_len = num_len(op1.val.num);
                        str1 = format_str(op1_len + 1, "%ld", op1.val.num);
                        free1 = true;
                    }
                    if (op2.type == Value_String) {
                        op2_len = strlen(op2.val.str);
                        str2 = op2.val.str;
                        if (op2.mutable) free2 = true;
                    } else if (op2.type == Value_Number) {
                        op2_len = num_len(op2.val.num);
                        str2 = format_str(op2_len + 1, "%ld", op2.val.num);
                        free2 = true;
                    }

                    stack_push(((Value){
                        Value_String,
                        .val.str=format_str(op1_len + op2_len + 1, "%s%s", str1, str2),
                        true,
                        0
                    }));

                    if (free1) free(str1);
                    if (free2) free(str2);
                }
                break;
            case OP_SUBTRACT:
                binary_op(-, "ERROR in %s on line %ld: cant subtract type %s and %s\n");
            case OP_MULTIPLY:
                binary_op(*, "ERROR in %s on line %ld: cant multiply type %s and %s\n");
            case OP_DIVIDE:
                binary_op(/, "ERROR in %s on line %ld: cant divide type %s and %s\n");
            case OP_LESS:
                binary_op(<, "ERROR in %s on line %ld: cant less than type %s and %s\n");
            case OP_LESS_EQUAL:
                binary_op(<=, "ERROR in %s on line %ld: cant less equal than type %s and %s\n");
            case OP_GREATER:
                binary_op(>, "ERROR in %s on line %ld: cant greater than type %s and %s\n");
            case OP_GREATER_EQUAL:
                binary_op(>=, "ERROR in %s on line %ld: cant greater equal than type %s and %s\n");
            case OP_AND:
                binary_op(==, "ERROR in %s on line %ld: cant logical and type %s and %s\n");
            case OP_OR:
                binary_op(||, "ERROR in %s on line %ld: cant logical or type %s and %s\n");
            case OP_NOT:;
                Value op = stack_pop;
                ASSERT(op.type == Value_Number, "cant logical not type %s", find_value_type(op.type))
                stack_push(((Value) {
                    Value_Number,
                    .val.num=(!(op.val.num)),
                    false,
                    0 
                }));
                break;
            case OP_NOT_EQUAL:{
                Value op2 = stack_pop;
                Value op1 = stack_pop;

                if (op1.type == Value_Number && op2.type == Value_Number) {
                    stack_push(((Value) {
                        Value_Number,
                        .val.num=(op1.val.num != op2.val.num),
                        false,
                        0
                    }));
                } else if (op1.type == Value_String && op2.type == Value_String) {
                    stack_push(((Value){
                        Value_Number,
                        .val.num=!!strcmp(op1.val.str, op2.val.str),
                        false,
                        0
                    }));
                } else ERR("ERROR in %s on line %ld: cant not equal type %s and %s\n", get_loc, find_value_type(op1.type), find_value_type(op2.type))
                break;}
            case OP_IS_EQUAL: {
                Value op2 = stack_pop;
                Value op1 = stack_pop;

                if (op1.type == Value_Number && op2.type == Value_Number) {
                    stack_push(((Value) {
                        Value_Number,
                        .val.num=(op1.val.num == op2.val.num),
                        false,
                        0
                    }));
                } else if (op1.type == Value_String && op2.type == Value_String) {
                    stack_push(((Value){
                        Value_Number,
                        .val.num=!strcmp(op1.val.str, op2.val.str),
                        false,
                        0
                    }));
                } else ERR("ERROR in %s on line %ld: cant is equal type %s and %s\n", get_loc, find_value_type(op1.type), find_value_type(op2.type))
                break;}
            case OP_SET_GLOBAL:{
                Value name = vm->func->constants.data[read_index];
                i += 2;
                Value value = stack_pop;

                Variable *var = get_entry(vm->vars->entries, vm->vars->capacity, name.hash)->value;
                if (var == NULL) ERR("ERROR in %s on line %ld: tried to set nonexistent global %s\n", get_loc, name.val.str)
                else if (var->value.type == Value_Array) {
                    Value *array = vm->func->arrays.data[var->value.val.num];
                    for (int i = 1; i < array[0].val.num; i++) {
                        if (array[i].type == Value_String && array[i].mutable == true) free(array[i].val.str);
                    }
                    free(vm->func->arrays.data[var->value.val.num]);
                    Value *new_array = vm->func->arrays.data[value.val.num];
                    array = calloc(new_array[0].val.num, sizeof(Value));

                    for (int i = 0; i < new_array[0].val.num; i++) {
                        if (new_array[i].type == Value_String && new_array[i].mutable)
                            array[i] = (Value){Value_String, .val.str=strdup(new_array[i].val.str), true, 0};
                        else 
                            array[i] = new_array[i];
                    }
                    vm->func->arrays.data[var->value.val.num] = array;
                } else {
                var->value = value;}
                break;}
            case OP_START_IF:
            case OP_JUMP_IF_FALSE:{
                Value val = stack_pop;
                if (!val.val.num) {
                    i += read_index - 1;
                } else {
                    i += 2;
                }
                break;}
            case OP_JUMP:
                i = read_index - 1;
                break;
            case OP_ARRAY:{
                u_int16_t index = read_index;
                stack_push(((Value) {
                    Value_Array,      
                    .val.num=index,   
                    false,            
                    0                 
                }));                  
                i += 2;
                break;}
            case OP_GET_ELEMENT:{
                Value array = stack_pop;
                Value index = stack_pop;

                int64_t arr_index = ((Variable*)get_entry(vm->vars->entries, vm->vars->capacity, array.hash)->value)->value.val.num;

                if (vm->func->arrays.data[arr_index][index.val.num].type == Value_Identifier) {
                    Variable *var = get_entry(vm->vars->entries, vm->vars->capacity, vm->func->arrays.data[arr_index][index.val.num].hash)->value;
                    stack_push(var->value);
                } else if (vm->func->arrays.data[arr_index][index.val.num].type == Value_String && vm->func->arrays.data[arr_index][index.val.num].mutable) stack_push(((Value) {
                    Value_String, 
                    .val.str=vm->func->arrays.data[arr_index][index.val.num].val.str,
                    false,
                    0
                }));
                else stack_push(vm->func->arrays.data[arr_index][index.val.num]);

                break;}
            case OP_SET_ELEMENT:{
                Value new_val = stack_pop;
                Value index = stack_pop;
                Value array = stack_pop;

                int64_t var_index = ((Variable*)get_entry(vm->vars->entries, vm->vars->capacity, array.hash)->value)->value.val.num;
                if (vm->func->arrays.data[var_index][index.val.num].type == Value_String && vm->func->arrays.data[var_index][index.val.num].mutable) free(vm->func->arrays.data[var_index][index.val.num].val.str);
                vm->func->arrays.data[var_index][index.val.num] = new_val;

                break;}
            case OP_GET_GLOBAL:{
                Value var_name = vm->func->constants.data[read_index];
                i += 2;
                Variable *var = get_entry(vm->vars->entries, vm->vars->capacity, var_name.hash)->value;
                if (var == NULL) ERR("ERROR in %s on line %ld: tried to get nonexistent var %s\n", get_loc, var_name.val.str);
                stack_push(var->value);
                break;}
            case OP_GET_LOCAL: {
                Value val = vm->stack[vm->func->code.data[++i]];
                if (val.type == Value_String && val.mutable) stack_push(((Value) {
                    Value_String, 
                    .val.str=val.val.str,
                    false,
                    0
                }));
                else stack_push(val);
                i += 2;
                break;}
            case OP_SET_LOCAL: {
                i++;
                if (vm->stack[vm->func->code.data[i]].type == Value_String && vm->stack[vm->func->code.data[i]].mutable) free(vm->stack[vm->func->code.data[i]].val.str);
                vm->stack[vm->func->code.data[i]] = stack_pop;
                break;}
            case OP_POP:
                vm->stack_top -= read_index;
                i += 2;
                break;
            case OP_LEN:;
                Value array = stack_pop;
                ASSERT(array.type == Value_Array, "ERROR in %s on line %ld: cant do len of %s\n", get_loc, find_value_type(array.type))
                stack_push(((Value) {
                    Value_Number,      
                    .val.num=vm->func->arrays.data[array.val.num][0].val.num - 1,   
                    false,            
                    0                 
                }));                 
                break;
            case OP_CAST_STR:{
                Value val = stack_pop;
                if (val.type == Value_String) stack_push(val);
                else if (val.type == Value_Number) stack_push(((Value) {
                    Value_String, 
                    .val.str=format_str(num_len(val.val.num) + 1, "%ld", val.val.num),
                    true,
                    0
                }));
                else ERR("ERROR in %s on line %ld: cant cast type %s as string\n", get_loc, find_value_type(val.type))
                break;}
            case OP_CAST_NUM:{
                Value val = stack_pop;
                if (val.type == Value_Number) stack_push(val);
                else if (val.type == Value_String) stack_push(((Value) {
                    Value_Number,
                    .val.num=strtoint(val.val.str, strlen(val.val.str)),
                    false,
                    0
                }));
                else ERR("ERROR in %s on line %ld: cant cast type %s as number\n", get_loc, find_value_type(val.type))
                break;}
            case OP_CALL:{
                Call_Frame *frame = &vm->call_stack[vm->call_stack_count++];
                frame->func = &vm->funcs.data[read_index];
                frame->slots = vm->stack_top - 1;
                vm->func = &vm->funcs.data[read_index];
                i = -1;
                break;}
            default: ERR("ERROR in %s on line %ld: cant do %s\n", get_loc, find_op_code(vm->func->code.data[i]))
        }
    }
}
