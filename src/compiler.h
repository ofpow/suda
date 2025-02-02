#pragma once

bool ast_value_equal(AST_Value *val1, AST_Value *val2) {
    if (val1->type == Value_Number && val2->type == Value_Number) {
        return (NUM(val1->value) == NUM(val2->value));
    } else if (val1->type == Value_String && val2->type == Value_String) {
        return (!strcmp(STR(val1->value), STR(val2->value)));
    } else if (val1->type == Value_Identifier && val2->type == Value_Identifier) {
        return (!strcmp(STR(val1->value), STR(val2->value)));
    } else return false;
}

int exponentiate(int base, int64_t power) {
    int64_t result = 1;

    for (int i = 0; i < power; i++) {
        result *= base;
    }

    return result;
}

#define STACK_SIZE 16384
#define LOCALS_MAX 256
#define CALL_STACK_SIZE 64

#define COMBYTE(_byte1, _byte2) (((_byte1) << 8) | (_byte2))
#define FIRST_BYTE(_val) (u_int8_t)((_val) >> 8)
#define SECOND_BYTE(_val) (u_int8_t)((_val) & 0xFF)
#define INVALID_LOC ((Location){ "INVALID LOCATION", -1 })
#define current_loc(_node) ((Location){ _node->file, _node->line })
#define append_code(_code, _loc) append(c->func.code, _code); append(c->func.locs, _loc)
#define make_loc(_loc) (Location){_loc}
#define get_loc vm->func->locs.data[pc].file, vm->func->locs.data[pc].line


#define ops \
    X(OP_CONSTANT)\
    X(OP_ARRAY)\
    X(OP_PRINTLN)\
    X(OP_PRINT)\
    X(OP_DEFINE_GLOBAL)\
    X(OP_SET_GLOBAL)\
    X(OP_GET_GLOBAL)\
    X(OP_SET_LOCAL)\
    X(OP_GET_LOCAL)\
    X(OP_ADD)\
    X(OP_SUBTRACT)\
    X(OP_MULTIPLY)\
    X(OP_DIVIDE)\
    X(OP_MODULO)\
    X(OP_BIT_AND)\
    X(OP_BIT_OR)\
    X(OP_BIT_XOR)\
    X(OP_BIT_NOT)\
    X(OP_LSHIFT)\
    X(OP_RSHIFT)\
    X(OP_POWER)\
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
    X(OP_START_IF)\
    X(OP_JUMP)\
    X(OP_GET_ELEMENT)\
    X(OP_SET_ELEMENT_GLOBAL)\
    X(OP_SET_ELEMENT_LOCAL)\
    X(OP_APPEND_GLOBAL)\
    X(OP_APPEND_LOCAL)\
    X(OP_POP)\
    X(OP_CAST_STR)\
    X(OP_CAST_NUM)\
    X(OP_CALL)\
    X(OP_CALL_NATIVE)\
    X(OP_RETURN)\
    X(OP_RETURN_NOTHING)\
    X(OP_BREAK)\
    X(OP_CONTINUE)\
    X(OP_FOR)\
    X(OP_ENUMERATE)\
    X(OP_GET_GLOBAL_GET_CONSTANT)\
    X(OP_GET_LOCAL_GET_CONSTANT)\
    X(OP_DONE)\

#ifdef PROFILE
#define X(x) 0,
u_int64_t op_profiler[] = {
    ops
};
#undef X
#endif

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
        case AST_Modulo: return OP_MODULO;
        case AST_Bit_And: return OP_BIT_AND;
        case AST_Bit_Or: return OP_BIT_OR;
        case AST_Bit_Xor: return OP_BIT_XOR;
        case AST_Bit_Not: return OP_BIT_NOT;
        case AST_Lshift: return OP_LSHIFT;
        case AST_Rshift: return OP_RSHIFT;
        case AST_Power: return OP_POWER;
        default: ERR("no op code for ast type %d\n", type);
    }
    return 0;
}

u_int32_t next_power_of_two(u_int32_t num) {
    u_int32_t result = 1;
    for (; result < num; result *= 2);
    return result;
}

define_array(Code, u_int8_t);
define_array(Constants, Value);
define_array(Loop_Indices, Value);

typedef struct Function {
    Code code;
    Locations locs;

    Constants constants;

    int arity;
    char *name;
} Function;

define_array(Functions, Function);

typedef struct Local {
    AST_Value *name;
    int depth;
} Local;

typedef struct Compiler {
    Jump_Indices if_indices;
    Jump_Indices while_indices;
    Jump_Indices for_indices;

    Function func;

    Local locals[LOCALS_MAX];
    u_int8_t locals_count;
    int64_t depth;
} Compiler;

void add_local(Node *n, Compiler *c) {
    if (c->locals_count >= LOCALS_MAX - 1) ERR("ERROR in %s on line %ld: too many local vars\n", n->file, n->line)
    
    for (int i = c->locals_count - 1; i >= 0; i--) {
        if (c->locals[i].depth != -1 && c->locals[i].depth < c->depth) break;

        if (ast_value_equal(c->locals[i].name, n->value)) 
            ERR("ERROR in %s on line %ld: cant redefine local variable %s\n", n->file, n->line, STR(n->value->value))
    }

    c->locals[c->locals_count].name = n->value;
    c->locals[c->locals_count].depth = c->depth;
    c->locals_count++;
}

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

void free_func(Function func) {
    free(func.code.data);
    free(func.locs.data);
    for (int i = 0; i < func.constants.index; i++) {
        if (func.constants.data[i].type == Value_Array) {
            free_value_array(func.constants.data[i].val.array);
        }
    }
    free(func.constants.data);
}

void compile_identifier(Node *n, Compiler *c) {
    if (is_local(n->value, c)) {
        append_code(OP_GET_LOCAL, current_loc(n));
        append_code(resolve_local(n->value, c), INVALID_LOC);
        return;
    } else {
        append(c->func.constants, ((Value){n->value->type, .val.str={n->value->value, strlen(n->value->value)}, false, n->value->hash}));
    }
    u_int16_t index = c->func.constants.index - 1;
    append_code(OP_GET_GLOBAL, current_loc(n));
    append_code(FIRST_BYTE(index), INVALID_LOC);
    append_code(SECOND_BYTE(index), INVALID_LOC);
}

void compile_constant(Node *n, Compiler *c) {
    if (n->value->type == Value_Number)
        append(c->func.constants, ((Value){n->value->type, .val.num=NUM(n->value->value), false, 0}));
    else if (n->value->type == Value_String)
        append(c->func.constants, ((Value){n->value->type, .val.str={n->value->value, strlen(n->value->value)}, false, 0}));
    else if (n->value->type == Value_Identifier)
        append(c->func.constants, ((Value){n->value->type, .val.str={n->value->value, strlen(n->value->value)}, false, n->value->hash}));
    else
        ERR("ERROR in %s on line %ld: cant compile constant of type %d\n", n->file, n->line, n->value->type)

    u_int16_t index = c->func.constants.index - 1;
    append_code(OP_CONSTANT, current_loc(n));
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
                append_code(OP_GET_LOCAL, current_loc(n));
                append_code(resolve_local(n->value, c), INVALID_LOC);
            } else {
                append(c->func.constants, ((Value){Value_Identifier, .val.str={n->value->value, strlen(n->value->value)}, false, n->value->hash})); // name
                u_int16_t index = c->func.constants.index - 1;
                append_code(OP_GET_GLOBAL, current_loc(n));
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
        case AST_Modulo:
        case AST_Bit_And:
        case AST_Bit_Or:
        case AST_Bit_Xor:
        case AST_Lshift:
        case AST_Rshift:
        case AST_Power:
            if (n->left->type == AST_Identifier && n->right->type == AST_Literal) {
                //compile variable
                if (is_local(n->left->value, c)) {
                    append_code(OP_GET_LOCAL_GET_CONSTANT, current_loc(n));
                    append_code(resolve_local(n->left->value, c), INVALID_LOC);
                } else {
                    append(c->func.constants, ((Value){Value_Identifier, .val.str={n->left->value->value, strlen(n->left->value->value)}, false, n->left->value->hash})); // name
                    u_int16_t index = c->func.constants.index - 1;
                    append_code(OP_GET_GLOBAL_GET_CONSTANT, current_loc(n));
                    append_code(FIRST_BYTE(index), INVALID_LOC);
                    append_code(SECOND_BYTE(index), INVALID_LOC);
                }

                compile_constant(n->right, c);

                u_int16_t index = COMBYTE(c->func.code.data[c->func.code.index - 2], c->func.code.data[c->func.code.index - 1]);
                c->func.code.index -= 3;
                append_code(FIRST_BYTE(index), INVALID_LOC);
                append_code(SECOND_BYTE(index), INVALID_LOC);

                append_code(ast_to_op_code(n->type), current_loc(n));
            } else {
                compile_expr(n->left, c);
                compile_expr(n->right, c);
                append_code(ast_to_op_code(n->type), current_loc(n));
            }
            break;
        case AST_Not:
            compile_expr(n->left, c);
            append_code(OP_NOT, current_loc(n));
            break;
        case AST_Bit_Not:
            compile_expr(n->left, c);
            append_code(OP_BIT_NOT, current_loc(n));
            break;
        case AST_Array:{
            u_int32_t array_len = NUM(n->value[0].value);
            u_int32_t array_size = next_power_of_two(array_len);
            Value *array = calloc(array_size, sizeof(Value));
            for (u_int32_t i = 0; i < array_len; i++) {
                switch (n->value[i].type) {
                    case Value_Array:
                        array[i].type = Value_Array;

                        // first 4 bytes: allocated size
                        // last 4 bytes: number of elements including header info
                        array[i].val.num = MAKE_ARRAY_INFO(array_size, array_len);
                        break;
                    case Value_Number:
                        array[i].type = Value_Number;
                        array[i].val.num = NUM(n->value[i].value);
                        break;
                    case Value_String:
                        array[i].type = Value_String;
                        size_t len = strlen(n->value[i].value);
                        char *s = format_str(len, "%.*s", len - 2, STR(n->value[i].value) + 1);
                        array[i].val.str = (string){s, len - 2};
                        array[i].mutable = true;
                        free(n->value[i].value);
                        n->value[i].value = NULL;
                        break;
                    case Value_Identifier:
                        array[i].type = Value_Identifier;
                        array[i].val.str.chars = STR(n->value[i].value);
                        array[i].mutable = true;
                        array[i].hash = n->value[i].hash;
                        n->value[i].value = NULL;
                        break;
                    default: ERR("ERROR in %s on line %ld: cant do array type %s\n", n->file, n->line, find_value_type(n->value[i].type))
                }
            }
            Value val = {
                Value_Array,
                .val.array=array,
                false,
                0
            };
            append(c->func.constants, val);

            u_int16_t index = c->func.constants.index - 1;
            append_code(OP_ARRAY, current_loc(n));
            append_code(FIRST_BYTE(index), INVALID_LOC);
            append_code(SECOND_BYTE(index), INVALID_LOC);
            break;}
        case AST_At:{
            if (n->right != NULL) {
                compile_expr(n->left, c); // index
                compile_expr(n->right, c); // parent at
                append_code(OP_GET_ELEMENT, current_loc(n));
            } else {
                compile_expr(n->left, c); // the index
                compile_identifier(n, c);
                append_code(OP_GET_ELEMENT, current_loc(n));
            }
            break;}
        case AST_Cast_Str:
            compile_expr(n->left, c);
            append_code(OP_CAST_STR, current_loc(n));
            break;
        case AST_Cast_Num:
            compile_expr(n->left, c);
            append_code(OP_CAST_NUM, current_loc(n));
            break;
        case AST_Fn_Call:{
            for (int j = 0; j < n->func_args_index; j++) {
                compile_expr(n->func_args[j], c);
            }

            int index = is_native(n->value->value);
            if (index > -1) {
                ASSERT(native_arities[index] == n->func_args_index, "ERROR in %s on line %ld: cant call native function %s with %ld args, it needs %d\n", n->file, n->line, STR(n->value->value), n->func_args_index, native_arities[index])

                append_code(OP_CALL_NATIVE, current_loc(n));
                append_code(FIRST_BYTE(index), INVALID_LOC);
                append_code(SECOND_BYTE(index), INVALID_LOC);
            } else {

                u_int16_t index = resolve_func(n->value);

                ASSERT(p->funcs.data[index - 1]->arity == n->func_args_index, "ERROR in %s on line %ld: cant call function %s with %ld args, it needs %ld\n", n->file, n->line, STR(n->value->value), n->func_args_index, p->funcs.data[index - 1]->arity)

                append_code(OP_CALL, current_loc(n));
                append_code(FIRST_BYTE(index), INVALID_LOC);
                append_code(SECOND_BYTE(index), INVALID_LOC);
            }
            break;}
        default: ERR("ERROR in %s on line %ld: cant compile node type %s as expr\n", n->file, n->line, find_ast_type(n->type));
    }
}

void compile(Node **nodes, int64_t nodes_size, Compiler *c) {
    for (int i = 0; i < nodes_size; i++) {
        switch (nodes[i]->type) {
            case AST_Println:
                compile_expr(nodes[i]->left, c);
                append_code(OP_PRINTLN, current_loc(nodes[i]));
                break;
            case AST_Print:
                compile_expr(nodes[i]->left, c);
                append_code(OP_PRINT, current_loc(nodes[i]));
                break;
            case AST_Var_Assign:
                compile_expr(nodes[i]->left, c); // value
                
                if (c->depth > 0) {
                    add_local(nodes[i], c);
                } else {
                    append(c->func.constants, ((Value){Value_Identifier, .val.str={nodes[i]->value->value, strlen(nodes[i]->value->value)}, false, nodes[i]->value->hash})); // name
                    u_int16_t index = c->func.constants.index - 1;
                    append_code(OP_DEFINE_GLOBAL, current_loc(nodes[i]));
                    append_code(FIRST_BYTE(index), INVALID_LOC);
                    append_code(SECOND_BYTE(index), INVALID_LOC);
                }
                break;
            case AST_Identifier:
                compile_expr(nodes[i]->left, c); // value
                
                if (is_local(nodes[i]->value, c)) {
                    append_code(OP_SET_LOCAL, current_loc(nodes[i]));
                    append_code(resolve_local(nodes[i]->value, c), INVALID_LOC);
                } else {
                    append(c->func.constants, ((Value){Value_Identifier, .val.str={nodes[i]->value->value, strlen(nodes[i]->value->value)}, false, nodes[i]->value->hash})); // name
                    u_int16_t index = c->func.constants.index - 1;
                    append_code(OP_SET_GLOBAL, current_loc(nodes[i]));
                    append_code(FIRST_BYTE(index), INVALID_LOC);
                    append_code(SECOND_BYTE(index), INVALID_LOC);
                }
                break;
            case AST_If:
                compile_expr(nodes[i]->left, c);

                append(c->if_indices, c->func.code.index);
                append_code(OP_START_IF, current_loc(nodes[i]));
                append_code(0, INVALID_LOC);
                append_code(0, INVALID_LOC);

                c->depth++;
                break;
            case AST_While:
                append(c->while_indices, c->func.code.index);
                compile_expr(nodes[i]->left, c);

                append(c->while_indices, c->func.code.index);
                append_code(OP_JUMP_IF_FALSE, current_loc(nodes[i]));
                append_code(0, INVALID_LOC);
                append_code(0, INVALID_LOC);
                c->depth++;
                break;
            case AST_Semicolon:;
                c->depth--;

                u_int16_t amount = 0;
                while(c->locals_count > 0 && c->locals[c->locals_count - 1].depth > c->depth) { amount++; c->locals_count--; }

                if (amount > 0 && (nodes[nodes[i]->jump_index]->type != AST_For && nodes[nodes[i]->jump_index]->type != AST_Enumerate)) {
                    append_code(OP_POP, current_loc(nodes[i]));
                    append_code(FIRST_BYTE(amount), INVALID_LOC);
                    append_code(SECOND_BYTE(amount), INVALID_LOC);
                }

                if (nodes[nodes[i]->jump_index]->type == AST_While) {
                    u_int16_t start_index = 0;
                    for (int i = c->while_indices.index; i; i--) {
                        if (c->func.code.data[c->while_indices.data[i]] == OP_JUMP_IF_FALSE) {
                            start_index = c->while_indices.data[i - 1];
                            break;
                        }
                    }

                    while (1) {
                        u_int16_t index = c->while_indices.data[--c->while_indices.index];
                        if (c->func.code.data[index] == OP_JUMP_IF_FALSE) {
                            u_int16_t x = c->func.code.index + 3 - index;
                            c->func.code.data[index + 1] = FIRST_BYTE(x);
                            c->func.code.data[index + 2] = SECOND_BYTE(x);
                            break;
                        } else if (c->func.code.data[index] == OP_BREAK) {
                            u_int16_t x = c->func.code.index + 3 - index;
                            c->func.code.data[index + 1] = FIRST_BYTE(x);
                            c->func.code.data[index + 2] = SECOND_BYTE(x);
                        } else if (c->func.code.data[index] == OP_CONTINUE) {
                            c->func.code.data[index + 1] = FIRST_BYTE(start_index);
                            c->func.code.data[index + 2] = SECOND_BYTE(start_index);
                        }

                    }

                    u_int16_t index = c->while_indices.data[--c->while_indices.index];
                    append_code(OP_JUMP, current_loc(nodes[i]));
                    append_code(FIRST_BYTE(index), INVALID_LOC);
                    append_code(SECOND_BYTE(index), INVALID_LOC);
                } else if (nodes[nodes[i]->jump_index]->type == AST_For || nodes[nodes[i]->jump_index]->type == AST_Enumerate) {
                    u_int16_t index = c->for_indices.data[--c->for_indices.index];
                    u_int16_t offset = c->func.code.index;
                    
                    c->func.code.data[index] = FIRST_BYTE(offset);
                    c->func.code.data[index + 1] = SECOND_BYTE(offset);

                    index = c->for_indices.data[--c->for_indices.index];
                    append_code(OP_JUMP, current_loc(nodes[i]));
                    append_code(FIRST_BYTE(index), INVALID_LOC);
                    append_code(SECOND_BYTE(index), INVALID_LOC);

                    if (nodes[nodes[i]->jump_index]->type == AST_For)
                        index = 1;
                    else if (nodes[nodes[i]->jump_index]->type == AST_Enumerate)
                        index = 2;

                    append_code(OP_POP, current_loc(nodes[i]));
                    append_code(FIRST_BYTE(index), INVALID_LOC);
                    append_code(SECOND_BYTE(index), INVALID_LOC);

                    if (nodes[nodes[i]->jump_index]->type == AST_For)
                        c->locals_count--;
                    else if (nodes[nodes[i]->jump_index]->type == AST_Enumerate)
                        c->locals_count -= 2;
                } else if (nodes[nodes[i]->jump_index]->type == AST_If) {
                    u_int16_t index = c->if_indices.data[--c->if_indices.index];

                    u_int16_t offset = c->func.code.index - index;
                    c->func.code.data[index + 1] = FIRST_BYTE(offset);
                    c->func.code.data[index + 2] = SECOND_BYTE(offset);
                } else {
                    while (1) {
                        if (c->if_indices.index < 1) break;
                        u_int16_t index = c->if_indices.data[--c->if_indices.index];
                        if (c->func.code.data[index] == OP_START_IF) {
                            c->if_indices.index++; 
                            break;
                        } else if (c->func.code.data[index] == OP_JUMP_IF_FALSE) {
                            if (COMBYTE(c->func.code.data[index + 1], c->func.code.data[index + 2]) != 0) continue;
                            u_int16_t i = c->func.code.index - index;
                            c->func.code.data[index + 1] = FIRST_BYTE(i);
                            c->func.code.data[index + 2] = SECOND_BYTE(i);
                        } else {
                            c->func.code.data[index + 1] = FIRST_BYTE(c->func.code.index);
                            c->func.code.data[index + 2] = SECOND_BYTE(c->func.code.index);
                        }
                    }
                }
                break;
            case AST_Else:;
                u_int16_t index = c->if_indices.data[--c->if_indices.index];
                u_int16_t offset = c->func.code.index - index + 3;
                c->func.code.data[index + 1] = FIRST_BYTE(offset);
                c->func.code.data[index + 2] = SECOND_BYTE(offset);
                append(c->if_indices, c->func.code.index);
                append_code(OP_JUMP, current_loc(nodes[i]));
                append_code(0, INVALID_LOC);
                append_code(0, INVALID_LOC);
                break;
            case AST_Elif:{
                u_int16_t index = c->if_indices.data[--c->if_indices.index];
                u_int16_t offset = c->func.code.index - index + 3;
                c->func.code.data[index + 1] = FIRST_BYTE(offset);
                c->func.code.data[index + 2] = SECOND_BYTE(offset);

                append(c->if_indices, c->func.code.index);
                append_code(OP_JUMP, current_loc(nodes[i]));
                append_code(0, INVALID_LOC);
                append_code(0, INVALID_LOC);
                
                compile_expr(nodes[i]->left, c);

                append(c->if_indices, c->func.code.index);
                append_code(OP_JUMP_IF_FALSE, current_loc(nodes[i]));
                append_code(0, INVALID_LOC);
                append_code(0, INVALID_LOC);
                break;}
            case AST_At:
            case AST_At_Assign:
                compile_expr(nodes[i]->left, c); // index
                if (nodes[i]->right) {// new value
                    compile_expr(nodes[i]->right, c); 
                } else {
                    compile_expr(nodes[i]->left->left, c);
                }

                if (is_local(nodes[i]->value, c)) {
                    append_code(OP_SET_ELEMENT_LOCAL, current_loc(nodes[i]));
                    append_code(resolve_local(nodes[i]->value, c), INVALID_LOC);
                } else {
                    append(c->func.constants, ((Value){Value_Identifier, .val.str={nodes[i]->value->value, strlen(nodes[i]->value->value)}, false, nodes[i]->value->hash}));
                    u_int16_t index = c->func.constants.index - 1;
                    append_code(OP_SET_ELEMENT_GLOBAL, current_loc(nodes[i]));
                    append_code(FIRST_BYTE(index), INVALID_LOC);
                    append_code(SECOND_BYTE(index), INVALID_LOC);
                }
                break;
            case AST_Fn_Call:{
                for (int j = 0; j < nodes[i]->func_args_index; j++) {
                    compile_expr(nodes[i]->func_args[j], c);
                }

                int index = is_native(nodes[i]->value->value);
                if (index > -1) {

                    ASSERT(native_arities[index - 1] == nodes[i]->func_args_index, "ERROR in %s on line %ld: cant call native function %s with %ld args, it needs %d\n", nodes[i]->file, nodes[i]->line, STR(nodes[i]->value->value), nodes[i]->func_args_index, native_arities[index - 1])
                    append_code(OP_CALL_NATIVE, current_loc(nodes[i]));
                    append_code(FIRST_BYTE(index), INVALID_LOC);
                    append_code(SECOND_BYTE(index), INVALID_LOC);
                } else {

                    u_int16_t index = resolve_func(nodes[i]->value);

                    ASSERT(p->funcs.data[index - 1]->arity == nodes[i]->func_args_index, "ERROR in %s on line %ld: cant call function %s with %ld args, it needs %ld\n", nodes[i]->file, nodes[i]->line, STR(nodes[i]->value->value), nodes[i]->func_args_index, p->funcs.data[index - 1]->arity)

                    append_code(OP_CALL, current_loc(nodes[i]));
                    append_code(FIRST_BYTE(index), INVALID_LOC);
                    append_code(SECOND_BYTE(index), INVALID_LOC);
                }
                break;}
            case AST_Return:{
                compile_expr(nodes[i]->left, c);
                append_code(OP_RETURN, current_loc(nodes[i]));
                break;}
            case AST_Append:{
                compile_identifier(nodes[i], c);
                compile_expr(nodes[i]->left, c);

                if (is_local(nodes[i]->value, c)) {
                    append_code(OP_APPEND_LOCAL, current_loc(nodes[i]));
                    append_code(resolve_local(nodes[i]->value, c), INVALID_LOC);
                } else {
                    append(c->func.constants, ((Value){Value_Identifier, .val.str={nodes[i]->value->value, strlen(nodes[i]->value->value)}, false, nodes[i]->value->hash}));
                    u_int16_t index = c->func.constants.index - 1;
                    append_code(OP_APPEND_GLOBAL, current_loc(nodes[i]));
                    append_code(FIRST_BYTE(index), INVALID_LOC);
                    append_code(SECOND_BYTE(index), INVALID_LOC);
                }

                break;}
            case AST_Break:{
                append(c->while_indices, c->func.code.index);
                append_code(OP_BREAK, current_loc(nodes[i]));
                append_code(0, INVALID_LOC);
                append_code(0, INVALID_LOC);
                break;}
            case AST_Continue:{
                append(c->while_indices, c->func.code.index);
                append_code(OP_CONTINUE, current_loc(nodes[i]));
                append_code(0, INVALID_LOC);
                append_code(0, INVALID_LOC);
                break;}
            case AST_For:{
                append(c->for_indices, c->func.code.index);
                compile_expr(nodes[i]->right, c);
                add_local(nodes[i]->left, c);
                
                append(c->func.constants, ((Value){Value_Number, .val.num=1, false, 0}));
                u_int16_t index = c->func.constants.index - 1;
                
                append_code(OP_FOR, current_loc(nodes[i]));
                append_code(FIRST_BYTE(index), INVALID_LOC);
                append_code(SECOND_BYTE(index), INVALID_LOC);
                append_code(resolve_local(nodes[i]->left->value, c), INVALID_LOC);

                append(c->for_indices, c->func.code.index);
                append_code(0, INVALID_LOC);
                append_code(0, INVALID_LOC);
                c->depth++;
                break;}
            case AST_Enumerate:{
                append(c->for_indices, c->func.code.index);
                compile_expr(nodes[i]->right, c); // array
                add_local(nodes[i]->left->left, c); // index
                add_local(nodes[i]->left->right, c); // value
    
                append(c->func.constants, ((Value){Value_Number, .val.num=1, false, 0}));
                u_int16_t index = c->func.constants.index - 1;
                
                append_code(OP_ENUMERATE, current_loc(nodes[i]));
                append_code(FIRST_BYTE(index), INVALID_LOC);
                append_code(SECOND_BYTE(index), INVALID_LOC);
                append_code(resolve_local(nodes[i]->left->left->value, c), INVALID_LOC);
                append_code(resolve_local(nodes[i]->left->right->value, c), INVALID_LOC);

                append(c->for_indices, c->func.code.index);
                append_code(0, INVALID_LOC);
                append_code(0, INVALID_LOC);
                c->depth++;
                break;}
            default: ERR("ERROR in %s on line %ld: cant compile node type %s\n", nodes[i]->file, nodes[i]->line, find_ast_type(nodes[i]->type))
        }
    }
}

Function compile_func(AST_Function *func){
    Compiler c = {0};
    if (func->name != NULL) c.depth++;
    for (int i = 0; i < func->arity; i++) {
        c.locals[i].name = func->args[i];
        c.locals[i].depth = 1;
        c.locals_count++;
    }
    c.if_indices = (Jump_Indices){calloc(10, sizeof(int64_t)), 0, 10};
    c.while_indices = (Jump_Indices){calloc(10, sizeof(int64_t)), 0, 10};
    c.for_indices = (Jump_Indices){calloc(10, sizeof(int64_t)), 0, 10};
    c.func.code = (Code){calloc(10, sizeof(u_int8_t)), 0, 10};
    c.func.constants = (Constants){calloc(10, sizeof(Value)), 0, 10};
    c.func.locs = (Locations){calloc(10, sizeof(Location)), 0, 10};

    compile(func->nodes.data, func->nodes.index, &c);

    if (func->name != NULL && c.func.code.index > 0 && c.func.code.data[c.func.code.index - 1] != OP_RETURN) {
        append(c.func.code, OP_RETURN_NOTHING);
    }

    c.func.name = func->name;
    c.func.arity = func->arity;

    free(c.if_indices.data);
    free(c.while_indices.data);
    free(c.for_indices.data);
    return c.func;
}
