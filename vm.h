#pragma once

#define STACK_SIZE 16384

#define FIRST_BYTE(_val) (u_int8_t)((_val) >> 8)
#define SECOND_BYTE(_val) (u_int8_t)((_val) & 0xFF)
#define COMBYTE(_byte1, _byte2) (((_byte1) << 8) | (_byte2))
#define stack_pop (*--vm->stack_top)
#define stack_push(_val) do {       \
    *vm->stack_top = (_val);        \
    vm->stack_top++;                \
} while (0);                        \

#define binary_op(op, msg) do {                                             \
    Value op1 = stack_pop;                                                  \
    Value op2 = stack_pop;                                                  \
    if (op1.type == Value_Number && op2.type == Value_Number) {             \
        stack_push(((Value) {                                               \
            Value_Number,                                                   \
            .val.num=(op1.val.num op op2.val.num),                          \
            false,                                                          \
            0                                                               \
        }));                                                                \
    } else ERR(msg, find_value_type(op1.type), find_value_type(op2.type))   \
    break;                                                                  \
} while (0)                                                                 \

typedef enum {
    OP_CONSTANT,
    OP_PRINTLN,
    OP_DEFINE_VARIABLE,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_LESS,
    OP_LESS_EQUAL,
    OP_GREATER,
    OP_GREATER_EQUAL,
    OP_IS_EQUAL,
    OP_AND,
    OP_OR,
    OP_NOT,
    OP_NOT_EQUAL,
} Op_Code;

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

typedef struct VM {
    u_int8_t *code;
    int64_t code_index;
    int64_t code_capacity;

    Value *constants;
    int64_t constants_index;
    int64_t constants_capacity;

    Map *vars;

    Value stack[STACK_SIZE];
    Value *stack_top;
} VM;

void disassemble(VM *vm) {
    for (int i = 0; i < vm->code_index; i++) {
        switch(vm->code[i]) {
            case OP_CONSTANT:
                printf("OP_CONSTANT: index %d\n", (((u_int16_t)vm->code[i + 1] << 8) | vm->code[i + 2]));
                i += 2;
                break;
            case OP_PRINTLN:
                printf("OP_PRINTLN\n");
                break;
            default:
                ERR("cant handle op type %d\n", vm->code[i]);
        }
    }
}

void compile_constant(AST_Value *value, VM *vm) {
    if (value->type == Value_Number)
        append(vm->constants, ((Value){value->type, .val.num=NUM(value->value), false, 0}), vm->constants_index, vm->constants_capacity)
    else if (value->type == Value_String)
        append(vm->constants, ((Value){value->type, .val.str=value->value, false, 0}), vm->constants_index, vm->constants_capacity)
    else if (value->type == Value_Identifier)
        append(vm->constants, ((Value){value->type, .val.str=value->value, false, value->hash}), vm->constants_index, vm->constants_capacity)
    else
        ERR("cant compile constant of type %d\n", value->type)

    u_int16_t index = vm->constants_index - 1;
    append(vm->code, OP_CONSTANT, vm->code_index, vm->code_capacity)
    append(vm->code, FIRST_BYTE(index), vm->code_index, vm->code_capacity)
    append(vm->code, SECOND_BYTE(index), vm->code_index, vm->code_capacity)
}

void compile_expr(Node *n, VM *vm) {
    switch (n->type) {
        case AST_Literal:
            compile_constant(n->value, vm);
            break;
        case AST_Identifier:
            compile_constant(n->value, vm);
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
            compile_expr(n->left, vm);
            compile_expr(n->right, vm);
            append(vm->code, ast_to_op_code(n->type), vm->code_index, vm->code_capacity)
            break;
        case AST_Not:
            compile_expr(n->left, vm);
            append(vm->code, OP_NOT, vm->code_index, vm->code_capacity)
            break;
        default: ERR("cant handle node type %s\n", find_ast_type(n->type));
    }
}

void compile(Node **nodes, int64_t nodes_size, VM *vm) {
    for (int i = 0; i < nodes_size; i++) {
        switch (nodes[i]->type) {
            case AST_Println:
                compile_expr(nodes[i]->left, vm);
                append(vm->code, OP_PRINTLN, vm->code_index, vm->code_capacity)
                break;
            case AST_Var_Assign:
                compile_constant(nodes[i]->value, vm); // name
                compile_expr(nodes[i]->left, vm); // value
                append(vm->code, OP_DEFINE_VARIABLE, vm->code_index, vm->code_capacity)
                break;
            default: ERR("cant compile node type %s\n", find_ast_type(nodes[i]->type))
        }
    }
}

void run(VM *vm) {
    for (int i = 0; i < vm->code_index; i++) {
        switch (vm->code[i]) {
            case OP_CONSTANT:;
                u_int16_t index = COMBYTE(vm->code[i + 1], vm->code[i + 2]);
                stack_push(vm->constants[index])
                i += 2;
                break;
            case OP_PRINTLN:;
                Value print = stack_pop;
                if (print.type == Value_Number) {
                    printf("%ld\n", print.val.num);
                } else if (print.type == Value_String) {
                    printf("%s\n", print.val.str);
                }   else if (print.type == Value_Identifier) {
                    Variable *val = get_entry(vm->vars->entries, vm->vars->capacity, print.hash)->value;
                    if (val->value.type == Value_Number)
                        printf("%ld\n", val->value.val.num);
                    else if (val->value.type == Value_String)
                        printf("%s\n", val->value.val.str);
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
            default: ERR("cant do op %d\n", vm->code[i])
        }
    }
}
