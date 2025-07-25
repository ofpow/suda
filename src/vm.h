#pragma once


#define read_index (COMBYTE(vm->func->code.data[pc + 1], vm->func->code.data[pc + 2]))
#define stack_pop (*--vm->stack_top)
#define stack_push(_val) do { \
    *vm->stack_top = (_val);  \
    vm->stack_top++;          \
} while (0)                   \

#define dup_value(_val) (_val.type == Value_String) ?                                         \
    (Value){Value_String, .val.str={strdup(_val.val.str.chars), _val.val.str.len}, true, 0} : \
    _val                                                                                      \

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
dispatch()                                                                          \

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
dispatch()                                              \

typedef struct Call_Frame {
    Function *func;
    Value *slots;
    int64_t return_index;
    Location loc;
#ifdef PROFILE
    int index;
#endif
} Call_Frame;

typedef struct VM {
    Function *func;

    Map *vars;

    Value stack[STACK_SIZE];
    Value *stack_top;

    Functions funcs;
    Call_Frame call_stack[STACK_SIZE];
    int call_stack_count;
} VM;

Value *dup_array(Value *val) {
    Value *array = calloc(ARRAY_SIZE(val[0].val.num), sizeof(Value));

    u_int32_t len = ARRAY_LEN(val[0].val.num);
    for (u_int32_t i = 0; i < len; i++) {
        if ((val[i].type == Value_Number) || (val[i].type == Value_Array)) {
            array[i] = val[i];
        } else if (val[i].type == Value_String) {
            array[i] = (Value){
                Value_String,
                .val.str={strdup(val[i].val.str.chars), val[i].val.str.len},
                true,
                0
            };
        } else if (val[i].type == Value_Identifier) {
            array[i] = (Value){
                Value_String,
                .val.str={strdup(val[i].val.str.chars), val[i].val.str.len},
                true,
                val[i].hash
            };
        }
    }
    return array;
}

void free_value_array(Value *array) {
    u_int32_t len = ARRAY_LEN(array[0].val.num);
    for (u_int32_t i = 1; i < len; i++) {
        if ((((array[i].type == Value_String)) || (array[i].type == Value_Identifier)) && array[i].mutable)
            free(array[i].val.str.chars);
    }
    free(array);
}

char *format_array(Value *val) {
    Value *array = val->val.array;
    if (ARRAY_LEN(array[0].val.num) < 2) return strdup("[]");

    int64_t str_len = 2;
    char *str = format_str(str_len, "[");

    u_int32_t len = ARRAY_LEN(array[0].val.num);
    for (u_int32_t i = 1; i < len; i++) {
        if (array[i].type == Value_Number) {
            int64_t len = num_len(array[i].val.num) + 2;
            if (array[i].val.num < 0) len++;
            char *num = format_str(len, "%ld", array[i].val.num);
            str_len += len;
            str = realloc(str, str_len);
            strcat(str, num);
            strcat(str, ", ");
            free(num);
        } else if (array[i].type == Value_Identifier) {
            str_len += array[i].val.str.len + 2;
            str = realloc(str, str_len);
            strcat(str, array[i].val.str.chars);
            strcat(str, ", ");
        } else if (array[i].type == Value_String) {
            int len = array[i].val.str.len + 4;
            str_len += len;
            str = realloc(str, str_len);
            char *x = format_str(len + 1, "\"%s\"", array[i].val.str.chars);
            strcat(str, x);
            strcat(str, ", ");
            free(x);
        } else if (array[i].type == Value_Array) {
            char *x = format_array(&array[i]);
            str_len += strlen(x) + 2;
            str = realloc(str, str_len);
            strcat(str, x);
            strcat(str, ", ");
            free(x);
        } else ERR("cant print %s as part of array\n", find_value_type(array[i].type))
    }
    str[str_len - 3] = ']';
    str[str_len - 2] = 0;
    return str;
}

void print_value(Value val) {
    switch (val.type) {
        case Value_Number:
            printf("%ld\n", val.val.num);
            break;
        case Value_String:
            printf("\"%.*s\"\n", Print(val.val.str));
            break;
        case Value_Identifier:
            printf("%.*s\n", Print(val.val.str));
            break;
        case Value_Array:
            printf("array\n");
            break;
        default: ERR("cant print type %s\n", find_value_type(val.type))
    }
}

void print_state(Value *stack_top, Value *stack) {
    printf("{");
    for (Value *val = stack_top; val > stack; val--) {
        switch (val->type) {
            case Value_Number:
                printf("%ld, ", val->val.num);
                break;
            case Value_String:
                printf("\"%.*s\", ", Print(val->val.str));
                break;
            case Value_Identifier:
                printf("%.*s, ", Print(val->val.str));
                break;
            case Value_Array:
                printf("array, ");
                break;
            default: ERR("cant print type %s\n", find_value_type(val->type))
        }
    }
    printf("}\n");
}

void disassemble(VM *vm) {
    for (int j = 0; j < vm->funcs.index; j++) {
        if (j > 0) printf("\n");
        Function func = vm->funcs.data[j];
        printf("Disassembly of function %s:\n", func.name);

        int line = 0;
        char *line_str = NULL;
        for (int i = 0; i < func.code.index; i++) {
            free(line_str);
            if (func.locs.data[i].line > line) {
                line = func.locs.data[i].line;
                line_str = format_str(7, "%6ld", line - 1);
            } else {
                line_str = format_str(7, "     |");
            }
            switch(func.code.data[i]) {
                case OP_CONSTANT:
                    printf("%-6d %s OP_CONSTANT:          ", i, line_str);
                    print_value(func.constants.data[COMBYTE(func.code.data[i + 1], func.code.data[i + 2])]);
                    i += 2;
                    break;
                case OP_GET_ELEMENT:
                    printf("%-6d %s OP_GET_ELEMENT\n", i, line_str);
                    break;
                case OP_SET_ELEMENT_GLOBAL:
                    printf("%-6d %s OP_SET_ELEMENT_GLOBAL\n", i, line_str);
                    i += 2;
                    break;
                case OP_SET_ELEMENT_LOCAL:
                    printf("%-6d %s OP_SET_ELEMENT_LOCAL\n", i, line_str);
                    i++;
                    break;
                case OP_PRINTLN:
                    printf("%-6d %s OP_PRINTLN\n", i, line_str);
                    break;
                case OP_PRINT:
                    printf("%-6d %s OP_PRINT\n", i, line_str);
                    break;
                case OP_DEFINE_GLOBAL:
                    printf("%-6d %s OP_DEFINE_GLOBAL:     var %s\n", i, line_str, func.constants.data[COMBYTE(func.code.data[i + 1], func.code.data[i + 2])].val.str.chars);
                    i += 2;
                    break;
                case OP_ADD:
                    printf("%-6d %s OP_ADD\n", i, line_str);
                    break;
                case OP_SUBTRACT:
                    printf("%-6d %s OP_SUBTRACT\n", i, line_str);
                    break;
                case OP_MULTIPLY:
                    printf("%-6d %s OP_MULTIPLY\n", i, line_str);
                    break;
                case OP_DIVIDE:
                    printf("%-6d %s OP_DIVIDE\n", i, line_str);
                    break;
                case OP_LESS:
                    printf("%-6d %s OP_LESS\n", i, line_str);
                    break;
                case OP_LESS_EQUAL:
                    printf("%-6d %s OP_LESS_EQUAL\n", i, line_str);
                    break;
                case OP_GREATER:
                    printf("%-6d %s OP_GREATER\n", i, line_str);
                    break;
                case OP_GREATER_EQUAL:
                    printf("%-6d %s OP_GREATER_EQUAL\n", i, line_str);
                    break;
                case OP_IS_EQUAL:
                    printf("%-6d %s OP_IS_EQUAL\n", i, line_str);
                    break;
                case OP_AND:
                    printf("%-6d %s OP_AND\n", i, line_str);
                    break;
                case OP_OR:
                    printf("%-6d %s OP_OR\n", i, line_str);
                    break;
                case OP_NOT:
                    printf("%-6d %s OP_NOT\n", i, line_str);
                    break;
                case OP_NOT_EQUAL:
                    printf("%-6d %s OP_NOT_EQUAL\n", i, line_str);
                    break;
                case OP_SET_GLOBAL:
                    printf("%-6d %s OP_SET_GLOBAL:        var %s \n", i, line_str, func.constants.data[COMBYTE(func.code.data[i + 1], func.code.data[i + 2])].val.str.chars);
                    i += 2;
                    break;
                case OP_GET_GLOBAL:
                    printf("%-6d %s OP_GET_GLOBAL:        var %s \n", i, line_str, func.constants.data[COMBYTE(func.code.data[i + 1], func.code.data[i + 2])].val.str.chars);
                    i += 2;
                    break;
                case OP_JUMP_IF_FALSE:
                    printf("%-6d %s OP_JUMP_IF_FALSE:     offset %d\n", i, line_str, COMBYTE(func.code.data[i + 1], func.code.data[i + 2]));
                    i += 2;
                    break;
                case OP_START_IF:
                    printf("%-6d %s OP_START_IF:          offset %d\n", i, line_str, COMBYTE(func.code.data[i + 1], func.code.data[i + 2]));
                    i += 2;
                    break;
                case OP_JUMP:
                    printf("%-6d %s OP_JUMP:              index %d\n", i, line_str, COMBYTE(func.code.data[i + 1], func.code.data[i + 2]));
                    i += 2;
                    break;
                case OP_SET_LOCAL:
                    printf("%-6d %s OP_SET_LOCAL:         index %d\n", i, line_str, func.code.data[i + 1]);
                    i++;
                    break;
                case OP_GET_LOCAL:
                    printf("%-6d %s OP_GET_LOCAL:         index %d\n", i, line_str, func.code.data[i + 1]);
                    i++;
                    break;
                case OP_POP:
                    printf("%-6d %s OP_POP:               amount: %d\n", i, line_str, COMBYTE(func.code.data[i + 1], func.code.data[i + 2])); 
                    i += 2;
                    break;
                case OP_CAST_STR:
                    printf("%-6d %s OP_CAST_STR\n", i, line_str);
                    break;
                case OP_CAST_NUM:
                    printf("%-6d %s OP_CAST_NUM\n", i, line_str);
                    break;
                case OP_CALL:
                    printf("%-6d %s OP_CALL:              func: %s\n", i, line_str, vm->funcs.data[COMBYTE(func.code.data[i + 1], func.code.data[i + 2])].name); 
                    i += 2;
                    break;
                case OP_CALL_NATIVE:
                    printf("%-6d %s OP_CALL_NATIVE:       func: %s\n", i, line_str, native_names[COMBYTE(func.code.data[i + 1], func.code.data[i + 2])]); 
                    i += 2;
                    break;
                case OP_RETURN:
                    printf("%-6d %s OP_RETURN\n", i, line_str);
                    break;
                case OP_MODULO:
                    printf("%-6d %s OP_MODULO\n", i, line_str);
                    break;
                case OP_BIT_AND:
                    printf("%-6d %s OP_BIT_AND\n", i, line_str);
                    break;
                case OP_BIT_OR:
                    printf("%-6d %s OP_BIT_OR\n", i, line_str);
                    break;
                case OP_BIT_XOR:
                    printf("%-6d %s OP_BIT_XOR\n", i, line_str);
                    break;
                case OP_BIT_NOT:
                    printf("%-6d %s OP_BIT_NOT\n", i, line_str);
                    break;
                case OP_LSHIFT:
                    printf("%-6d %s OP_LSHIFT\n", i, line_str);
                    break;
                case OP_RSHIFT:
                    printf("%-6d %s OP_RSHIFT\n", i, line_str);
                    break;
                case OP_POWER:
                    printf("%-6d %s OP_POWER\n", i, line_str);
                    break;
                case OP_RETURN_NOTHING:
                    printf("%-6d %s OP_RETURN_NOTHING\n", i, line_str);
                    break;
                case OP_APPEND_LOCAL:
                    printf("%-6d %s OP_APPEND_LOCAL       index %d\n", i, line_str, func.code.data[i + 1]);
                    i++;
                    break;
                case OP_APPEND_GLOBAL:
                    printf("%-6d %s OP_APPEND_GLOBAL      var ", i, line_str);
                    print_value(func.constants.data[COMBYTE(func.code.data[i + 1], func.code.data[i + 2])]);
                    i += 2;
                    break;
                case OP_BREAK:
                    printf("%-6d %s OP_BREAK:             offset %d\n", i, line_str, COMBYTE(func.code.data[i + 1], func.code.data[i + 2]));
                    i += 2;
                    break;
                case OP_CONTINUE:
                    printf("%-6d %s OP_CONTINUE:          index %d\n", i, line_str, COMBYTE(func.code.data[i + 1], func.code.data[i + 2]));
                    i += 2;
                    break;
                case OP_FOR:
                    printf("%-6d %s OP_FOR                iter %d, local %d, index %d\n", i, line_str,
                            COMBYTE(func.code.data[i + 1], func.code.data[i + 2]), func.code.data[i + 3],
                            COMBYTE(func.code.data[i + 4], func.code.data[i + 5]));
                    i += 5;
                    break;
                case OP_ARRAY:
                    printf("%-6d %s OP_ARRAY:             index %d\n", i, line_str, COMBYTE(func.code.data[i + 1], func.code.data[i + 2]));
                    i += 2;
                    break;
                case OP_ENUMERATE:
                    printf("%-6d %s OP_ENUMERATE          iter %d, local %d, index %d, jump %d\n", i, line_str,
                            COMBYTE(func.code.data[i + 1], func.code.data[i + 2]), func.code.data[i + 3],
                            func.code.data[i + 4], COMBYTE(func.code.data[i + 5], func.code.data[i + 6]));
                    i += 6;
                    break;
                case OP_GET_GLOBAL_GET_CONSTANT:
                    printf("%-6d %s OP_GET_GLOBAL_GET_CONSTANT\n", i, line_str);
                    i += 4;
                    break;
                case OP_GET_LOCAL_GET_CONSTANT:
                    printf("%-6d %s OP_GET_LOCAL_GET_CONSTANT\n", i, line_str);
                    i += 3;
                    break;
                case OP_DONE:
                    printf("%-6d %s OP_DONE\n", i, line_str);
                    break;
                default:
                    ERR("ERROR cant disassemble op type %s\n", find_op_code(func.code.data[i]));
            }
        free(line_str);
        line_str = NULL;
        }
    }
}

void run(VM *vm) {
#ifdef PROFILE
    struct timespec tstart=(struct timespec){0,0};
    struct timespec tend=(struct timespec){0,0};
    clock_gettime(CLOCK_MONOTONIC, &tstart);
#endif
    
    static void* dispatch_table[] = {
#define X(x) &&x,
        ops
#undef X
    };
    Call_Frame *frame = &vm->call_stack[vm->call_stack_count - 1];
#ifdef PROFILE
#define dispatch() pc++; instr_profiler[frame->index]++; op_profiler[vm->func->code.data[pc]]++; debug("%-6d %s\n", pc, find_op_code(vm->func->code.data[pc])); goto *dispatch_table[vm->func->code.data[pc]]
#else
#define dispatch() pc++; debug("%-6d %s\n", pc, find_op_code(vm->func->code.data[pc])); goto *dispatch_table[vm->func->code.data[pc]]
#endif
    
    int pc = -1; 
    dispatch();
    while (1) {
        OP_CONSTANT:{
            u_int16_t index = read_index;
            stack_push(vm->func->constants.data[index]);
            pc += 2;
            dispatch();}
        OP_ARRAY:
            stack_push(((Value){
                Value_Array,
                .val.array=dup_array(vm->func->constants.data[read_index].val.array),
                true,
                0
            }));
            pc += 2;
            dispatch();
        OP_PRINTLN:{
            Value print = stack_pop;
            if (print.type == Value_Number) {
                printf("%ld\n", print.val.num);
            } else if (print.type == Value_String) {
                printf("%.*s\n", Print(print.val.str));
                if (print.mutable == true) free(print.val.str.chars);
            } else if (print.type == Value_Array) {
                char *s = format_array(&print);
                printf("%s\n", s);
                free(s);
                if (print.mutable == true) free_value_array(print.val.array);
            } else
                ERR("ERROR in %s on line %ld: cant print type %s\n", get_loc, find_value_type(print.type))
            dispatch();}
        OP_PRINT:{
            Value print = stack_pop;
            if (print.type == Value_Number) {
                printf("%ld", print.val.num);
            } else if (print.type == Value_String) {
                printf("%.*s", Print(print.val.str));
                if (print.mutable == true) free(print.val.str.chars);
            } else if (print.type == Value_Array) {
                char *s = format_array(&print);
                printf("%s", s);
                free(s);
                if (print.mutable == true) free_value_array(print.val.array);
            } else
                ERR("ERROR in %s on line %ld: cant print type %s\n", get_loc, find_value_type(print.type))
            dispatch();}
        OP_DEFINE_GLOBAL:;
            Value name = vm->func->constants.data[read_index];
            Value value = stack_pop;

            Variable *var = calloc(1, sizeof(Variable));
            var->name = name.val.str.chars;

            if (value.type == Value_Array && value.mutable == false) {
                value.val.array = dup_array(value.val.array);
                value.mutable = true;
            }
            var->value = value;

            bool valid = insert_entry(vm->vars, name.hash, Entry_Variable, var);
            if (!valid) ERR("ERROR in %s on line %ld: cant assign `%s` multiple times\n", get_loc, name.val.str.chars)

            pc += 2;
            dispatch();
        OP_SET_GLOBAL:{
            Value name = vm->func->constants.data[read_index];
            Value value = stack_pop;

            Variable *var = get_entry(vm->vars->entries, vm->vars->capacity, name.hash)->value;
            if (var == NULL) ERR("ERROR in %s on line %ld: tried to set nonexistent global %.*s\n", get_loc, Print(name.val.str))
            else if (var->value.type == Value_Array && var->value.mutable) {
                free_value_array(var->value.val.array);
            } else if (var->value.type == Value_String && var->value.mutable) {
                free(var->value.val.str.chars);
            }

            if (value.type == Value_Array && !value.mutable) {
                value.val.array = dup_array(value.val.array);
                value.mutable = true;
            }
            var->value = value;
            pc += 2;
            dispatch();}
        OP_GET_GLOBAL:{
            Value var_name = vm->func->constants.data[read_index];
            Variable *var = get_entry(vm->vars->entries, vm->vars->capacity, var_name.hash)->value;
            if (var == NULL) ERR("ERROR in %s on line %ld: tried to get nonexistent var %s\n", get_loc, var_name.val.str.chars);
            if (var->value.mutable) stack_push(((Value) {
                var->value.type, 
                .val.str=var->value.val.str,
                false,
                0
            }));
            else stack_push(var->value);
            pc += 2;
            dispatch();}
        OP_SET_LOCAL: {
            pc++;
            if (frame->slots[vm->func->code.data[pc]].type == Value_String && frame->slots[vm->func->code.data[pc]].mutable) free(frame->slots[vm->func->code.data[pc]].val.str.chars);
            else if (frame->slots[vm->func->code.data[pc]].type == Value_Array && frame->slots[vm->func->code.data[pc]].mutable) free_value_array(frame->slots[vm->func->code.data[pc]].val.array);

            Value val = stack_pop;
            if (val.type == Value_Array && val.mutable == false) {
                val.val.array = dup_array(val.val.array);
                val.mutable = true;
            }
            frame->slots[vm->func->code.data[pc]] = val;
            dispatch();}
        OP_GET_LOCAL:{
            pc++;
            Value val = frame->slots[vm->func->code.data[pc]];
            if (val.mutable) stack_push(((Value) {
                val.type, 
                .val.str=val.val.str,
                false,
                0
            }));
            else stack_push(val);
            dispatch();}
        OP_ADD:{
            Value op2 = stack_pop;
            Value op1 = stack_pop;

            if (op1.type == Value_Number && op2.type == Value_Number) {
                stack_push(((Value) {
                    Value_Number,
                    .val.num=(op1.val.num + op2.val.num),
                    false,
                    0
                }));
            } else if (op1.type == Value_Array && op2.type == Value_Array) {
                u_int32_t arr1_len = ARRAY_LEN(op1.val.array[0].val.num);
                u_int32_t arr2_len = ARRAY_LEN(op2.val.array[0].val.num);

                u_int32_t len = arr1_len + arr2_len - 1;
                u_int32_t size = next_power_of_two(len);

                Value *array = calloc(size, sizeof(Value));

                for (u_int32_t i = 1; i < arr1_len; i++) {
                    array[i] = dup_value(op1.val.array[i]);
                }

                for (u_int32_t i = 1; i < arr2_len; i++) {
                    array[i + arr1_len - 1] = dup_value(op2.val.array[i]);
                }

                array[0].type = Value_Array;
                array[0].val.num = MAKE_ARRAY_INFO(size, len);

                stack_push(((Value){
                    Value_Array,
                    .val.array=array,
                    true,
                    0
                }));
                if (op1.mutable) free_value_array(op1.val.array);
                if (op2.mutable) free_value_array(op2.val.array);
            } else if (op1.type == Value_String && op2.type == Value_String) {
                stack_push(((Value){
                    Value_String,
                    .val.str={format_str(op1.val.str.len + op2.val.str.len + 1, "%s%s", op1.val.str.chars, op2.val.str.chars), op1.val.str.len + op2.val.str.len},
                    true,
                    0
                }));

                if (op1.mutable) free(op1.val.str.chars);
                if (op2.mutable) free(op2.val.str.chars);
            } else ERR("ERROR in %s on line %ld: cant add type %s and %s\n", get_loc, find_value_type(op1.type), find_value_type(op2.type))
            dispatch();}
        OP_SUBTRACT:
            binary_op(-, "ERROR in %s on line %ld: cant subtract type %s and %s\n");
        OP_MULTIPLY:
            binary_op(*, "ERROR in %s on line %ld: cant multiply type %s and %s\n");
        OP_DIVIDE:
            binary_op(/, "ERROR in %s on line %ld: cant divide type %s and %s\n");
        OP_MODULO:
            binary_op(%, "ERROR in %s on line %ld: cant modulo type %s and %s\n");
        OP_BIT_AND:
            binary_op(&, "ERROR in %s on line %ld: cant bitwise and type %s and %s\n");
        OP_BIT_OR:
            binary_op(|, "ERROR in %s on line %ld: cant bitwise or type %s and %s\n");
        OP_BIT_XOR:
            binary_op(^, "ERROR in %s on line %ld: cant bitwise xor type %s and %s\n");
        OP_BIT_NOT:
            unary_op(~, "ERROR in %s on line %ld: cant bitwise not type %s\n");
        OP_LSHIFT:
            binary_op(<<, "ERROR in %s on line %ld: cant lshift type %s and %s\n");
        OP_RSHIFT:
            binary_op(>>, "ERROR in %s on line %ld: cant rshift type %s and %s\n");
        OP_POWER:{
            Value op2 = stack_pop;
            Value op1 = stack_pop;
            ASSERT((op1.type == Value_Number) && (op2.type == Value_Number), "cant exponentiate type %s and type %s\n", find_value_type(op1.type), find_value_type(op2.type))
            stack_push(((Value) {
                Value_Number,
                .val.num=(exponentiate(op1.val.num, op2.val.num)),
                false,
                0 
            }));
            dispatch();}
        OP_LESS:
            binary_op(<, "ERROR in %s on line %ld: cant less than type %s and %s\n");
        OP_LESS_EQUAL:
            binary_op(<=, "ERROR in %s on line %ld: cant less equal than type %s and %s\n");
        OP_GREATER:
            binary_op(>, "ERROR in %s on line %ld: cant greater than type %s and %s\n");
        OP_GREATER_EQUAL:
            binary_op(>=, "ERROR in %s on line %ld: cant greater equal than type %s and %s\n");
        OP_IS_EQUAL: {
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
                if (op1.val.str.len != op2.val.str.len) {
                    stack_push(((Value){
                        Value_Number,
                        .val.num=0,
                        false,
                        0
                    }));
                } else {
                    stack_push(((Value){
                        Value_Number,
                        .val.num=!strcmp(op1.val.str.chars, op2.val.str.chars),
                        false,
                        0
                    }));
                }
            } else stack_push(((Value){
                    Value_Number,
                    .val.num=0,
                    false,
                    0
                }));
            if (op1.type == Value_String && op1.mutable) free(op1.val.str.chars);
            if (op2.type == Value_String && op2.mutable) free(op2.val.str.chars);
            dispatch();}
        OP_AND:
            binary_op(&&, "ERROR in %s on line %ld: cant logical and type %s and %s\n");
        OP_OR:
            binary_op(||, "ERROR in %s on line %ld: cant logical or type %s and %s\n");
        OP_NOT:;
            unary_op(!, "ERROR in %s on line %ld: cant logical not type %s\n");
        OP_NOT_EQUAL:{
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
                    .val.num=!!strcmp(op1.val.str.chars, op2.val.str.chars),
                    false,
                    0
                }));
            } else stack_push(((Value){
                    Value_Number,
                    .val.num=0,
                    false,
                    0
                }));
            if (op1.type == Value_String && op1.mutable) free(op1.val.str.chars);
            if (op2.type == Value_String && op2.mutable) free(op2.val.str.chars);
            dispatch();}
        OP_JUMP_IF_FALSE:
        OP_START_IF:{
            Value val = stack_pop;
            if (!val.val.num) {
                pc += read_index - 1;
            } else {
                pc += 2;
            }
            dispatch();}
        OP_JUMP:
            pc = read_index - 1;
            dispatch();
        OP_GET_ELEMENT:{
            Value array = stack_pop;
            Value index = stack_pop;

            if (array.type == Value_Array) {
                if (index.val.num >= ARRAY_LEN(array.val.array[0].val.num)) ERR("ERROR in %s on line %ld: index %ld out of bounds, greater than %d\n", get_loc, index.val.num, ARRAY_LEN(array.val.array[0].val.num) - 1)
                else if (index.val.num < 1) ERR("ERROR in %s on line %ld: tried to access at index less than 1: %ld\n", get_loc, index.val.num)

                if (array.val.array[index.val.num].type == Value_Identifier) {
                    stack_push(array);
                } else if (array.val.array[index.val.num].type == Value_String && array.val.array[index.val.num].mutable) stack_push(((Value) {
                    Value_String, 
                    .val.str=array.val.array[index.val.num].val.str,
                    false,
                    0
                }));
                else stack_push(array.val.array[index.val.num]);
            } else if (array.type == Value_String) {
                if (index.val.num > (int64_t)array.val.str.len) ERR("ERROR in %s on line %ld: index %ld out of bounds for `%.*s`\n", get_loc, index.val.num, Print(array.val.str))
                else if (index.val.num < 1) ERR("ERROR in %s on line %ld: tried to access `%.*s` at index less than 1\n", get_loc, Print(array.val.str))

                stack_push(((Value) {
                    Value_String,
                    .val.str={&chars[(array.val.str.chars[index.val.num - 1] - 32) * 2], 1},
                    false,
                    0
                }));                  
            } else ERR("ERROR in %s on line %ld: cant get element of type %s\n", get_loc, find_value_type(array.type))

            dispatch();}
        OP_SET_ELEMENT_GLOBAL:{
            Value new_val = stack_pop;
            Value index = stack_pop;

            Value var_name = vm->func->constants.data[read_index];
            Variable *var = get_entry(vm->vars->entries, vm->vars->capacity, var_name.hash)->value;
            if (!var) ERR("ERROR in %s on line %ld: tried to set element of nonexistent array %.*s\n", get_loc, Print(var_name.val.str))

            Value *array = &var->value;
            if (!array->mutable && array->type == Value_Array) {
                array->val.array = dup_array(array->val.array);
                array->mutable = true;
            }

            if (array->type == Value_String) {
                if (index.val.num > (int64_t)array->val.str.len) ERR("ERROR in %s on line %ld: index %ld out of bounds for `%.*s`\n", get_loc, index.val.num, Print(array->val.str))
                else if (index.val.num < 1) ERR("ERROR in %s on line %ld: tried to access `%.*s` at index less than 1\n", get_loc, Print(array->val.str))

                array->val.str.chars[index.val.num - 1] = new_val.val.str.chars[0];
            } else if (array->type == Value_Array) {
                if (index.val.num >= ARRAY_LEN(array->val.array[0].val.num)) ERR("ERROR in %s on line %ld: index %ld out of bounds\n", get_loc, index.val.num)
                else if (index.val.num < 1) ERR("ERROR in %s on line %ld: tried to access at index less than 1\n", get_loc)

                if (array->val.array[index.val.num].type == Value_String && array->val.array[index.val.num].mutable) free(array->val.array[index.val.num].val.str.chars);
                array->val.array[index.val.num] = new_val;
            } else ERR("ERROR in %s on line %ld: cant set element to type %s\n", get_loc, find_value_type(array->type))

            pc += 2;
            dispatch();}
        OP_SET_ELEMENT_LOCAL:{
            Value new_val = stack_pop;
            Value index = stack_pop;

            Value *array = &frame->slots[vm->func->code.data[pc + 1]];
            if (!array->mutable && array->type == Value_Array) {
                array->val.array = dup_array(array->val.array);
                array->mutable = true;
            }

            if (array->type == Value_String) {
                if (index.val.num > (int64_t)array->val.str.len) ERR("ERROR in %s on line %ld: index %ld out of bounds for `%.*s`\n", get_loc, index.val.num, Print(array->val.str))
                else if (index.val.num < 1) ERR("ERROR in %s on line %ld: tried to access `%.*s` at index less than 1\n", get_loc, Print(array->val.str))

                array->val.str.chars[index.val.num - 1] = new_val.val.str.chars[0];
            } else if (array->type == Value_Array) {
                if (index.val.num >= ARRAY_LEN(array->val.array[0].val.num)) ERR("ERROR in %s on line %ld: index %ld out of bounds\n", get_loc, index.val.num)
                else if (index.val.num < 1) ERR("ERROR in %s on line %ld: tried to access at index less than 1\n", get_loc)

                if (array->val.array[index.val.num].type == Value_String && array->val.array[index.val.num].mutable) free(array->val.array[index.val.num].val.str.chars);
                array->val.array[index.val.num] = new_val;
            } else ERR("ERROR in %s on line %ld: cant set element to type %s\n", get_loc, find_value_type(array->type))
            
            pc++;
            dispatch();}
        OP_APPEND_GLOBAL:{
            Value val = stack_pop;
            Value appendee = stack_pop;

            Value *array = appendee.val.array;

            u_int32_t len = ARRAY_LEN(array[0].val.num);
            u_int32_t size = ARRAY_SIZE(array[0].val.num);

            if (len >= size) {
                size *= 2;
                array = realloc(array, size * sizeof(Value));
            }

            array[len] = val;

            array[0].val.num = MAKE_ARRAY_INFO(size, (len + 1));

            appendee.val.array = array;
            appendee.mutable = true;

            Value name = vm->func->constants.data[read_index];

            Variable *var = get_entry(vm->vars->entries, vm->vars->capacity, name.hash)->value;
            if (var == NULL) ERR("ERROR in %s on line %ld: tried to get append to nonexistent variable %.*s\n", get_loc, Print(name.val.str));

            var->value = appendee;

            pc += 2;
            dispatch();}
        OP_APPEND_LOCAL:{
            Value val = stack_pop;
            Value appendee = stack_pop;

            Value *array = appendee.val.array;

            u_int32_t len = ARRAY_LEN(array[0].val.num);
            u_int32_t size = ARRAY_SIZE(array[0].val.num);

            if (len >= size) {
                size *= 2;
                array = realloc(array, size * sizeof(Value));
            }

            array[len] = val;

            array[0].val.num = MAKE_ARRAY_INFO(size, (len + 1));

            appendee.val.array = array;
            appendee.mutable = true;
            
            frame->slots[vm->func->code.data[++pc]] = appendee;

            dispatch();}
        OP_POP:
            for (Value *val = vm->stack_top; val > frame->slots; val--) 
                if (val->type == Value_Array) {
                    Value *array = val->val.array;
                    u_int32_t len = ARRAY_LEN(array[0].val.num);
                    for (u_int32_t j = 1; j < len; j++) {
                        if (array[j].type == Value_String && array[j].mutable)
                            free(array[j].val.str.chars);
                    }
                    free(array);
                    val->mutable = false;
                }

            pc += 2;
            dispatch();
        OP_CAST_STR:{
            Value val = stack_pop;
            if (val.type == Value_String) stack_push(val);
            else if (val.type == Value_Number) {
                int len = num_len(val.val.num);
                stack_push(((Value) {
                    Value_String, 
                    .val.str={format_str(len + 1, "%ld", val.val.num), len},
                    true,
                    0
                }));
            } else ERR("ERROR in %s on line %ld: cant cast type %s as string\n", get_loc, find_value_type(val.type))
            dispatch();}
        OP_CAST_NUM:{
            Value val = stack_pop;
            if (val.type == Value_Number) stack_push(val);
            else if (val.type == Value_String) {
                stack_push(((Value) {
                    Value_Number,
                    .val.num=strtoint(val.val.str.chars, val.val.str.len),
                    false,
                    0
                }));
                if (val.mutable) free(val.val.str.chars);
            } else ERR("ERROR in %s on line %ld: cant cast type %s as number\n", get_loc, find_value_type(val.type))
            dispatch();}
        OP_CALL:{
            vm->call_stack[vm->call_stack_count - 1].return_index = pc + 2;
            if (vm->call_stack_count >= CALL_STACK_SIZE) ERR("ERROR in %s on line %ld: call stack overflow\n", get_loc)
            frame = &vm->call_stack[vm->call_stack_count++];
            frame->func = &vm->funcs.data[read_index];
            frame->slots = vm->stack_top - frame->func->arity;
            frame->loc = vm->func->locs.data[pc];
            #ifdef PROFILE
            clock_gettime(CLOCK_MONOTONIC, &tend);
            time_profiler[vm->call_stack[vm->call_stack_count - 2].index]
                += ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - 
                   ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec);
            clock_gettime(CLOCK_MONOTONIC, &tstart);

            frame->index = read_index;
            #endif
            vm->func = &vm->funcs.data[read_index];
            pc = -1;
            dispatch();}
        OP_CALL_NATIVE:{
            Native native = natives[read_index];
            Value result = native(vm->stack_top - native_arities[read_index], make_loc(get_loc));
            vm->stack_top -= native_arities[read_index];
            stack_push(result);
            pc += 2;
            dispatch();}
        OP_RETURN:{
            #ifdef PROFILE
            clock_gettime(CLOCK_MONOTONIC, &tend);
            time_profiler[vm->call_stack[vm->call_stack_count - 1].index]
                += ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - 
                   ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec);
            clock_gettime(CLOCK_MONOTONIC, &tstart);
            #endif

            Value result = stack_pop;
            if ((result.type == Value_Array) && !result.mutable) {
                result.val.array = dup_array(result.val.array);
                result.mutable = true;
            } else if (result.type == Value_String && !result.mutable) {
                result.val.str.chars = strdup(result.val.str.chars);
                result.mutable = true;
            }

            for (Value *val = vm->stack_top - 1; val >= frame->slots; val--) {
                if (val->mutable) {
                    val->mutable = false;
                    if (val->type == Value_Array)
                        free_value_array(val->val.array);
                    else if (val->type == Value_String)
                        free(val->val.str.chars);
                    else
                        ERR("ERROR in %s on line %ld: cant handle mutable function arg type %s\n", get_loc, find_value_type(val->type));
                }
            }

            vm->call_stack_count--;
            vm->stack_top = frame->slots;
            frame = &vm->call_stack[vm->call_stack_count - 1];
            vm->func = frame->func;
            pc = frame->return_index;

            stack_push(result);
            dispatch();}
        OP_RETURN_NOTHING:{
            #ifdef PROFILE
            clock_gettime(CLOCK_MONOTONIC, &tend);
            time_profiler[vm->call_stack[vm->call_stack_count - 1].index]
                += ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - 
                   ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec);
            clock_gettime(CLOCK_MONOTONIC, &tstart);
            #endif

            for (Value *val = vm->stack_top - 1; val >= frame->slots; val--) {
                if (val->mutable) {
                    val->mutable = false;
                    if (val->type == Value_Array)
                        free_value_array(val->val.array);
                    else if (val->type == Value_String)
                        free(val->val.str.chars);
                    else
                        ERR("ERROR in %s on line %ld: cant handle mutable function arg type %s\n", get_loc, find_value_type(val->type));
                }
            }

            vm->call_stack_count--;
            vm->stack_top = frame->slots;
            frame = &vm->call_stack[vm->call_stack_count - 1];
            vm->func = frame->func;
            pc = frame->return_index;
            dispatch();}
        OP_BREAK:{
            pc += read_index - 1;
            dispatch();}
        OP_CONTINUE:{
            pc = read_index - 1;
            dispatch();}
        OP_FOR:{
            Value array = stack_pop;
            Value *index = &vm->func->constants.data[read_index];
            pc += 2;
            Value *local = &frame->slots[vm->func->code.data[++pc]];
            if (array.type == Value_Array) {
                u_int32_t len = ARRAY_LEN(array.val.array[0].val.num);
                if (len < 2) {pc -= 3; ERR("ERROR in %s on line %ld: tried to iterate through array with no elements\n", get_loc)}
                if (index->val.num == 1) {
                    stack_push(array.val.array[1]);
                } else if (index->val.num >= len) {
                    index->val.num = 0;
                    pc = read_index + 3;
                    vm->stack_top--;
                } else {
                    *local = array.val.array[index->val.num];
                }
            } else if (array.type == Value_String) {
                if (index->val.num == 1) {
                    stack_push(((Value){
                        Value_String,
                        .val.str={&chars[(array.val.str.chars[index->val.num - 1] - 32) * 2], 1},
                        false,
                        0
                    }));
                } else if (index->val.num >= (int64_t)array.val.str.len + 1) {
                    index->val.num = 0;
                    pc = read_index + 3;
                    vm->stack_top--;
                } else {
                    *local = (Value){
                        Value_String,
                        .val.str={&chars[(array.val.str.chars[index->val.num - 1] - 32) * 2], 1},
                        false,
                        0
                    };
                }
            } else {
                pc -= 3;
                ERR("ERROR in %s on line %ld: cant loop through type %s\n", get_loc, find_value_type(array.type))
            } 
            index->val.num++;
            pc += 2;
            dispatch();}
        OP_ENUMERATE:{
            Value array = stack_pop;
            Value *index = &vm->func->constants.data[read_index];
            pc += 2;
            Value *index_var = &frame->slots[vm->func->code.data[++pc]];
            Value *local = &frame->slots[vm->func->code.data[++pc]];
            
            if (index->val.num > 1)
                if (index_var->val.num != (index->val.num - 1))
                    index->val.num = index_var->val.num + 1;

            if (array.type == Value_Array) {
                u_int32_t len = ARRAY_LEN(array.val.array[0].val.num);
                if (len < 2) {pc -= 3; ERR("ERROR in %s on line %ld: tried to iterate through array with no elements\n", get_loc)}
                if (index->val.num == 1) {
                    stack_push(((Value){
                        Value_Number,
                        .val.num=1,
                        false,
                        0
                    }));
                    stack_push(array.val.array[1]);
                } else if (index->val.num >= len) {
                    index->val.num = 0;
                    pc = read_index + 3;
                    vm->stack_top -= 2;
                } else {
                    index_var->val.num = index->val.num;
                    *local = array.val.array[index->val.num];
                }
            } else if (array.type == Value_String) {
                if (index->val.num == 1) {
                    stack_push(((Value){
                        Value_Number,
                        .val.num=1,
                        false,
                        0
                    }));
                    stack_push(((Value){
                        Value_String,
                        .val.str={&chars[(array.val.str.chars[index->val.num - 1] - 32) * 2], 1},
                        false,
                        0
                    }));
                } else if (index->val.num >= (int64_t)array.val.str.len + 1) {
                    index->val.num = 0;
                    pc = read_index + 3;
                    vm->stack_top -= 2;
                } else {
                    index_var->val.num = index->val.num;
                    *local = (Value){
                        Value_String,
                        .val.str={&chars[(array.val.str.chars[index->val.num - 1] - 32) * 2], 1},
                        false,
                        0
                    };
                }
            } else {
                pc -= 3;
                ERR("ERROR in %s on line %ld: cant loop through type %s\n", get_loc, find_value_type(array.type))
            } 
            index->val.num++;
            pc += 2;
            dispatch();}
        OP_GET_GLOBAL_GET_CONSTANT:{
            Value var_name = vm->func->constants.data[read_index];
            Variable *var = get_entry(vm->vars->entries, vm->vars->capacity, var_name.hash)->value;
            if (var == NULL) ERR("ERROR in %s on line %ld: tried to get nonexistent var %s\n", get_loc, var_name.val.str.chars);
            if (var->value.mutable) stack_push(((Value) {
                var->value.type, 
                .val.str=var->value.val.str,
                false,
                0
            }));
            else stack_push(var->value);
            pc += 2;
            u_int16_t index = read_index;
            stack_push(vm->func->constants.data[index]);
            pc += 2;
            dispatch();}
        OP_GET_LOCAL_GET_CONSTANT:{
            pc++;
            Value val = frame->slots[vm->func->code.data[pc]];
            if (val.mutable) stack_push(((Value) {
                val.type, 
                .val.str=val.val.str,
                false,
                0
            }));
            else stack_push(val);
            u_int16_t index = read_index;
            stack_push(vm->func->constants.data[index]);
            pc += 2;
            dispatch();}
        OP_DONE:
            return;
    }
}

#undef read_index
