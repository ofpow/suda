#pragma once

#define read_index (COMBYTE(vm->func->code.data[i + 1], vm->func->code.data[i + 2]))
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

#define unary_op(op, msg) do {                          \
    Value op1 = stack_pop;                              \
    if (op1.type == Value_Number) {                     \
        stack_push(((Value) {                           \
            Value_Number,                               \
            .val.num=(op(op1.val.num)),                 \
            false,                                      \
            0                                           \
        }));                                            \
    } else ERR(msg, get_loc, find_value_type(op1.type)) \
} while (0);                                            \
break                                                   \

typedef struct Call_Frame {
    Function *func;
    Value *slots;
    int64_t return_index;
    Location loc;
} Call_Frame;

typedef struct VM {
    Function *func;

    Arrays arrays;

    Map *vars;

    Value stack[STACK_SIZE];
    Value *stack_top;

    Functions funcs;
    Call_Frame call_stack[STACK_SIZE];
    int call_stack_count;
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

void print_value(Value val) {
    switch (val.type) {
        case Value_Number:
            printf("%ld\n", val.val.num);
            break;
        case Value_String:
            printf("\"%s\"\n", val.val.str);
            break;
        case Value_Identifier:
            printf("%s\n", val.val.str);
            break;
        default: ERR("cant print type %s\n", find_value_type(val.type))
    }
}

void disassemble(VM *vm) {
    for (int j = 0; j < vm->funcs.index; j++) {
        if (j > 0) printf("\n");
        printf("Disassembly of function %s:\n", vm->funcs.data[j].name);
        for (int i = 0; i < vm->funcs.data[j].code.index; i++) {
            switch(vm->funcs.data[j].code.data[i]) {
                case OP_CONSTANT:
                    printf("%-6d OP_CONSTANT:      ", i);
                    print_value(vm->funcs.data[j].constants.data[vm->funcs.data[j].code.data[i + 1]]);
                    i++;
                    break;
                case OP_CONSTANT_LONG:
                    printf("%-6d OP_CONSTANT_LONG: ", i);
                    print_value(vm->funcs.data[j].constants.data[COMBYTE(vm->funcs.data[j].code.data[i + 1], vm->funcs.data[j].code.data[i + 2])]);
                    i += 2;
                    break;
                case OP_ARRAY:
                    printf("%-6d OP_ARRAY:         index %d\n", i, COMBYTE(vm->funcs.data[j].code.data[i + 1], vm->funcs.data[j].code.data[i + 2]));
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
                case OP_PRINT:
                    printf("%-6d OP_PRINT\n", i);
                    break;
                case OP_DEFINE_GLOBAL:
                    printf("%-6d OP_DEFINE_GLOBAL: var %s\n", i, vm->funcs.data[j].constants.data[COMBYTE(vm->funcs.data[j].code.data[i + 1], vm->funcs.data[j].code.data[i + 2])].val.str);
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
                    printf("%-6d OP_SET_GLOBAL:    var %s \n", i, vm->funcs.data[j].constants.data[COMBYTE(vm->funcs.data[j].code.data[i + 1], vm->funcs.data[j].code.data[i + 2])].val.str);
                    i += 2;
                    break;
                case OP_GET_GLOBAL:
                    printf("%-6d OP_GET_GLOBAL:    var %s \n", i, vm->funcs.data[j].constants.data[COMBYTE(vm->funcs.data[j].code.data[i + 1], vm->funcs.data[j].code.data[i + 2])].val.str);
                    i += 2;
                    break;
                case OP_JUMP_IF_FALSE:
                    printf("%-6d OP_JUMP_IF_FALSE: offset %d\n", i, COMBYTE(vm->funcs.data[j].code.data[i + 1], vm->funcs.data[j].code.data[i + 2]));
                    i += 2;
                    break;
                case OP_START_IF:
                    printf("%-6d OP_START_IF:      offset %d\n", i, COMBYTE(vm->funcs.data[j].code.data[i + 1], vm->funcs.data[j].code.data[i + 2]));
                    i += 2;
                    break;
                case OP_JUMP:
                    printf("%-6d OP_JUMP:          index %d\n", i, COMBYTE(vm->funcs.data[j].code.data[i + 1], vm->funcs.data[j].code.data[i + 2]));
                    i += 2;
                    break;
                case OP_SET_LOCAL:
                    printf("%-6d OP_SET_LOCAL:     index %d\n", i, vm->funcs.data[j].code.data[i + 1]);
                    i++;
                    break;
                case OP_GET_LOCAL:
                    printf("%-6d OP_GET_LOCAL:     index %d\n", i, vm->funcs.data[j].code.data[i + 1]);
                    i++;
                    break;
                case OP_POP:
                    printf("%-6d OP_POP:           amount: %d\n", i, COMBYTE(vm->funcs.data[j].code.data[i + 1], vm->funcs.data[j].code.data[i + 2])); 
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
                    printf("%-6d OP_CALL:          func: %s\n", i, vm->funcs.data[COMBYTE(vm->funcs.data[j].code.data[i + 1], vm->funcs.data[j].code.data[i + 2])].name); 
                    i += 2;
                    break;
                case OP_RETURN:
                    printf("%-6d OP_RETURN\n", i);
                    break;
                case OP_MODULO:
                    printf("%-6d OP_MODULO\n", i);
                    break;
                case OP_BIT_AND:
                    printf("%-6d OP_BIT_AND\n", i);
                    break;
                case OP_BIT_OR:
                    printf("%-6d OP_BIT_OR\n", i);
                    break;
                case OP_BIT_XOR:
                    printf("%-6d OP_BIT_XOR\n", i);
                    break;
                case OP_BIT_NOT:
                    printf("%-6d OP_BIT_NOT\n", i);
                    break;
                case OP_LSHIFT:
                    printf("%-6d OP_LSHIFT\n", i);
                    break;
                case OP_RSHIFT:
                    printf("%-6d OP_RSHIFT\n", i);
                    break;
                case OP_POWER:
                    printf("%-6d OP_POWER\n", i);
                    break;
                case OP_RETURN_NOTHING:
                    printf("%-6d OP_RETURN_NOTHING\n", i);
                    break;
                case OP_APPEND:
                    printf("%-6d OP_APPEND:        var ", i);
                    print_value(vm->funcs.data[j].constants.data[COMBYTE(vm->funcs.data[j].code.data[i + 1], vm->funcs.data[j].code.data[i + 2])]);
                    i += 2;
                    break;
                case OP_BREAK:
                    printf("%-6d OP_BREAK:         offset %d\n", i, COMBYTE(vm->funcs.data[j].code.data[i + 1], vm->funcs.data[j].code.data[i + 2]));
                    i += 2;
                    break;
                case OP_CONTINUE:
                    printf("%-6d OP_CONTINUE:      index %d\n", i, COMBYTE(vm->funcs.data[j].code.data[i + 1], vm->funcs.data[j].code.data[i + 2]));
                    i += 2;
                    break;
                case OP_FOR:
                    printf("%-6d OP_FOR\n", i);
                    i += 5;
                    break;
                default:
                    ERR("ERROR in %s on line %ld: cant disassemble op type %s\n", get_loc, find_op_code(vm->funcs.data[j].code.data[i]));
            }
        }
    }
}

void run(VM *vm) {
    for (int i = 0; i < vm->func->code.index; i++) {
        Call_Frame *frame = &vm->call_stack[vm->call_stack_count - 1];
        debug("%-6d %s\n", i, find_op_code(vm->func->code.data[i]));
        switch (vm->func->code.data[i]) {
            case OP_CONSTANT:;{
                u_int8_t index = vm->func->code.data[++i];
                stack_push(vm->func->constants.data[index]);
                break;}
            case OP_CONSTANT_LONG:;
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
            case OP_PRINT:{
                Value print = stack_pop;
                if (print.type == Value_Number) {
                    printf("%ld", print.val.num);
                } else if (print.type == Value_String) {
                    printf("%s", print.val.str);
                    if (print.mutable == true) free(print.val.str);
                } else if (print.type == Value_Array) {
                    print_array(vm, &print);
                } else
                    ERR("ERROR in %s on line %ld: cant print type %s\n", get_loc, find_value_type(print.type))
                break;}
            case OP_DEFINE_GLOBAL:;
                Value name = vm->func->constants.data[read_index];
                Value value = stack_pop;

                Variable *var = calloc(1, sizeof(Variable));
                var->name = name.val.str;
                var->value = value;

                bool valid = insert_entry(vm->vars, name.hash, Entry_Variable, var);
                if (!valid) ERR("ERROR in %s on line %ld: cant assign `%s` multiple times\n", get_loc, name.val.str)

                i += 2;
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
            case OP_MODULO:
                binary_op(%, "ERROR in %s on line %ld: cant modulo type %s and %s\n");
            case OP_BIT_AND:
                binary_op(&, "ERROR in %s on line %ld: cant bitwise and type %s and %s\n");
            case OP_BIT_OR:
                binary_op(|, "ERROR in %s on line %ld: cant bitwise or type %s and %s\n");
            case OP_BIT_XOR:
                binary_op(^, "ERROR in %s on line %ld: cant bitwise xor type %s and %s\n");
            case OP_LSHIFT:
                binary_op(<<, "ERROR in %s on line %ld: cant lshift type %s and %s\n");
            case OP_RSHIFT:
                binary_op(>>, "ERROR in %s on line %ld: cant rshift type %s and %s\n");
            case OP_POWER:{
                Value op2 = stack_pop;
                Value op1 = stack_pop;
                ASSERT((op1.type == Value_Number) && (op2.type == Value_Number), "cant exponentiate type %s and type %s\n", find_value_type(op1.type), find_value_type(op2.type))
                stack_push(((Value) {
                    Value_Number,
                    .val.num=(exponentiate(op1.val.num, op2.val.num)),
                    false,
                    0 
                }));
                break;}
            case OP_BIT_NOT:
                unary_op(~, "ERROR in %s on line %ld: cant bitwise not type %s\n");
            case OP_NOT:;
                unary_op(!, "ERROR in %s on line %ld: cant logical not type %s\n");
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
                } else stack_push(((Value){
                        Value_Number,
                        .val.num=0,
                        false,
                        0
                    }));
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
                } else stack_push(((Value){
                        Value_Number,
                        .val.num=0,
                        false,
                        0
                    }));
                break;}
            case OP_SET_GLOBAL:{
                Value name = vm->func->constants.data[read_index];
                i += 2;
                Value value = stack_pop;

                Variable *var = get_entry(vm->vars->entries, vm->vars->capacity, name.hash)->value;
                if (var == NULL) ERR("ERROR in %s on line %ld: tried to set nonexistent global %s\n", get_loc, name.val.str)
                else if (var->value.type == Value_Array) {
                    Value *array = vm->arrays.data[var->value.val.num];
                    for (int i = 1; i < array[0].val.num; i++) {
                        if (array[i].type == Value_String && array[i].mutable == true) free(array[i].val.str);
                    }
                    free(vm->arrays.data[var->value.val.num]);
                    Value *new_array = vm->arrays.data[value.val.num];
                    array = calloc(new_array[0].val.num, sizeof(Value));

                    for (int i = 0; i < new_array[0].val.num; i++) {
                        if (new_array[i].type == Value_String && new_array[i].mutable)
                            array[i] = (Value){Value_String, .val.str=strdup(new_array[i].val.str), true, 0};
                        else 
                            array[i] = new_array[i];
                    }
                    vm->arrays.data[var->value.val.num] = array;
                } else if (var->value.type == Value_String && var->value.mutable) {
                    free(var->value.val.str);
                    var->value = value;
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

                Variable *var = get_entry(vm->vars->entries, vm->vars->capacity, array.hash)->value;
                if (var->value.type == Value_String) {
                    stack_push(((Value) {
                        Value_String,
                        .val.str=format_str(2, "%c", var->value.val.str[index.val.num - 1]),
                        true,
                        0
                    }));                  
                } else {
                    int64_t arr_index = var->value.val.num;
                    if (vm->arrays.data[arr_index][index.val.num].type == Value_Identifier) {
                        Variable *var = get_entry(vm->vars->entries, vm->vars->capacity, vm->arrays.data[arr_index][index.val.num].hash)->value;
                        stack_push(var->value);
                    } else if (vm->arrays.data[arr_index][index.val.num].type == Value_String && vm->arrays.data[arr_index][index.val.num].mutable) stack_push(((Value) {
                        Value_String, 
                        .val.str=vm->arrays.data[arr_index][index.val.num].val.str,
                        false,
                        0
                    }));
                    else stack_push(vm->arrays.data[arr_index][index.val.num]);
                }

                break;}
            case OP_SET_ELEMENT:{
                Value new_val = stack_pop;
                Value index = stack_pop;
                Value array = stack_pop;

                Variable *var = get_entry(vm->vars->entries, vm->vars->capacity, array.hash)->value;
                if (var->value.type == Value_String) {
                    var->value.val.str[index.val.num - 1] = new_val.val.str[0];
                } else {
                    int64_t var_index = var->value.val.num;
                    if (vm->arrays.data[var_index][index.val.num].type == Value_String && vm->arrays.data[var_index][index.val.num].mutable) free(vm->arrays.data[var_index][index.val.num].val.str);
                    vm->arrays.data[var_index][index.val.num] = new_val;
                }

                break;}
            case OP_GET_GLOBAL:{
                Value var_name = vm->func->constants.data[read_index];
                i += 2;
                Variable *var = get_entry(vm->vars->entries, vm->vars->capacity, var_name.hash)->value;
                if (var == NULL) ERR("ERROR in %s on line %ld: tried to get nonexistent var %s\n", get_loc, var_name.val.str);
                if (var->value.type == Value_String && var->value.mutable) stack_push(((Value) {
                    Value_String, 
                    .val.str=var->value.val.str,
                    false,
                    0
                }));
                else stack_push(var->value);
                break;}
            case OP_GET_LOCAL: {
                i++;
                Value val = frame->slots[vm->func->code.data[i]];
                if (val.type == Value_String && val.mutable) stack_push(((Value) {
                    Value_String, 
                    .val.str=val.val.str,
                    false,
                    0
                }));
                else stack_push(val);
                break;}
            case OP_SET_LOCAL: {
                i++;
                if (frame->slots[vm->func->code.data[i]].type == Value_String && frame->slots[vm->func->code.data[i]].mutable) free(frame->slots[vm->func->code.data[i]].val.str);
                frame->slots[vm->func->code.data[i]] = stack_pop;
                break;}
            case OP_POP:
                vm->stack_top -= read_index;
                i += 2;
                break;
            case OP_LEN:;
                Value array = stack_pop;
                if (array.type == Value_Array) {
                    stack_push(((Value){
                        Value_Number,
                        .val.num=vm->arrays.data[array.val.num][0].val.num - 1,
                        false,
                        0
                    }));
                } else if (array.type == Value_String) {
                    stack_push(((Value){
                        Value_Number,
                        .val.num=strlen(array.val.str),
                        false,
                        0
                    }));
                } else ERR("ERROR in %s on line %ld: cant do len of %s\n", get_loc, find_value_type(array.type))
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
                vm->call_stack[vm->call_stack_count - 1].return_index = i + 2;
                if (vm->call_stack_count >= CALL_STACK_SIZE) ERR("ERROR in %s on line %ld: call stack overflow\n", get_loc)
                Call_Frame *frame = &vm->call_stack[vm->call_stack_count++];
                frame->func = &vm->funcs.data[read_index];
                frame->slots = vm->stack_top - frame->func->arity;
                frame->loc = vm->func->locs.data[i];
                vm->func = &vm->funcs.data[read_index];
                i = -1;
                break;}
            case OP_RETURN:{
                Value result = stack_pop;
                vm->call_stack_count--;
                vm->stack_top = frame->slots;
                frame = &vm->call_stack[vm->call_stack_count - 1];
                vm->func = frame->func;
                i = frame->return_index;
                stack_push(result);
                break;}
            case OP_RETURN_NOTHING:{
                vm->call_stack_count--;
                vm->stack_top = frame->slots;
                frame = &vm->call_stack[vm->call_stack_count - 1];
                vm->func = frame->func;
                i = frame->return_index;
                break;}
            case OP_APPEND:{
                Value val = stack_pop;
                Value var_name = vm->func->constants.data[read_index];
                i += 2;

                Variable *var = get_entry(vm->vars->entries, vm->vars->capacity, var_name.hash)->value;
                if (var == NULL) ERR("ERROR in %s on line %ld: tried to append to nonexistent var %s\n", get_loc, var_name.val.str);

                Value *array = vm->arrays.data[var->value.val.num];
                int64_t arr_len = array[0].val.num + 1;
                array = realloc(array, arr_len * sizeof(Value));
                array[arr_len - 1] = val;
                array[0].val.num = arr_len;
                vm->arrays.data[var->value.val.num] = array;
                break;}
            case OP_BREAK:{
                i += read_index - 1;
                break;}
            case OP_CONTINUE:{
                i = read_index - 1;
                break;}
            case OP_FOR:{
                Value array = stack_pop;
                Value *index = &vm->func->constants.data[read_index];
                i += 2;
                Value *local = &frame->slots[vm->func->code.data[++i]];
                if (array.type == Value_Array) {
                    if (index->val.num == 1) {
                        stack_push(vm->arrays.data[array.val.num][1]);
                    } else if (index->val.num >= vm->arrays.data[array.val.num][0].val.num) {
                        index->val.num = 1;
                        i = read_index - 1;
                    } else {
                        *local = vm->arrays.data[array.val.num][index->val.num];
                    }
                    index->val.num++;
                } else if (array.type == Value_String) {
                    if (index->val.num == 1) {
                        stack_push(((Value){
                            Value_String,
                            .val.str=format_str(2, "%c", array.val.str[index->val.num - 1]),
                            true,
                            0
                        }));
                    } else if (index->val.num >= (int64_t)strlen(array.val.str) + 1) {
                        free(local->val.str);
                        index->val.num = 1;
                        i = read_index + 1;
                    } else {
                        free(local->val.str);
                        *local = (Value){
                            Value_String,
                            .val.str=format_str(2, "%c", array.val.str[index->val.num - 1]),
                            true,
                            0
                        };
                    }
                    index->val.num++;
                } else {
                    i -= 3;
                    ERR("ERROR in %s on line %ld: cant loop through type %s\n", get_loc, find_value_type(array.type))
                } 
                i += 2;
                break;}
            case OP_EXIT:{
                Value val = stack_pop;
                free_mem(val.val.num);
                break;}
            default: ERR("ERROR in %s on line %ld: cant do %s\n", get_loc, find_op_code(vm->func->code.data[i]))
        }
    }
}
