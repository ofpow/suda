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

#define binary_op(op, msg) do {                                                                                                    \
    Value op1 = stack_pop;                                                                                                         \
    Value op2 = stack_pop;                                                                                                         \
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
    OP_SET_VARIABLE,
    OP_JUMP_IF_FALSE,
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

define_array(Code, u_int8_t);
define_array(Constants, Value);

typedef struct VM {
    Jump_Indices jump_indices;

    Code code;

    Constants constants;

    Map *vars;

    Value stack[STACK_SIZE];
    Value *stack_top;
} VM;

void disassemble(VM *vm) {
    for (int i = 0; i < vm->code.index; i++) {
        switch(vm->code.data[i]) {
            case OP_CONSTANT:
                printf("%-6d OP_CONSTANT:      index %d\n", i, COMBYTE(vm->code.data[i + 1], vm->code.data[i + 2]));
                i += 2;
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
                printf("%-6d OP_JUMP_IF_FALSE: index %d\n", i, COMBYTE(vm->code.data[i + 1], vm->code.data[i + 2]));
                i += 2;
                break;
            default:
                ERR("cant disassemble op type %d\n", vm->code.data[i]);
        }
    }
}

void compile_constant(AST_Value *value, VM *vm) {
    if (value->type == Value_Number)
        append_new(vm->constants, ((Value){value->type, .val.num=NUM(value->value), false, 0}));
    else if (value->type == Value_String)
        append_new(vm->constants, ((Value){value->type, .val.str=value->value, false, 0}));
    else if (value->type == Value_Identifier)
        append_new(vm->constants, ((Value){value->type, .val.str=value->value, false, value->hash}));
    else
        ERR("cant compile constant of type %d\n", value->type)

    u_int16_t index = vm->constants.index - 1;
    append_new(vm->code, OP_CONSTANT);
    append_new(vm->code, FIRST_BYTE(index));
    append_new(vm->code, SECOND_BYTE(index));
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
            append_new(vm->code, ast_to_op_code(n->type));
            break;
        case AST_Not:
            compile_expr(n->left, vm);
            append_new(vm->code, OP_NOT);
            break;
        default: ERR("cant handle node type %s\n", find_ast_type(n->type));
    }
}

void compile(Node **nodes, int64_t nodes_size, VM *vm) {
    for (int i = 0; i < nodes_size; i++) {
        switch (nodes[i]->type) {
            case AST_Println:
                compile_expr(nodes[i]->left, vm);
                append_new(vm->code, OP_PRINTLN);
                break;
            case AST_Var_Assign:
                compile_constant(nodes[i]->value, vm); // name
                compile_expr(nodes[i]->left, vm); // value
                append_new(vm->code, OP_DEFINE_VARIABLE);
                break;
            case AST_Identifier:
                compile_constant(nodes[i]->value, vm); // name
                compile_expr(nodes[i]->left, vm); // value
                append_new(vm->code, OP_SET_VARIABLE);
                break;
            case AST_If:
                compile_expr(nodes[i]->left, vm);

                append_new(vm->jump_indices, vm->code.index);
                append_new(vm->code, OP_JUMP_IF_FALSE);
                append_new(vm->code, 0);
                append_new(vm->code, 0);
                break;
            case AST_Semicolon:;
                int64_t index = vm->jump_indices.data[vm->jump_indices.index - 1];
                u_int16_t offset = vm->code.index - index;
                vm->code.data[index + 1] = FIRST_BYTE(offset);
                vm->code.data[index + 2] = SECOND_BYTE(offset);
                vm->jump_indices.index--;
                break;
            default: ERR("cant compile node type %s\n", find_ast_type(nodes[i]->type))
        }
    }
}

void run(VM *vm) {
    for (int i = 0; i < vm->code.index; i++) {
        switch (vm->code.data[i]) {
            case OP_CONSTANT:;
                u_int16_t index = COMBYTE(vm->code.data[i + 1], vm->code.data[i + 2]);
                stack_push(vm->constants.data[index])
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
            case OP_JUMP_IF_FALSE:{
                Value val = stack_pop;
                if (!val.val.num) {
                    i += COMBYTE(vm->code.data[i + 1], vm->code.data[i + 2]) - 1;
                } else {
                    i += 2;
                }
            break;}
            default: ERR("cant do op %d\n", vm->code.data[i])
        }
    }
}
