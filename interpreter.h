#pragma once

#define AST_IS_EVALUATABLE(type) ((type == AST_Literal || IS_AST_MATH_OP(type) || type == AST_Identifier || type == AST_At || type == AST_Function || type == AST_Function_Call || type == AST_Len || type == AST_Cast_Num || type == AST_Cast_Str || type == AST_Array))

int exponentiate(int base, int64_t power) {
    int64_t result = 1;

    for (int i = 0; i < power; i++) {
        result *= base;
    }

    return result;
}

typedef struct {
    Node **nodes;
    int64_t stmts_capacity;
    
    int64_t program_counter;

    Variable *vars;
    int64_t vars_index;
    int64_t vars_capacity;

    Variable *local_vars;
    int64_t local_vars_index;
    int64_t local_vars_capacity;

    Function **funcs;
    int64_t funcs_capacity;

    int64_t auto_jump;
} Interpreter;

AST_Type value_to_ast_type(Value_Type type, int64_t line, const char *file) {
    if (type == Value_Number || type == Value_String) return AST_Literal;
    else if (type == Value_Identifier) return AST_Identifier;
    else if (type == Value_Array) return AST_Array;
    else ERR("ERROR in %s on line %ld: value type %d has not AST type\n", file, line, type);
    return -1;
}

AST_Value *do_statement(Node *n, Interpreter *interpreter);

char *format_array(AST_Value *array) {
    int64_t arr_size = NUM(array->value);
    if (arr_size < 2) return format_str(3, "[]");
    int64_t str_len = 2;
    char *array_str = format_str(str_len, "[");
    for (int i = 1; i < arr_size; i++) {
        if (array[i].value == NULL) ERR("array has null value at index %d\n", i)
        if (array[i].type == Value_Number) {
            int len = num_len(NUM(array[i].value)) + 1;
            char *num = format_str(len, "%ld", NUM(array[i].value));
            str_len += (len + 1);
            array_str = realloc(array_str, str_len);
            strcat(array_str, num);
            strcat(array_str, ", ");
            free(num);
        }
        else {
            str_len += (strlen(array[i].value) + 2);
            array_str = realloc(array_str, str_len);
            strcat(array_str, array[i].value);
            strcat(array_str, ", ");
        }
    }
    array_str[str_len - 3] = ']';
    array_str[str_len - 2] = 0;
    return array_str;
}

AST_Value *eval_node(Node *n, Interpreter *interpreter, int64_t mutable);

AST_Value *call_function(Interpreter *interpreter, Node *n) {
    Function *func = get_func(interpreter->funcs, interpreter->funcs_capacity, n->value->value, n->line);

    char *call_info = format_str(strlen(func->name) + 3 + num_len(n->line), "%s:%ld", func->name, n->line);
    append(call_stack, call_info, call_stack_index, call_stack_capacity)

    ASSERT((func->arity == n->func_args_index), "ERROR in %s on line %ld: cant call function %s with %ld arguments, it needs %ld arguments\n", n->file, n->line, func->name, n->func_args_index, func->arity)

    Interpreter intrprtr = {
        // nodes to execute
        func->nodes,
        func->nodes_size,
        0,

        //global variables inherited from call
        interpreter->vars,
        interpreter->vars_index,
        interpreter->vars_capacity,
        
        // arguments and local variables
        calloc(func->arity + 1, sizeof(Variable)),
        0,
        func->arity + 1,

        // other functions
        interpreter->funcs,
        interpreter->funcs_capacity,
        0,
    };

    for (int i = 0; i < func->arity; i++) {
        intrprtr.local_vars[intrprtr.local_vars_index] = (Variable) {
            func->args[i]->value,
            eval_node(n->func_args[i], interpreter, 1),
            i,
        };
        intrprtr.local_vars_index++;
    }

    AST_Value *rtrn;
    while (intrprtr.program_counter < intrprtr.stmts_capacity) {
        rtrn = do_statement(intrprtr.nodes[intrprtr.program_counter], &intrprtr);
        if (rtrn != NULL) {
            for (int i = 0; i < intrprtr.local_vars_index; i++) free_ast_value(intrprtr.local_vars[i].value);
            free(intrprtr.local_vars);
            return rtrn;
        }
        intrprtr.program_counter++;
    }
    for (int i = 0; i < intrprtr.local_vars_index; i++) free_ast_value(intrprtr.local_vars[i].value);
    free(intrprtr.local_vars);
    call_stack_index--;
    //free(call_info);
    return NULL;
}
// TODO: use something like this for appending to array instead of append keyword
AST_Value *add_array(AST_Value *op1, AST_Value *op2) {
    int64_t op1_len = NUM(op1->value);
    int64_t op2_len = NUM(op2->value);
    AST_Value *new = calloc(op1_len + op2_len + 1, sizeof(AST_Value));
    new->type = Value_Array;
    new->value = dup_int(op1_len + op2_len - 1);
    new->mutable = 1;
    for (int i = 1; i < op1_len; i++) {
        new[i] = (AST_Value) { op1[i].type, strdup(op1[i].value), 1 };
    }
    for (int i = 1; i < op2_len; i++) {
        new[i + op1_len - 1] = (AST_Value) { op2[i].type, strdup(op2[i].value), 1 };
    }
    return new;
}

void assign_variable(Interpreter *interpreter, char *var_name, AST_Value *var_val, int64_t line, const char *file) {
    if (check_variable(var_name, interpreter->vars, interpreter->vars_index) >= 0) 
        ERR("ERROR in %s on line %ld: cant assign `%s` multiple times\n", file, line, var_name)
    else if (check_variable(var_name, interpreter->local_vars, interpreter->local_vars_index) >= 0) 
        ERR("ERROR in %s on line %ld: cant assign `%s` multiple times\n", file, line, var_name)

    if (interpreter->local_vars != NULL) {
        if (interpreter->local_vars_index >= interpreter->local_vars_capacity) {
            interpreter->local_vars_capacity *= 2;
            interpreter->local_vars = realloc(interpreter->local_vars, sizeof(Variable) * interpreter->local_vars_capacity);
        }
        interpreter->local_vars[interpreter->local_vars_index] = (Variable) { var_name, var_val, interpreter->local_vars_index };
        interpreter->local_vars_index++;
    } else {
        if (interpreter->vars_index >= interpreter->vars_capacity) {
            interpreter->vars_capacity *= 2;
            interpreter->vars = realloc(interpreter->vars, sizeof(Variable) * interpreter->vars_capacity);
        }

        interpreter->vars[interpreter->vars_index] = (Variable) { var_name, var_val, interpreter->vars_index };
        interpreter->vars_index++;
    }
}

void reassign_variable(Interpreter *interpreter, char *var_name, AST_Value *new_val, int64_t line, const char *file) {
    Variable var;
    int64_t var_index;
    var_index = check_variable(var_name, interpreter->local_vars, interpreter->local_vars_index);
    if (var_index >= 0) var = interpreter->local_vars[var_index];
    else {
        var_index = check_variable(var_name, interpreter->vars, interpreter->vars_index);
        if (var_index >= 0) var = interpreter->vars[var_index];
        else ERR("ERROR in %s on line %ld: can't assign to undefined variable %s\n", file, line, var_name)
    }

    
    if (interpreter->local_vars != NULL) {
        debug("REASSIGN variable `%s` from `%s` to `%s`\n", interpreter->local_vars[var.index].name, interpreter->local_vars[var.index].value->value, new_val->value)
        free_ast_value(interpreter->local_vars[var.index].value);
        interpreter->local_vars[var.index].value = new_val;
    } else {
        debug("REASSIGN variable `%s` from `%s` to `%s`\n", interpreter->vars[var.index].name, interpreter->vars[var.index].value->value, new_val->value)
        free_ast_value(interpreter->vars[var.index].value);
        interpreter->vars[var.index].value = new_val;
    }
}

void unassign_variable(Interpreter *interpreter, char *var_name, int64_t line, const char *file) {
    Variable var;
    int64_t var_index;
    var_index = check_variable(var_name, interpreter->local_vars, interpreter->local_vars_index);
    if (var_index >= 0) var = interpreter->local_vars[var_index];
    else {
        var_index = check_variable(var_name, interpreter->vars, interpreter->vars_index);
        if (var_index >= 0) var = interpreter->vars[var_index];
        else ERR("ERROR in %s on line %ld: can't unassign to undefined variable %s\n", file, line, var_name)
    }

    if (interpreter->local_vars != NULL) {
        debug("UNASSIGN variable `%s`\n", interpreter->local_vars[var.index].name)
        free_ast_value(interpreter->local_vars[var.index].value);
        interpreter->local_vars_index--;
    } else {
        debug("UNASSIGN variable `%s`\n", interpreter->vars[var.index].name)
        free_ast_value(interpreter->vars[var.index].value);
        interpreter->vars_index--;
    }
}

AST_Value *ast_math(AST_Value *op1, AST_Value *op2, int64_t op, int64_t line, const char *file) {
    ASSERT((op1 != NULL), "ERROR in %s on line %ld: cant do math with a null op\n", file, line)
    int64_t op1_len = strlen(op1->value);
    int64_t op2_len;
    if (op2 != NULL) op2_len = strlen(op2->value);
    switch (op) {
        case AST_Add:
            if (op1->type == Value_Number && op2->type == Value_Number) return new_ast_value(Value_Number, dup_int(NUM(op1->value) + NUM(op2->value)), 1);
            else if (op1->type == Value_Array && op2->type == Value_Array) return add_array(op1, op2);
            else {
                char *val1;
                char *val2;
                if (op1->type == Value_Number) {
                    val1 = format_str(num_len(NUM(op1->value)) + 1, "%ld", NUM(op1->value));
                    op1_len = num_len(NUM(op1->value));
                } else
                    val1 = op1->value;

                if (op2->type == Value_Number) {
                    val2 = format_str(num_len(NUM(op2->value)) + 1, "%ld", NUM(op2->value));
                    op2_len = num_len(NUM(op2->value));
                } else
                    val2 = op2->value;
                AST_Value *rtrn = new_ast_value(Value_String, format_str(op1_len + op2_len + 1, "%s%s", val1, val2), 1);
                if (op1->type == Value_Number) free(val1);
                if (op2->type == Value_Number) free(val2);
                return rtrn;
            }
        case AST_Sub:
            ASSERT((op1->type == Value_Number && op2->type == Value_Number), "ERROR in %s on line %ld: Cant subtract type %s and type %s\n", file, line, find_ast_value_type(op1->type), find_ast_value_type(op2->type))
            return new_ast_value(Value_Number, dup_int(NUM(op1->value) - NUM(op2->value)), 1);
        case AST_Mult:
            ASSERT((op1->type == Value_Number && op2->type == Value_Number), "ERROR in %s on line %ld: Cant multiply type %s and type %s\n", file, line, find_ast_value_type(op1->type), find_ast_value_type(op2->type))
            return new_ast_value(Value_Number, dup_int(NUM(op1->value) * NUM(op2->value)), 1);
        case AST_Div:
            ASSERT((op1->type == Value_Number && op2->type == Value_Number), "ERROR in %s on line %ld: Cant divide type %s and type %s\n", file, line, find_ast_value_type(op1->type), find_ast_value_type(op2->type))
            return new_ast_value(Value_Number, dup_int(NUM(op1->value) / NUM(op2->value)), 1);
        case AST_Less:
            ASSERT((op1->type == Value_Number && op2->type == Value_Number), "ERROR in %s on line %ld: Cant less than type %s and type %s\n", file, line, find_ast_value_type(op1->type), find_ast_value_type(op2->type))
            return new_ast_value(Value_Number, dup_int(NUM(op1->value) < NUM(op2->value)), 1);
        case AST_Less_Equal:
            ASSERT((op1->type == Value_Number && op2->type == Value_Number), "ERROR in %s on line %ld: Cant less equal type %s and type %s\n", file, line, find_ast_value_type(op1->type), find_ast_value_type(op2->type))
            return new_ast_value(Value_Number, dup_int(NUM(op1->value) <= NUM(op2->value)), 1);
        case AST_Greater:
            ASSERT((op1->type == Value_Number && op2->type == Value_Number), "ERROR in %s on line %ld: Cant greater than type %s and type %s\n", file, line, find_ast_value_type(op1->type), find_ast_value_type(op2->type))
            return new_ast_value(Value_Number, dup_int(NUM(op1->value) > NUM(op2->value)), 1);
        case AST_Greater_Equal:
            ASSERT((op1->type == Value_Number && op2->type == Value_Number), "ERROR in %s on line %ld: Cant greater equal type %s and type %s\n", file, line, find_ast_value_type(op1->type), find_ast_value_type(op2->type))
            return new_ast_value(Value_Number, dup_int(NUM(op1->value) >= NUM(op2->value)), 1);
        case AST_Is_Equal:
            if (op1->type == Value_Number && op2->type == Value_Number)
                return new_ast_value(Value_Number, dup_int(NUM(op1->value) == NUM(op2->value)), 1);
            else if (op1->type == Value_String && op2->type == Value_String)
                return new_ast_value(Value_Number, dup_int(!strcmp(op1->value, op2->value)), 1);
            else
                return new_ast_value(Value_Number, dup_int(0), 1);
            break;
        case AST_And:
            ASSERT((op1->type == Value_Number && op2->type == Value_Number), "ERROR in %s on line %ld: Cant logical and type %s and type %s\n", file, line, find_ast_value_type(op1->type), find_ast_value_type(op2->type))
            return new_ast_value(Value_Number, dup_int(NUM(op1->value) && NUM(op2->value)), 1);
        case AST_Or:
            ASSERT((op1->type == Value_Number && op2->type == Value_Number), "ERROR in %s on line %ld: Cant logical or type %s and type %s\n", file, line, find_ast_value_type(op1->type), find_ast_value_type(op2->type))
            return new_ast_value(Value_Number, dup_int(NUM(op1->value) || NUM(op2->value)), 1);
        case AST_Not:
            return new_ast_value(Value_Number, dup_int(!(NUM(op1->value))), 1);
        case AST_Not_Equal:
            return new_ast_value(Value_Number, format_str(2, "%ld", !!strcmp(op1->value, op2->value)), 1);
        case AST_Modulo:
            ASSERT((op1->type == Value_Number && op2->type == Value_Number), "ERROR in %s on line %ld: Cant modulo type %s and type %s\n", file, line, find_ast_value_type(op1->type), find_ast_value_type(op2->type))
            return new_ast_value(Value_Number, dup_int(NUM(op1->value) % NUM(op2->value)), 1);
        case AST_Bit_Or:
            ASSERT((op1->type == Value_Number && op2->type == Value_Number), "ERROR in %s on line %ld: Cant bitwise or type %s and type %s\n", file, line, find_ast_value_type(op1->type), find_ast_value_type(op2->type))
            return new_ast_value(Value_Number, dup_int(NUM(op1->value) | NUM(op2->value)), 1);
        case AST_Bit_And:
            ASSERT((op1->type == Value_Number && op2->type == Value_Number), "ERROR in %s on line %ld: Cant bitwise and type %s and type %s\n", file, line, find_ast_value_type(op1->type), find_ast_value_type(op2->type))
            return new_ast_value(Value_Number, dup_int(NUM(op1->value) & NUM(op2->value)), 1);
        case AST_Bit_Xor:
            ASSERT((op1->type == Value_Number && op2->type == Value_Number), "ERROR in %s on line %ld: Cant bitwise xor type %s and type %s\n", file, line, find_ast_value_type(op1->type), find_ast_value_type(op2->type))
            return new_ast_value(Value_Number, dup_int(NUM(op1->value) ^ NUM(op2->value)), 1);
        case AST_Bit_Not:
            ASSERT((op1->type == Value_Number), "ERROR in %s on line %ld: Cant bitwise not type %s\n", file, line, find_ast_value_type(op1->type))
            return new_ast_value(Value_Number, dup_int(~(NUM(op1->value))), 1);
        case AST_Rshift:
            ASSERT((op1->type == Value_Number && op2->type == Value_Number), "ERROR in %s on line %ld: Cant rshift type %s and type %s\n", file, line, find_ast_value_type(op1->type), find_ast_value_type(op2->type))
            return new_ast_value(Value_Number, dup_int(NUM(op1->value) >> NUM(op2->value)), 1);
        case AST_Lshift:
            ASSERT((op1->type == Value_Number && op2->type == Value_Number), "ERROR in %s on line %ld: Cant lshift type %s and type %s\n", file, line, find_ast_value_type(op1->type), find_ast_value_type(op2->type))
            return new_ast_value(Value_Number, dup_int(NUM(op1->value) << NUM(op2->value)), 1);
        case AST_Power:
            ASSERT((op1->type == Value_Number && op2->type == Value_Number), "ERROR in %s on line %ld: Cant exponentiate type %s and type %s\n", file, line, find_ast_value_type(op1->type), find_ast_value_type(op2->type))
            return new_ast_value(Value_Number, dup_int(exponentiate(NUM(op1->value), NUM(op2->value))), 1);
        default:
            ERR("ERROR in %s on line %ld: unknown math op %ld\n", file, line, op)
    }
    return NULL;
}

AST_Value *eval_node(Node *n, Interpreter *interpreter, int64_t mutable) {
    if (n == NULL) return NULL;
    switch (n->type) {
        case AST_Literal:
            if (mutable <= 0) {
                AST_Value *new_val = n->value;
                new_val->mutable = 0;
                return new_val;
            }
            if (n->value->type == Value_Number)
                return new_ast_value(Value_Number, dup_int(NUM(n->value->value)), 1);
            return new_ast_value(n->value->type, strdup(n->value->value), 1);
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
        case AST_Not:
        case AST_Not_Equal:
        case AST_Modulo:
        case AST_Bit_And:
        case AST_Bit_Or:
        case AST_Bit_Xor:
        case AST_Bit_Not:
        case AST_Lshift:
        case AST_Rshift:
        case AST_Power:;
            AST_Value *op1 = eval_node(n->left, interpreter, 0);
            AST_Value *op2 = eval_node(n->right, interpreter, 0);
            AST_Value *result = ast_math(op1, op2, n->type, n->line, n->file);
            if (op1->mutable > 0) free_ast_value(op1);
            if (op2 && op2->mutable > 0) free_ast_value(op2);
            return result;
        case AST_Identifier: {
            Variable var;
            if (check_variable(n->value->value, interpreter->local_vars, interpreter->local_vars_index) >= 0) var = get_var(n->value->value, interpreter->local_vars, interpreter->local_vars_index, n->line);
            else var = get_var(n->value->value, interpreter->vars, interpreter->vars_index, n->line);
            if (var.value == NULL) return NULL;

            if (mutable <= 0) {
                AST_Value *new_val = var.value;
                new_val->mutable = 0;
                return new_val;
            }
            if (var.value->type == Value_Array) {
                int64_t arr_len = NUM(var.value->value);
                AST_Value *array = calloc(arr_len, sizeof(struct AST_Value));
                array->mutable = 1;
                array->type = Value_Array;
                array->value = dup_int(arr_len);
                for (int i = 1; i < arr_len; i++) {
                    array[i].type = var.value[i].type;
                    if (array[i].type == Value_Number) 
                        array[i].value = dup_int(NUM(var.value[i].value));
                    else
                        array[i].value = strdup(var.value[i].value);
                }
                return array;
            }
            if (var.value->type == Value_Number)
                return new_ast_value(Value_Number, dup_int(NUM(var.value->value)), 1);
            return new_ast_value(var.value->type, strdup(var.value->value), 1);
        }
        case AST_Array: {
            if (mutable <= 0) return n->value;
            int64_t arr_len = NUM(n->value->value);
            AST_Value *array = calloc(arr_len, sizeof(struct AST_Value));
            array->mutable = 1;
            array->type = Value_Array;
            array[0].value = dup_int(NUM(n->value->value));
            for (int i = 1; i < arr_len; i++) {
                array[i].type = n->value[i].type;
                if (n->value[i].type == Value_Number){
                    array[i].value = dup_int(NUM(n->value[i].value));
                } else {
                    array[i].value = strdup(n->value[i].value);
                }
            }
            return array;
        }
        case AST_At: {
            Variable var;
            int64_t index = 0;
            if (n->left->type != AST_Literal) {
                AST_Value *val = eval_node(n->left, interpreter, 0);
                index = NUM(val->value);
                if (val->mutable > 0) free_ast_value(val);
            } else index = NUM(n->left->value->value);
        
            if (check_variable(n->value->value, interpreter->local_vars, interpreter->local_vars_index) >= 0) var = get_var(n->value->value, interpreter->local_vars, interpreter->local_vars_index, n->line);
            else var = get_var(n->value->value, interpreter->vars, interpreter->vars_index, n->line);

            if (var.value->type == Value_String) {
                return new_ast_value(Value_String, format_str(2, "%c", STR(var.value->value)[index - 1]), 1);
            }
            int64_t arr_len = NUM(var.value[0].value);
            if (index >= arr_len) ERR("ERROR in %s on line %ld: Index %ld is out of bounds for array %s, length %ld\n", n->file, n->line, index, var.name, arr_len - 1)
            else if (index < 1) ERR("ERROR in %s on line %ld: invalid index %ld, it is less than 1\n", n->file, n->line, index)

            if (var.value[index].type == Value_String) {
                int64_t len = strlen(var.value[index].value);
                if (STR(var.value[index].value)[0] != '"') return new_ast_value(var.value[index].type, strdup(var.value[index].value), 1);
                else return new_ast_value(var.value[index].type, format_str(len - 1, "%.*s", len, STR(var.value[index].value) + 1), 1);
            } else if (var.value[index].type == Value_Number) {
                if (mutable <= 0) {
                    AST_Value *val = &var.value[index];
                    val->mutable = 0;
                    return val;
                }
                return new_ast_value(var.value[index].type, dup_int(NUM(var.value[index].value)), 1);
            } else if (var.value[index].type == Value_Identifier) {
                Variable rtrn;
                if (interpreter->local_vars != NULL) var = get_var(var.value[index].value, interpreter->local_vars, interpreter->local_vars_index, n->line);
                else rtrn = get_var(var.value[index].value, interpreter->vars, interpreter->vars_index, n->line);

                if (mutable <= 0) {
                    AST_Value *new_val = rtrn.value;
                    new_val->mutable = 0;
                    return new_val;
                }
                if (rtrn.value->type == Value_Array) {
                    int64_t arr_len = NUM(rtrn.value->value);
                    AST_Value *array = calloc(arr_len + 1, sizeof(rtrn.value[0]));
                    array->mutable = 1;
                    for (int i = 0; i < arr_len; i++) {
                        array[i].type = rtrn.value[i].type;
                        array[i].value = strdup(rtrn.value[i].value);
                    }
                    return array;
                }
                return new_ast_value(rtrn.value->type, strdup(rtrn.value->value), 1);
            } else ERR("ERROR in %s on line %ld: Can't evaluate %s as part of array\n", n->file, n->line, find_ast_value_type(var.value[index].type))
            break;
        }
        case AST_Function_Call:
            return call_function(interpreter, n);
        case AST_Len:
            ASSERT((n->left->value->type == Value_Array || n->left->value->type == Value_String || n->left->value->type == Value_Identifier || n->left->value->type == Value_Number), "ERROR in %s on line %ld: cant do len on value type %s\n", n->file, n->line, find_ast_value_type(n->left->value->type))
            AST_Value *op = eval_node(n->left, interpreter, 0);
            switch (op->type) {
                case Value_String:
                    return new_ast_value(Value_Number, dup_int(strlen(op->value)), 1);
                case Value_Array:;
                    return new_ast_value(Value_Number, dup_int(NUM(op[0].value) - 1), 1);
                case Value_Number:;
                    return new_ast_value(Value_Number, dup_int(num_len(NUM(op->value)) + 1), 1);
                default: ERR("ERROR in %s on line %ld: cant evaluate length of value type %s\n", n->file, n->line, find_ast_value_type(op->type))
            }
            break;
        case AST_Cast_Num: {
            AST_Value *val = eval_node(n->left, interpreter, mutable);
            if (val->type == Value_Number) return val;
            ASSERT((val->type == Value_String), "ERROR in %s on line %ld: cant cast type %s to number\n", n->file, n->line, find_ast_value_type(val->type))
            if (val->mutable > 0) {
                int64_t *x = dup_int(strtoint(STR(val->value), strlen(STR(val->value))));
                free(val->value);
                val->value = x;
                val->type = Value_Number;
                return val;
            } else
                return new_ast_value(Value_Number, dup_int(strtoint(STR(val->value), strlen(STR(val->value)))), 1);
        }
        case AST_Cast_Str: {
            AST_Value *val = eval_node(n->left, interpreter, mutable);
            if (val->type == Value_String) return val;
            ASSERT((val->type == Value_Number), "ERROR in %s on line %ld: cant cast type %s to string\n", n->file, n->line, find_ast_value_type(val->type))
            if (val->mutable > 0) {
                char *x = format_str(num_len(NUM(val->value)) + 1, "%ld", NUM(val->value));
                free(val->value);
                val->value = x;
                val->type = Value_String;
                return val;
            } else 
                return new_ast_value(Value_String, format_str(num_len(NUM(val->value)) + 1, "%ld", NUM(val->value)), 1);
        }
        default: ERR("ERROR in %s on line %ld: cant evaluate node type `%s`\n", n->file, n->line, find_ast_type(n->type))
    }
    return NULL;
}

AST_Value *do_statement(Node *n, Interpreter *interpreter) {
    switch (n->type) {
        case AST_Println:
        case AST_Print:;
            if (!n->left) ERR("ERROR in %s on line %ld: need something to print\n", n->file, n->line)
            ASSERT(AST_IS_EVALUATABLE(n->left->type), "Can't print `%s`\n", find_ast_type(n->left->type))
            AST_Value *print = eval_node(n->left, interpreter, 0);
            if (print == NULL) { printf("\n"); break; }

            if (print->type == Value_Array) {
                char *array = format_array(print);
                if (n->type == AST_Println) printf("%s\n", array);
                else printf("%s", array);
                if (print->mutable > 0) {
                    int64_t arr_len = NUM(print->value);
                    for (int j = 0; j < arr_len; j++) {
                        free(print[j].value);
                    }
                    free(print);
                }
                free(array);
            } else {
                if (print->type == Value_String) {
                    if (n->type == AST_Println) printf("%s\n", STR(print->value));
                    else printf("%s", STR(print->value));
                } else {
                    if (n->type == AST_Println) printf("%ld\n", NUM(print->value));
                    else printf("%ld", NUM(print->value));
                }
                if (print->mutable > 0) free_ast_value(print);
            }

            break;
        case AST_Var_Assign:;
            char *var_name = n->value->value;
            AST_Value *var_val = eval_node(n->left, interpreter, 1);
            assign_variable(interpreter, var_name, var_val, n->line, n->file);
            debug("ASSIGN `%s` to variable `%s`\n", var_val->value, var_name)
            break;
        case AST_If:;
            AST_Value *expr = eval_node(n->left, interpreter, 0);
            if (NUM(expr->value) == 0) {
                interpreter->program_counter = n->jump_index;
            } else {
                interpreter->auto_jump = 1;
            }
            if (expr->mutable > 0) free_ast_value(expr);
            break;
        case AST_Elif:;{
            if (interpreter->auto_jump == 1) {
                if (interpreter->nodes[n->jump_index]->type == AST_Else)
                    interpreter->program_counter = n->jump_index - 1;
                else
                    interpreter->program_counter = n->jump_index;
                break;
            }
            AST_Value *expr = eval_node(n->left, interpreter, 0);
            if (NUM(expr->value) == 0) {
                interpreter->program_counter = n->jump_index;
            } else {
                interpreter->auto_jump = 1;
            }
            if (expr->mutable > 0) free_ast_value(expr);
            break;}
        case AST_For:;
            debug("FOR: index is `%s`\n", n->value->value)
            AST_Value *list = eval_node(n->right, interpreter, 0);
            int64_t index = NUM(n->value->value);
           
            int64_t len;
            AST_Value *new_value;
            if (list->type == Value_Array) {
                len = NUM(list[0].value);
                if (index >= (len - 1)) {
                    //TODO: only works if iterator is last variable in list, ie no variables created inside loop
                    unassign_variable(interpreter, n->left->value->value, n->line, n->file);
                    n->value = new_ast_value(Value_Number, dup_int(0), 1);
                    interpreter->program_counter = n->jump_index;
                    break;
                }
                if (list[index + 1].type == Value_String) {
                    int64_t len = strlen(list[index + 1].value);
                    new_value = new_ast_value(list[index + 1].type, format_str(len, "%.*s", len - 2, ((char*)list[index + 1].value) + 1), 1);
                } else {
                    new_value = new_ast_value(list[index + 1].type, dup_int(NUM(list[index + 1].value)), 1);
                }
            } else if (list->type == Value_String) {
                len = strlen(list->value) + 1;
                if (index >= (len - 1)) {
                    unassign_variable(interpreter, n->left->value->value, n->line, n->file);
                    n->value = new_ast_value(Value_Number, format_str(2, "0"), 1);
                    interpreter->program_counter = n->jump_index;
                    break;
                }
                new_value = new_ast_value(Value_String, format_str(2, "%c", ((char*)list->value)[index]), 1);
            } else ERR("ERROR in %s on line %ld: cant iterate through type %s\n", n->file, n->line, find_ast_value_type(list->type))

            if (index < 1) {
                assign_variable(interpreter, n->left->value->value, new_value, n->line, n->file);
            }
            else if (index < (len - 1)) {
                reassign_variable(interpreter, n->left->value->value, new_value, n->line, n->file);
            }
            index++;
            free(n->value->value);
            n->value->value = dup_int(index);
            break;
        case AST_Else:
            interpreter->program_counter = n->jump_index;
            break;
        case AST_Semicolon:
            if (interpreter->nodes[n->jump_index]->type == AST_While || interpreter->nodes[n->jump_index]->type == AST_For) {
                interpreter->program_counter = n->jump_index - 1;
            }
            interpreter->auto_jump = 0;
            break;
        case AST_Identifier:;{
            AST_Value *new_val = eval_node(n->left, interpreter, 1);
            reassign_variable(interpreter, n->value->value, new_val, n->line, n->file);

            break;}
        case AST_While:;{
            AST_Value *expr = eval_node(n->left, interpreter, 0);
            if (NUM(expr->value) == 0) {
                interpreter->program_counter = n->jump_index;
            }
            if (expr->mutable > 0) free_ast_value(expr);
            break;}
        case AST_Break:
            interpreter->program_counter = n->jump_index;
            break;
        case AST_Continue:
            interpreter->program_counter = n->jump_index;
            break;
        case AST_At:;{
            AST_Value *new_val;
            int64_t index;
            if (n->right) {
                new_val = eval_node(n->right, interpreter, 1);
                AST_Value *i = eval_node(n->left, interpreter, 0);
                index = NUM(i->value);
                if (i->mutable > 0) free_ast_value(i);
            } else {
                new_val = eval_node(n->left->left, interpreter, 1);
                AST_Value *i = eval_node(n->left, interpreter, 1);
                index = NUM(i->value);
                if (i->mutable > 0) free_ast_value(i);
            }

            Variable var;
            if (check_variable(n->value->value, interpreter->local_vars, interpreter->local_vars_index) >= 0) var = get_var(n->value->value, interpreter->local_vars, interpreter->local_vars_index, n->line);
            else var = get_var(n->value->value, interpreter->vars, interpreter->vars_index, n->line);

            //if it was assigned after array was created, add quotes around value
            if (new_val->type == Value_String && ((char*)new_val->value)[0] != '"' && var.value->type == Value_Array) {
                char *temp = strdup(new_val->value);
                free(new_val->value);
                new_val->value = format_str((strlen(temp) + 3), "\"%s\"", temp);
                free(temp);
            }

            if (var.value->type == Value_String) {
                ((char*)var.value->value)[index - 1] = ((char*)new_val->value)[0];
                if (new_val->mutable > 0) free_ast_value(new_val);
                break;
            }
            free(var.value[index].value);
            if (interpreter->local_vars != NULL)
                interpreter->local_vars[var.index].value[index] = *new_val;
            else
                interpreter->vars[var.index].value[index] = *new_val;
            free(new_val);

            break;}
        case AST_Return:
            return eval_node(n->left, interpreter, 1);
        case AST_Function_Call:;
            AST_Value *result = call_function(interpreter, n);
            if (result == NULL) break;
            if (result->mutable > 0) free_ast_value(result);
            break;
        case AST_Exit:;
            AST_Value *exit_val = eval_node(n->left, interpreter, 0);
            ASSERT(exit_val->type == Value_Number, "ERROR in %s on line %ld: tried to exit with non-number code\n", n->file, n->line)
            int64_t val = NUM(exit_val->value);
            if (exit_val->mutable > 0) free_ast_value(exit_val);
            free_mem(val);
            break;
        case AST_Append:;
            Variable var;
            if (check_variable(n->value->value, interpreter->local_vars, interpreter->local_vars_index) >= 0) var = get_var(n->value->value, interpreter->local_vars, interpreter->local_vars_index, n->line);
            else var = get_var(n->value->value, interpreter->vars, interpreter->vars_index, n->line);

            int64_t arr_len = NUM(var.value->value) + 1;
            AST_Value *new_val = eval_node(n->left, interpreter, 0);
            if (new_val->type == Value_String && STR(new_val->value)[0] != '"') {
                char *temp = new_val->value;
                new_val->value = format_str(strlen(temp) + 3, "\"%s\"", temp);
                free(temp);
            }
            
            if (interpreter->local_vars != NULL) {
                interpreter->local_vars[var.index].value = realloc(interpreter->local_vars[var.index].value, arr_len * sizeof(AST_Value));
                
                free(interpreter->local_vars[var.index].value->value);
                interpreter->local_vars[var.index].value->value = dup_int(arr_len);
                
                if (new_val->type == Value_Number)
                    interpreter->local_vars[var.index].value[arr_len - 1] = (AST_Value) { new_val->type, dup_int(NUM(new_val->value)), 1 };
                else
                    interpreter->local_vars[var.index].value[arr_len - 1] = (AST_Value) { new_val->type, strdup(new_val->value), 1 };
                //interpreter->local_vars[var.index].value[arr_len - 1] = (AST_Value) { new_val->type, strdup(new_val->value), 1 };
            } else {
                interpreter->vars[var.index].value = realloc(interpreter->vars[var.index].value, arr_len * sizeof(AST_Value));
                
                free(interpreter->vars[var.index].value->value);
                interpreter->vars[var.index].value->value = dup_int(arr_len);
                
                if (new_val->type == Value_Number)
                    interpreter->vars[var.index].value[arr_len - 1] = (AST_Value) { new_val->type, dup_int(NUM(new_val->value)), 1 };
                else
                    interpreter->vars[var.index].value[arr_len - 1] = (AST_Value) { new_val->type, strdup(new_val->value), 1 };
                //interpreter->vars[var.index].value[arr_len - 1] = (AST_Value) { new_val->type, strdup(new_val->value), 1 };
            }

            if (new_val->mutable > 0) free_ast_value(new_val);

            break;
        default: ERR("ERROR in %s on line %ld: Unsupported statement type `%s`\n", n->file, n->line, find_ast_type(n->type))
    }
    return NULL;
}

void interpret(Interpreter *interpreter) {
    while (interpreter->program_counter < interpreter->stmts_capacity) {
        do_statement(interpreter->nodes[interpreter->program_counter], interpreter);
        interpreter->program_counter++;
    }
}
