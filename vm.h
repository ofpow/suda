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

typedef enum {
    OP_CONSTANT,
    OP_PRINTLN,
    OP_DEFINE_VARIABLE,
} Op_Code;

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
            default: ERR("cant do op %d\n", vm->code[i])
        }
    }
}
