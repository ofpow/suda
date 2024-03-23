#pragma once

#define COMBYTE(_byte1, _byte2) (((_byte1) << 8) | (_byte2))
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

typedef struct Call_Frame {
    Function *func;
    Value *slots;
    int64_t return_index;
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
    for (int j = 0; j < vm->funcs.index; j++) {
        if (j > 0) printf("\n");
        printf("Disassembly of function %s:\n", vm->funcs.data[j].name);
        for (int i = 0; i < vm->funcs.data[j].code.index; i++) {
            switch(vm->funcs.data[j].code.data[i]) {
                case OP_CONSTANT:
                    printf("%-6d OP_CONSTANT:      index %d\n", i, COMBYTE(vm->funcs.data[j].code.data[i + 1], vm->funcs.data[j].code.data[i + 2]));
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
                    printf("%-6d OP_CALL\n", i);
                    i += 2;
                    break;
                case OP_RETURN:
                    printf("%-6d OP_RETURN\n", i);
                    break;
                default:
                    ERR("ERROR in %s on line %ld: cant disassemble op type %d\n", get_loc,  vm->funcs.data[j].code.data[i]);
            }
        }
    }
}

void run(VM *vm) {
    for (int i = 0; i < vm->func->code.index; i++) {
        Call_Frame *frame = &vm->call_stack[vm->call_stack_count - 1];
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
                vm->call_stack[vm->call_stack_count - 1].return_index = i + 2;
                Call_Frame *frame = &vm->call_stack[vm->call_stack_count++];
                frame->func = &vm->funcs.data[read_index];
                frame->slots = vm->stack_top - frame->func->arity;
                vm->func = &vm->funcs.data[read_index];
                i = -1;
                break;}
            case OP_RETURN:{
                Value result = stack_pop;
                vm->call_stack_count--;
                Call_Frame *frame = &vm->call_stack[vm->call_stack_count - 1];
                vm->func = frame->func;
                i = frame->return_index;
                stack_push(result);
                break;}
            default: ERR("ERROR in %s on line %ld: cant do %s\n", get_loc, find_op_code(vm->func->code.data[i]))
        }
    }
}
