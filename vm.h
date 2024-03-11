#pragma once

#define STACK_SIZE 16384

#define FIRST_BYTE(_val) (u_int8_t)((_val) >> 8)
#define SECOND_BYTE(_val) (u_int8_t)((_val) & 0xFF)
#define COMBYTE(_byte1, _byte2) (((_byte1) << 8) | (_byte2))
#define stack_pop (*--vm->stack_top)
#define stack_push(_val) do { \
    *vm->stack_top = (_val);  \
    vm->stack_top++;          \
} while (0)                   \

#define binary_op(op, msg) do {                                                                                                    \
    Value op2 = stack_pop;                                                                                                         \
    Value op1 = stack_pop;                                                                                                         \
    if (op1.type == Value_Identifier) op1 = ((Variable*)get_entry(vm->vars->entries, vm->vars->capacity, op1.hash)->value)->value; \
    if (op2.type == Value_Identifier) op2 = ((Variable*)get_entry(vm->vars->entries, vm->vars->capacity, op2.hash)->value)->value; \
    if (op1.type == Value_Number && op2.type == Value_Number) {                                                                    \
        stack_push(((Value) {                                                                                                      \
            Value_Number,                                                                                                          \
            .val.num=(op1.val.num op op2.val.num),                                                                                 \
            false,                                                                                                                 \
            0                                                                                                                      \
        }));                                                                                                                       \
    } else ERR(msg, find_value_type(op1.type), find_value_type(op2.type))                                                          \
} while (0);                                                                                                                       \
break                                                                                                                              \

#define ops \
    X(OP_CONSTANT)\
    X(OP_PRINTLN)\
    X(OP_DEFINE_VARIABLE)\
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
    X(OP_SET_VARIABLE)\
    X(OP_JUMP_IF_FALSE)\
    X(OP_JUMP)\
    X(OP_ARRAY)\
    X(OP_GET_ELEMENT)\
    X(OP_SET_ELEMENT)\
    X(OP_START_IF)\

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
        case AST_Var_Assign: return OP_DEFINE_VARIABLE;
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

typedef struct Compiler {
    Jump_Indices if_indices;
    Jump_Indices while_indices;

    Code code;

    Constants constants;

    Arrays arrays;
} Compiler;

typedef struct VM {
    Code code;

    Constants constants;

    Arrays arrays;

    Map *vars;

    Value stack[STACK_SIZE];
    Value *stack_top;
} VM;

void print_array(VM *vm, Value *val) {
    Value *array = vm->arrays.data[val->val.num];
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
    for (int i = 0; i < vm->code.index; i++) {
        switch(vm->code.data[i]) {
            case OP_CONSTANT:
                printf("%-6d OP_CONSTANT:      index %d\n", i, COMBYTE(vm->code.data[i + 1], vm->code.data[i + 2]));
                i += 2;
                break;
            case OP_ARRAY:
                printf("%-6d OP_ARRAY:         index %d\n", i, COMBYTE(vm->code.data[i + 1], vm->code.data[i + 2]));
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
            case OP_DEFINE_VARIABLE:
                printf("%-6d OP_DEFINE_VARIABLE\n", i);
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
            case OP_SET_VARIABLE:
                printf("%-6d OP_SET_VARIABLE \n", i);
                break;
            case OP_JUMP_IF_FALSE:
                printf("%-6d OP_JUMP_IF_FALSE: offset %d\n", i, COMBYTE(vm->code.data[i + 1], vm->code.data[i + 2]));
                i += 2;
                break;
            case OP_START_IF:
                printf("%-6d OP_START_IF:      offset %d\n", i, COMBYTE(vm->code.data[i + 1], vm->code.data[i + 2]));
                i += 2;
                break;
            case OP_JUMP:
                printf("%-6d OP_JUMP:          index %d\n", i, COMBYTE(vm->code.data[i + 1], vm->code.data[i + 2]));
                i += 2;
                break;
            default:
                ERR("cant disassemble op type %d\n", vm->code.data[i]);
        }
    }
}

void compile_constant(AST_Value *value, Compiler *c) {
    if (value->type == Value_Number)
        append_new(c->constants, ((Value){value->type, .val.num=NUM(value->value), false, 0}));
    else if (value->type == Value_String)
        append_new(c->constants, ((Value){value->type, .val.str=value->value, false, 0}));
    else if (value->type == Value_Identifier)
        append_new(c->constants, ((Value){value->type, .val.str=value->value, false, value->hash}));
    else
        ERR("cant compile constant of type %d\n", value->type)

    u_int16_t index = c->constants.index - 1;
    append_new(c->code, OP_CONSTANT);
    append_new(c->code, FIRST_BYTE(index));
    append_new(c->code, SECOND_BYTE(index));
}

void compile_expr(Node *n, Compiler *c) {
    switch (n->type) {
        case AST_Literal:
            compile_constant(n->value, c);
            break;
        case AST_Identifier:
            compile_constant(n->value, c);
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
            append_new(c->code, ast_to_op_code(n->type));
            break;
        case AST_Not:
            compile_expr(n->left, c);
            append_new(c->code, OP_NOT);
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
                    default: ERR("cant do array type %s\n", find_value_type(n->value[i].type))
                }
            }

            append_new(c->arrays, array);
            append_new(c->code, OP_ARRAY);
            u_int16_t index = c->arrays.index - 1;
            append_new(c->code, FIRST_BYTE(index));
            append_new(c->code, SECOND_BYTE(index));
            break;}
        case AST_At:{
            compile_expr(n->left, c); // the index
            compile_constant(n->value, c); //var name
            append_new(c->code, OP_GET_ELEMENT);
            break;}
        default: ERR("cant handle node type %s\n", find_ast_type(n->type));
    }
}

void compile(Node **nodes, int64_t nodes_size, Compiler *c) {
    for (int i = 0; i < nodes_size; i++) {
        switch (nodes[i]->type) {
            case AST_Println:
                compile_expr(nodes[i]->left, c);
                append_new(c->code, OP_PRINTLN);
                break;
            case AST_Var_Assign:
                compile_constant(nodes[i]->value, c); // name
                compile_expr(nodes[i]->left, c); // value
                append_new(c->code, OP_DEFINE_VARIABLE);
                break;
            case AST_Identifier:
                compile_constant(nodes[i]->value, c); // name
                compile_expr(nodes[i]->left, c); // value
                append_new(c->code, OP_SET_VARIABLE);
                break;
            case AST_If:
                compile_expr(nodes[i]->left, c);

                append_new(c->if_indices, c->code.index);
                append_new(c->if_indices, c->code.index);
                append_new(c->code, OP_START_IF);
                append_new(c->code, 0);
                append_new(c->code, 0);
                break;
            case AST_While:
                append_new(c->while_indices, c->code.index);
                compile_expr(nodes[i]->left, c);

                append_new(c->while_indices, c->code.index);
                append_new(c->code, OP_JUMP_IF_FALSE);
                append_new(c->code, 0);
                append_new(c->code, 0);
                break;
            case AST_Semicolon:;
                if (nodes[nodes[i]->jump_index]->type == AST_While) {
                    u_int16_t index = c->while_indices.data[--c->while_indices.index];
                    u_int16_t x = c->code.index + 3 - index;
                    c->code.data[index + 1] = FIRST_BYTE(x);
                    c->code.data[index + 2] = SECOND_BYTE(x);

                    index = c->while_indices.data[--c->while_indices.index];
                    append_new(c->code, OP_JUMP);
                    append_new(c->code, FIRST_BYTE(index));
                    append_new(c->code, SECOND_BYTE(index));
                } else {
                    while (1) {
                        u_int16_t index = c->if_indices.data[--c->if_indices.index];
                        if (c->code.data[index] == OP_START_IF) break;
                        u_int16_t offset = c->code.index - index;
                        if (c->code.data[index] == OP_JUMP) offset += index;
                        c->code.data[index + 1] = FIRST_BYTE(offset);
                        c->code.data[index + 2] = SECOND_BYTE(offset);
                    }
                }
                break;
            case AST_Else:;
                u_int16_t index = c->if_indices.data[--c->if_indices.index];
                u_int16_t offset = c->code.index - index + 3;
                c->code.data[index + 1] = FIRST_BYTE(offset);
                c->code.data[index + 2] = SECOND_BYTE(offset);
                append_new(c->if_indices, c->code.index);
                append_new(c->code, OP_JUMP);
                append_new(c->code, 0);
                append_new(c->code, 0);
                break;
            case AST_Elif:{
                u_int16_t index = c->if_indices.data[--c->if_indices.index];
                u_int16_t offset = c->code.index - index + 3;
                c->code.data[index + 1] = FIRST_BYTE(offset);
                c->code.data[index + 2] = SECOND_BYTE(offset);

                append_new(c->if_indices, c->code.index);
                append_new(c->code, OP_JUMP);
                append_new(c->code, 0);
                append_new(c->code, 0);
                
                compile_expr(nodes[i]->left, c);

                append_new(c->if_indices, c->code.index);
                append_new(c->code, OP_JUMP_IF_FALSE);
                append_new(c->code, 0);
                append_new(c->code, 0);
                break;}
            case AST_At:
                compile_constant(nodes[i]->value, c); // var name
                compile_expr(nodes[i]->left, c); // index
                compile_expr(nodes[i]->right, c); // new value
                append_new(c->code, OP_SET_ELEMENT);
                break;
            default: ERR("cant compile node type %s\n", find_ast_type(nodes[i]->type))
        }
    }
}

void run(VM *vm) {
    for (int i = 0; i < vm->code.index; i++) {
        debug("%-6d %s\n", i, find_op_code(vm->code.data[i]));
        switch (vm->code.data[i]) {
            case OP_CONSTANT:;
                u_int16_t index = COMBYTE(vm->code.data[i + 1], vm->code.data[i + 2]);
                stack_push(vm->constants.data[index]);
                i += 2;
                break;
            case OP_PRINTLN:;
                Value print = stack_pop;
                if (print.type == Value_Number) {
                    printf("%ld\n", print.val.num);
                } else if (print.type == Value_String) {
                    printf("%s\n", print.val.str);
                } else if (print.type == Value_Array) {
                    print_array(vm, &print);
                }   else if (print.type == Value_Identifier) {
                    Variable *val = get_entry(vm->vars->entries, vm->vars->capacity, print.hash)->value;
                    if (val->value.type == Value_Number)
                        printf("%ld\n", val->value.val.num);
                    else if (val->value.type == Value_String)
                        printf("%s\n", val->value.val.str);
                    else if (val->value.type == Value_Array)
                        print_array(vm, &val->value);
                    else ERR("cant print type %d\n", val->value.type)
                } else
                    ERR("cant print type %d\n", print.type)
                break;
            case OP_DEFINE_VARIABLE:;
                Value value = stack_pop;
                Value name = stack_pop;
                Variable *var = calloc(1, sizeof(Variable));
                var->name = name.val.str;
                var->value = value;
                var->index = -1;
                insert_entry(vm->vars, name.hash, Entry_Variable, var);
                break;
            case OP_ADD:;
                Value op2 = stack_pop;
                Value op1 = stack_pop;

                if (op1.type == Value_Identifier) op1 = ((Variable*)get_entry(vm->vars->entries, vm->vars->capacity, op1.hash)->value)->value;
                if (op2.type == Value_Identifier) op2 = ((Variable*)get_entry(vm->vars->entries, vm->vars->capacity, op2.hash)->value)->value;

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
                    int64_t op1_len;
                    int64_t op2_len;

                    if (op1.type == Value_String) str1 = op1.val.str;
                    else if (op1.type == Value_Number) {
                        op1_len = num_len(op1.val.num);
                        str1 = format_str(op1_len + 1, "%ld", op1.val.num);
                    }
                    if (op2.type == Value_String) str2 = op2.val.str;
                    else if (op2.type == Value_Number) {
                        op2_len = num_len(op2.val.num);
                        str2 = format_str(op2_len + 1, "%ld", op2.val.num);
                    }

                    stack_push(((Value){
                        Value_String,
                        .val.str=format_str(op1_len + op2_len + 1, "%s%s", str1, str2),
                        true,
                        0
                    }));
                }
                break;
            case OP_SUBTRACT:
                binary_op(-, "cant subtract type %s and %s\n");
            case OP_MULTIPLY:
                binary_op(*, "cant multiply type %s and %s\n");
            case OP_DIVIDE:
                binary_op(/, "cant divide type %s and %s\n");
            case OP_LESS:
                binary_op(<, "cant less than type %s and %s\n");
            case OP_LESS_EQUAL:
                binary_op(<=, "cant less equal than type %s and %s\n");
            case OP_GREATER:
                binary_op(>, "cant greater than type %s and %s\n");
            case OP_GREATER_EQUAL:
                binary_op(>=, "cant greater equal than type %s and %s\n");
            case OP_AND:
                binary_op(==, "cant logical and type %s and %s\n");
            case OP_OR:
                binary_op(||, "cant logical or type %s and %s\n");
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

                if (op1.type == Value_Identifier) op1 = ((Variable*)get_entry(vm->vars->entries, vm->vars->capacity, op1.hash)->value)->value;
                if (op2.type == Value_Identifier) op2 = ((Variable*)get_entry(vm->vars->entries, vm->vars->capacity, op2.hash)->value)->value;

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
                        true,
                        0
                    }));
                } else ERR("cant not equal type %s and %s\n", find_value_type(op1.type), find_value_type(op2.type))
                break;}
            case OP_IS_EQUAL: {
                Value op2 = stack_pop;
                Value op1 = stack_pop;

                if (op1.type == Value_Identifier) op1 = ((Variable*)get_entry(vm->vars->entries, vm->vars->capacity, op1.hash)->value)->value;
                if (op2.type == Value_Identifier) op2 = ((Variable*)get_entry(vm->vars->entries, vm->vars->capacity, op2.hash)->value)->value;

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
                        true,
                        0
                    }));
                } else ERR("cant is equal type %s and %s\n", find_value_type(op1.type), find_value_type(op2.type))
                break;}
            case OP_SET_VARIABLE:{
                Value value = stack_pop;
                Value name = stack_pop;

                Variable *var = get_entry(vm->vars->entries, vm->vars->capacity, name.hash)->value;
                var->value = value;
                break;}
            case OP_START_IF:
            case OP_JUMP_IF_FALSE:{
                Value val = stack_pop;
                if (!val.val.num) {
                    i += COMBYTE(vm->code.data[i + 1], vm->code.data[i + 2]) - 1;
                } else {
                    i += 2;
                }
                break;}
            case OP_JUMP:
                i = COMBYTE(vm->code.data[i + 1], vm->code.data[i + 2]) - 1;
                break;
            case OP_ARRAY:{
                u_int16_t index = COMBYTE(vm->code.data[i + 1], vm->code.data[i + 2]);
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

                int64_t var_index = ((Variable*)get_entry(vm->vars->entries, vm->vars->capacity, array.hash)->value)->value.val.num;

                stack_push(vm->arrays.data[var_index][index.val.num]);

                break;}
            case OP_SET_ELEMENT:{
                Value new_val = stack_pop;
                Value index = stack_pop;
                Value array = stack_pop;

                int64_t var_index = ((Variable*)get_entry(vm->vars->entries, vm->vars->capacity, array.hash)->value)->value.val.num;
                vm->arrays.data[var_index][index.val.num] = new_val;

                break;}
            default: ERR("cant do op %d\n", vm->code.data[i])
        }
    }
}
