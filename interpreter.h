#pragma once

#define AST_IS_EVALUATABLE(type) ((type == AST_Literal || IS_AST_MATH_OP(type) || type == AST_Identifier || type == AST_At || type == AST_Fn || type == AST_Fn_Call || type == AST_Len || type == AST_Cast_Num || type == AST_Cast_Str || type == AST_Array))

#define do_op(op) do {                                          \
    result = NUM(op1->value) op NUM(op2->value);                \
    if (!mutable) {                                             \
        if (result == 0) return &Zero;                          \
        else if (result == 1) return &One;                      \
    }                                                           \
    return new_ast_value(Value_Number, dup_int(result), 1, 0);  \
} while (0);                                             

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

int64_t one = 1;
AST_Value One = {Value_Number, &one, 0, 0};
int64_t zero = 0;
AST_Value Zero = {Value_Number, &zero, 0, 0};

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

AST_Value *eval_node(Node *n, Interpreter *interpreter, bool mutable);

AST_Value *call_function(Interpreter *interpreter, Node *n) {
    AST_Function *func = get_func(&interpreter->funcs, n->value->value, n->line);

    char *call_info;
    if (call_stack != NULL) {
        call_info = format_str(strlen(func->name) + 3 + num_len(n->line), "%s:%ld", func->name, n->line);
        append_verbose(call_stack, call_info, call_stack_index, call_stack_capacity)
    }

    ASSERT((func->arity == n->func_args_index), "ERROR in %s on line %ld: cant call function %s with %ld arguments, it needs %ld arguments\n", n->file, n->line, func->name, n->func_args_index, func->arity)

    Interpreter intrprtr = {
        // nodes to execute
        func->nodes,
        
        //program counter
        0,

        //global variables inherited from call
        interpreter->vars,
        
        // arguments and local variables
        new_map(8),

        // other functions
        interpreter->funcs,

        //autojump
        false
    };

    for (int i = 0; i < func->arity; i++) {
        AST_Variable *var = calloc(1, sizeof(AST_Variable));
        var->name = func->args[i]->value;
        var->value = eval_node(n->func_args[i], interpreter, 1);
        insert_entry(intrprtr.local_vars, func->args[i]->hash, Entry_AST_Variable, var);
    }

    AST_Value *rtrn;
    while (intrprtr.program_counter < intrprtr.nodes.index) {
        rtrn = do_statement(intrprtr.nodes.data[intrprtr.program_counter], &intrprtr);
        if (rtrn != NULL) {
            free_map(intrprtr.local_vars);
            if (call_stack != NULL) {
                call_stack_index--;
                call_stack[call_stack_index] = NULL;
                free(call_info);
            }
            return rtrn;
        }
        intrprtr.program_counter++;
    }
    free_map(intrprtr.local_vars);
    if (call_stack != NULL) {
        call_stack_index--;
        call_stack[call_stack_index] = NULL;
        free(call_info);
    }
    return NULL;
}
// TODO: use something like this for appending to array instead of append keyword
AST_Value *add_array(AST_Value *op1, AST_Value *op2) {
    int64_t op1_len = NUM(op1->value);
    int64_t op2_len = NUM(op2->value);
    AST_Value *new = calloc(op1_len + op2_len + 1, sizeof(AST_Value));
    new->type = Value_Array;
    new->value = dup_int(op1_len + op2_len - 1);
    new->mutable = true;
    for (int i = 1; i < op1_len; i++) {
        new[i] = (AST_Value) { op1[i].type, strdup(op1[i].value), 1, 0 };
    }
    for (int i = 1; i < op2_len; i++) {
        new[i + op1_len - 1] = (AST_Value) { op2[i].type, strdup(op2[i].value), 1, 0 };
    }
    return new;
}

void ast_assign_variable(Interpreter *interpreter, char *var_name, u_int32_t key, AST_Value *var_val, int64_t line, const char *file) {
    if (check_variable(interpreter, key) == true) 
        ERR("ERROR in %s on line %ld: cant assign `%s` multiple times\n", file, line, var_name)

    AST_Variable *var = calloc(1, sizeof(AST_Variable));
    var->name = var_name;
    var->value = var_val;

    if (interpreter->local_vars != NULL) {
        insert_entry(interpreter->local_vars, key, Entry_AST_Variable, var);
    } else {
        insert_entry(interpreter->vars, key, Entry_AST_Variable, var);
    }
}

void ast_reassign_variable(Interpreter *interpreter, char *var_name, u_int32_t key, AST_Value *new_val, int64_t line, const char *file) {
    AST_Variable *var = get_var(interpreter, var_name, key, line, file);
    free_ast_value(var->value);
    var->value = new_val;
}

void ast_unassign_variable(Interpreter *interpreter, char *var_name, u_int32_t hash, int64_t line, const char *file) {
    if (interpreter->local_vars != NULL) 
        if (delete_entry(interpreter->local_vars, hash) == true) return;

    if (delete_entry(interpreter->vars, hash) == false)
        ERR("ERROR in %s on line %ld: cant unassign nonexistent variable %s\n", file, line, var_name)
}

AST_Value *ast_math(AST_Value *op1, AST_Value *op2, int64_t op, int64_t line, const char *file, bool mutable) {
    int result;
    ASSERT((op1 != NULL), "ERROR in %s on line %ld: cant do math with a null op\n", file, line)
    int64_t op1_len = strlen(op1->value);
    int64_t op2_len;
    if (op2 != NULL) op2_len = strlen(op2->value);
    switch (op) {
        case AST_Add:
            if (op1->type == Value_Number && op2->type == Value_Number) return new_ast_value(Value_Number, dup_int(NUM(op1->value) + NUM(op2->value)), 1, 0);
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
                AST_Value *rtrn = new_ast_value(Value_String, format_str(op1_len + op2_len + 1, "%s%s", val1, val2), 1, 0);
                if (op1->type == Value_Number) free(val1);
                if (op2->type == Value_Number) free(val2);
                return rtrn;
            }
        case AST_Sub:
            if (op2 == NULL) return new_ast_value(Value_Number, dup_int(NUM(op1->value) * -1), 1, 0);
            ASSERT((op1->type == Value_Number && op2->type == Value_Number), "ERROR in %s on line %ld: Cant subtract type %s and type %s\n", file, line, find_value_type(op1->type), find_value_type(op2->type))
            return new_ast_value(Value_Number, dup_int(NUM(op1->value) - NUM(op2->value)), 1, 0);
        case AST_Mult:
            ASSERT((op1->type == Value_Number && op2->type == Value_Number), "ERROR in %s on line %ld: Cant multiply type %s and type %s\n", file, line, find_value_type(op1->type), find_value_type(op2->type))
            return new_ast_value(Value_Number, dup_int(NUM(op1->value) * NUM(op2->value)), 1, 0);
        case AST_Div:
            ASSERT((op1->type == Value_Number && op2->type == Value_Number), "ERROR in %s on line %ld: Cant divide type %s and type %s\n", file, line, find_value_type(op1->type), find_value_type(op2->type))
            return new_ast_value(Value_Number, dup_int(NUM(op1->value) / NUM(op2->value)), 1, 0);
        case AST_Less:
            ASSERT((op1->type == Value_Number && op2->type == Value_Number), "ERROR in %s on line %ld: Cant less than type %s and type %s\n", file, line, find_value_type(op1->type), find_value_type(op2->type))
            do_op(<)
        case AST_Less_Equal:
            ASSERT((op1->type == Value_Number && op2->type == Value_Number), "ERROR in %s on line %ld: Cant less equal type %s and type %s\n", file, line, find_value_type(op1->type), find_value_type(op2->type))
            do_op(<=)
        case AST_Greater:
            ASSERT((op1->type == Value_Number && op2->type == Value_Number), "ERROR in %s on line %ld: Cant greater than type %s and type %s\n", file, line, find_value_type(op1->type), find_value_type(op2->type))
            do_op(>)
        case AST_Greater_Equal:
            ASSERT((op1->type == Value_Number && op2->type == Value_Number), "ERROR in %s on line %ld: Cant greater equal type %s and type %s\n", file, line, find_value_type(op1->type), find_value_type(op2->type))
            do_op(>=)
        case AST_Is_Equal:
            if (op1->type == Value_Number && op2->type == Value_Number) {
                result = NUM(op1->value) == NUM(op2->value);
                if ((result == 0) && (!mutable)) return &Zero;
                else if ((result == 1) && (!mutable)) return &One;
                return new_ast_value(Value_Number, dup_int(result), 1, 0);
            } else if (op1->type == Value_String && op2->type == Value_String) {
                result = !strcmp(op1->value, op2->value);
                if ((result == 0) && (!mutable)) return &Zero;
                else if ((result == 1) && (!mutable)) return &One;
                return new_ast_value(Value_Number, dup_int(result), 1, 0);
            } else {
                if (!mutable) return &Zero;
                return new_ast_value(Value_Number, dup_int(0), 1, 0);
            }
            break;
        case AST_And:
            ASSERT((op1->type == Value_Number && op2->type == Value_Number), "ERROR in %s on line %ld: Cant logical and type %s and type %s\n", file, line, find_value_type(op1->type), find_value_type(op2->type))
            do_op(&&)
        case AST_Or:
            ASSERT((op1->type == Value_Number && op2->type == Value_Number), "ERROR in %s on line %ld: Cant logical or type %s and type %s\n", file, line, find_value_type(op1->type), find_value_type(op2->type))
            do_op(||)
        case AST_Not:
            return new_ast_value(Value_Number, dup_int(!(NUM(op1->value))), 1, 0);
        case AST_Not_Equal:
            if (op1->type == Value_Number && op2->type == Value_Number) {
                result = NUM(op1->value) != NUM(op2->value);
                if ((result == 0) && (!mutable)) return &Zero;
                else if ((result == 1) && (!mutable)) return &One;
                return new_ast_value(Value_Number, dup_int(result), 1, 0);
            } else if (op1->type == Value_String && op2->type == Value_String) {
                result = !!strcmp(op1->value, op2->value);
                if ((result == 0) && (!mutable)) return &Zero;
                else if ((result == 1) && (!mutable)) return &One;
                return new_ast_value(Value_Number, dup_int(result), 1, 0);
            } else {
                if (!mutable) return &Zero;
                return new_ast_value(Value_Number, dup_int(0), 1, 0);
            }
        case AST_Modulo:
            ASSERT((op1->type == Value_Number && op2->type == Value_Number), "ERROR in %s on line %ld: Cant modulo type %s and type %s\n", file, line, find_value_type(op1->type), find_value_type(op2->type))
            return new_ast_value(Value_Number, dup_int(NUM(op1->value) % NUM(op2->value)), 1, 0);
        case AST_Bit_Or:
            ASSERT((op1->type == Value_Number && op2->type == Value_Number), "ERROR in %s on line %ld: Cant bitwise or type %s and type %s\n", file, line, find_value_type(op1->type), find_value_type(op2->type))
            return new_ast_value(Value_Number, dup_int(NUM(op1->value) | NUM(op2->value)), 1, 0);
        case AST_Bit_And:
            ASSERT((op1->type == Value_Number && op2->type == Value_Number), "ERROR in %s on line %ld: Cant bitwise and type %s and type %s\n", file, line, find_value_type(op1->type), find_value_type(op2->type))
            return new_ast_value(Value_Number, dup_int(NUM(op1->value) & NUM(op2->value)), 1, 0);
        case AST_Bit_Xor:
            ASSERT((op1->type == Value_Number && op2->type == Value_Number), "ERROR in %s on line %ld: Cant bitwise xor type %s and type %s\n", file, line, find_value_type(op1->type), find_value_type(op2->type))
            return new_ast_value(Value_Number, dup_int(NUM(op1->value) ^ NUM(op2->value)), 1, 0);
        case AST_Bit_Not:
            ASSERT((op1->type == Value_Number), "ERROR in %s on line %ld: Cant bitwise not type %s\n", file, line, find_value_type(op1->type))
            return new_ast_value(Value_Number, dup_int(~(NUM(op1->value))), 1, 0);
        case AST_Rshift:
            ASSERT((op1->type == Value_Number && op2->type == Value_Number), "ERROR in %s on line %ld: Cant rshift type %s and type %s\n", file, line, find_value_type(op1->type), find_value_type(op2->type))
            return new_ast_value(Value_Number, dup_int(NUM(op1->value) >> NUM(op2->value)), 1, 0);
        case AST_Lshift:
            ASSERT((op1->type == Value_Number && op2->type == Value_Number), "ERROR in %s on line %ld: Cant lshift type %s and type %s\n", file, line, find_value_type(op1->type), find_value_type(op2->type))
            return new_ast_value(Value_Number, dup_int(NUM(op1->value) << NUM(op2->value)), 1, 0);
        case AST_Power:
            ASSERT((op1->type == Value_Number && op2->type == Value_Number), "ERROR in %s on line %ld: Cant exponentiate type %s and type %s\n", file, line, find_value_type(op1->type), find_value_type(op2->type))
            return new_ast_value(Value_Number, dup_int(exponentiate(NUM(op1->value), NUM(op2->value))), 1, 0);
        default:
            ERR("ERROR in %s on line %ld: unknown math op %ld\n", file, line, op)
    }
    return NULL;
}

AST_Value *eval_node(Node *n, Interpreter *interpreter, bool mutable) {
    if (n == NULL) return NULL;
    switch (n->type) {
        case AST_Literal:
            if (!mutable) {
                AST_Value *new_val = n->value;
                new_val->mutable = false;
                return new_val;
            }
            if (n->value->type == Value_Number)
                return new_ast_value(Value_Number, dup_int(NUM(n->value->value)), 1, 0);
            return new_ast_value(n->value->type, strdup(n->value->value), 1, 0);
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
            AST_Value *result = ast_math(op1, op2, n->type, n->line, n->file, mutable);
            if (op1->mutable) free_ast_value(op1);
            if (op2 && op2->mutable) free_ast_value(op2);
            return result;
        case AST_Identifier: {
            AST_Variable *var = get_var(interpreter, n->value->value, n->value->hash, n->line, n->file);
            if (var->value == NULL) return NULL;

            if (!mutable) {
                AST_Value *new_val = var->value;
                new_val->mutable = false;
                return new_val;
            }
            if (var->value->type == Value_Array) {
                int64_t arr_len = NUM(var->value->value);
                AST_Value *array = calloc(arr_len, sizeof(struct AST_Value));
                array->mutable = true;
                array->type = Value_Array;
                array->value = dup_int(arr_len);
                for (int i = 1; i < arr_len; i++) {
                    array[i].type = var->value[i].type;
                    if (array[i].type == Value_Number) 
                        array[i].value = dup_int(NUM(var->value[i].value));
                    else
                        array[i].value = strdup(var->value[i].value);
                }
                return array;
            }
            if (var->value->type == Value_Number)
                return new_ast_value(Value_Number, dup_int(NUM(var->value->value)), 1, 0);
            return new_ast_value(var->value->type, strdup(var->value->value), 1, 0);
        }
        case AST_Array: {
            if (!mutable) return n->value;
            int64_t arr_len = NUM(n->value->value);
            AST_Value *array = calloc(arr_len, sizeof(struct AST_Value));
            array->mutable = true;
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
            int64_t index = 0;
            if (n->left->type != AST_Literal) {
                AST_Value *val = eval_node(n->left, interpreter, 0);
                index = NUM(val->value);
                if (val->mutable) free_ast_value(val);
            } else index = NUM(n->left->value->value);

            AST_Variable *var = get_var(interpreter, n->value->value, n->value->hash, n->line, n->file);

            if (var->value->type == Value_String) {
                return new_ast_value(Value_String, format_str(2, "%c", STR(var->value->value)[index - 1]), 1, 0);
            }
            int64_t arr_len = NUM(var->value[0].value);
            if (index >= arr_len) ERR("ERROR in %s on line %ld: Index %ld is out of bounds for array %s, length %ld\n", n->file, n->line, index, var->name, arr_len - 1)
            else if (index < 1) ERR("ERROR in %s on line %ld: invalid index %ld, it is less than 1\n", n->file, n->line, index)

            if (var->value[index].type == Value_String) {
                int64_t len = strlen(var->value[index].value);
                if (STR(var->value[index].value)[0] != '"') return new_ast_value(var->value[index].type, strdup(var->value[index].value), 1, 0);
                else return new_ast_value(var->value[index].type, format_str(len - 1, "%.*s", len, STR(var->value[index].value) + 1), 1, 0);
            } else if (var->value[index].type == Value_Number) {
                if (!mutable) {
                    AST_Value *val = &var->value[index];
                    val->mutable = false;
                    return val;
                }
                return new_ast_value(var->value[index].type, dup_int(NUM(var->value[index].value)), 1, 0);
            } else if (var->value[index].type == Value_Identifier) {
                AST_Variable *rtrn = get_var(interpreter, var->value[index].value, var->value[index].hash, n->line, n->file);

                if (!mutable) {
                    AST_Value *new_val = rtrn->value;
                    new_val->mutable = false;
                    return new_val;
                }
                if (rtrn->value->type == Value_Array) {
                    int64_t arr_len = NUM(rtrn->value->value);
                    AST_Value *array = calloc(arr_len + 1, sizeof(rtrn->value[0]));
                    array->mutable = true;
                    for (int i = 0; i < arr_len; i++) {
                        array[i].type = rtrn->value[i].type;
                        array[i].value = strdup(rtrn->value[i].value);
                    }
                    return array;
                }
                return new_ast_value(rtrn->value->type, strdup(rtrn->value->value), 1, 0);
            } else ERR("ERROR in %s on line %ld: Can't evaluate %s as part of array\n", n->file, n->line, find_value_type(var->value[index].type))
            break;
        }
        case AST_Fn_Call:
            return call_function(interpreter, n);
        case AST_Len:{
            ASSERT((n->left->value->type == Value_Array || n->left->value->type == Value_String || n->left->value->type == Value_Identifier || n->left->value->type == Value_Number), "ERROR in %s on line %ld: cant do len on value type %s\n", n->file, n->line, find_value_type(n->left->value->type))
            AST_Value *op = eval_node(n->left, interpreter, false);
            AST_Value *result;
            switch (op->type) {
                case Value_String:
                    result = new_ast_value(Value_Number, dup_int(strlen(op->value)), 1, 0);
                    if (op->mutable) free_ast_value(op);
                    return result;
                case Value_Array:;
                    result = new_ast_value(Value_Number, dup_int(NUM(op[0].value) - 1), 1, 0);
                    if (op->mutable) free_ast_value(op);
                    return result;
                case Value_Number:;
                    result = new_ast_value(Value_Number, dup_int(num_len(NUM(op->value)) + 1), 1, 0);
                    if (op->mutable) free_ast_value(op);
                    return result;
                default: ERR("ERROR in %s on line %ld: cant evaluate length of value type %s\n", n->file, n->line, find_value_type(op->type))
            }
            break;}
        case AST_Cast_Num: {
            AST_Value *val = eval_node(n->left, interpreter, mutable);
            if (val->type == Value_Number) return val;
            ASSERT((val->type == Value_String), "ERROR in %s on line %ld: cant cast type %s to number\n", n->file, n->line, find_value_type(val->type))
            if (val->mutable) {
                int64_t *x = dup_int(strtoint(STR(val->value), strlen(STR(val->value))));
                free(val->value);
                val->value = x;
                val->type = Value_Number;
                return val;
            } else
                return new_ast_value(Value_Number, dup_int(strtoint(STR(val->value), strlen(STR(val->value)))), 1, 0);
        }
        case AST_Cast_Str: {
            AST_Value *val = eval_node(n->left, interpreter, mutable);
            if (val->type == Value_String) return val;
            ASSERT((val->type == Value_Number), "ERROR in %s on line %ld: cant cast type %s to string\n", n->file, n->line, find_value_type(val->type))
            if (val->mutable) {
                char *x = format_str(num_len(NUM(val->value)) + 1, "%ld", NUM(val->value));
                free(val->value);
                val->value = x;
                val->type = Value_String;
                return val;
            } else 
                return new_ast_value(Value_String, format_str(num_len(NUM(val->value)) + 1, "%ld", NUM(val->value)), 1, 0);
        }
        case AST_Input:;
            int input_capacity = 10;
            int input_index = 0;
            char *input = calloc(input_capacity, sizeof(char));
            char c;
            while ((c = fgetc(stdin))) {
                if (c == '\n') break;
                append_verbose(input, c, input_index, input_capacity)
            }
            return new_ast_value(Value_String, input, 1, 0);
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
                if (print->mutable) {
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
                if (print->mutable) free_ast_value(print);
            }

            break;
        case AST_Var_Assign:;
            char *var_name = n->value->value;
            AST_Value *var_val = eval_node(n->left, interpreter, 1);
            ast_assign_variable(interpreter, var_name, n->value->hash, var_val, n->line, n->file);
            debug("ASSIGN `%s` to variable `%s`\n", STR(var_val->value), var_name)
            break;
        case AST_If:;
            AST_Value *expr = eval_node(n->left, interpreter, 0);
            if (NUM(expr->value) == 0) {
                interpreter->program_counter = n->jump_index;
            } else {
                interpreter->auto_jump = 1;
            }
            if (expr->mutable) free_ast_value(expr);
            break;
        case AST_Elif:;{
            if (interpreter->auto_jump == 1) {
                if (interpreter->nodes.data[n->jump_index]->type == AST_Else)
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
            if (expr->mutable) free_ast_value(expr);
            break;}
        case AST_For:;
            debug("FOR: index is `%s`\n", STR(n->value->value))
            AST_Value *list = eval_node(n->right, interpreter, 0);
            int64_t index = NUM(n->value->value);
            free(n->value->value);
            n->value->value = NULL;
           
            int64_t len;
            AST_Value *new_value;
            if (list->type == Value_Array) {
                len = NUM(list[0].value);
                if (index >= (len - 1)) {
                    //TODO: only works if iterator is last variable in list, ie no variables created inside loop
                    ast_unassign_variable(interpreter, n->left->value->value, n->left->value->hash, n->line, n->file);
                    free_ast_value(n->value);
                    n->value = new_ast_value(Value_Number, dup_int(0), 1, 0);
                    interpreter->program_counter = n->jump_index;
                    break;
                }
                if (list[index + 1].type == Value_String) {
                    int64_t len = strlen(list[index + 1].value);
                    new_value = new_ast_value(list[index + 1].type, format_str(len, "%.*s", len - 2, ((char*)list[index + 1].value) + 1), 1, 0);
                } else {
                    new_value = new_ast_value(list[index + 1].type, dup_int(NUM(list[index + 1].value)), 1, 0);
                }
            } else if (list->type == Value_String) {
                len = strlen(list->value) + 1;
                if (index >= (len - 1)) {
                    ast_unassign_variable(interpreter, n->left->value->value, n->left->value->hash, n->line, n->file);
                    free_ast_value(n->value);
                    n->value = new_ast_value(Value_Number, dup_int(0), 1, 0);
                    interpreter->program_counter = n->jump_index;
                    break;
                }
                new_value = new_ast_value(Value_String, format_str(2, "%c", STR(list->value)[index]), 1, 0);
            } else ERR("ERROR in %s on line %ld: cant iterate through type %s\n", n->file, n->line, find_value_type(list->type))

            if (index < 1) {
                ast_assign_variable(interpreter, n->left->value->value, n->left->value->hash, new_value, n->line, n->file);
            }
            else if (index < (len - 1)) {
                ast_reassign_variable(interpreter, n->left->value->value, n->left->value->hash, new_value, n->line, n->file);
            }
            index++;
            n->value->value = dup_int(index);
            break;
        case AST_Else:
            interpreter->program_counter = n->jump_index;
            break;
        case AST_Semicolon:
            if (interpreter->nodes.data[n->jump_index]->type == AST_While || interpreter->nodes.data[n->jump_index]->type == AST_For) {
                interpreter->program_counter = n->jump_index - 1;
            }
            interpreter->auto_jump = 0;
            break;
        case AST_Identifier:;{
            AST_Value *new_val = eval_node(n->left, interpreter, 1);
            ast_reassign_variable(interpreter, n->value->value, n->value->hash, new_val, n->line, n->file);

            break;}
        case AST_While:;{
            AST_Value *expr = eval_node(n->left, interpreter, 0);
            if (NUM(expr->value) == 0) {
                interpreter->program_counter = n->jump_index;
            }
            if (expr->mutable) free_ast_value(expr);
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
                if (i->mutable) free_ast_value(i);
            } else {
                new_val = eval_node(n->left->left, interpreter, 1);
                AST_Value *i = eval_node(n->left, interpreter, 1);
                index = NUM(i->value);
                if (i->mutable) free_ast_value(i);
            }

            AST_Variable *var = get_var(interpreter, n->value->value, n->value->hash, n->line, n->file);

            //if it was assigned after array was created, add quotes around value
            if (new_val->type == Value_String && ((char*)new_val->value)[0] != '"' && var->value->type == Value_Array) {
                char *temp = strdup(new_val->value);
                free(new_val->value);
                new_val->value = format_str((strlen(temp) + 3), "\"%s\"", temp);
                free(temp);
            }

            if (var->value->type == Value_String) {
                ((char*)var->value->value)[index - 1] = ((char*)new_val->value)[0];
                if (new_val->mutable) free_ast_value(new_val);
                break;
            }
            free((AST_Value*)var->value[index].value);
            var->value[index] = *new_val;
            free(new_val);

            break;}
        case AST_Return:
            return eval_node(n->left, interpreter, 1);
        case AST_Fn_Call:;
            AST_Value *result = call_function(interpreter, n);
            if (result == NULL) break;
            if (result->mutable) free_ast_value(result);
            break;
        case AST_Exit:;
            AST_Value *exit_val = eval_node(n->left, interpreter, 0);
            ASSERT(exit_val->type == Value_Number, "ERROR in %s on line %ld: tried to exit with non-number code\n", n->file, n->line)
            int64_t val = NUM(exit_val->value);
            if (exit_val->mutable) free_ast_value(exit_val);
            free_mem(val);
            break;
        case AST_Append:;
            AST_Variable *var = get_var(interpreter, n->value->value, n->value->hash, n->line, n->file);

            int64_t arr_len = NUM(var->value->value) + 1;
            AST_Value *new_val = eval_node(n->left, interpreter, 0);
            if (new_val->type == Value_String && STR(new_val->value)[0] != '"') {
                char *temp = new_val->value;
                new_val->value = format_str(strlen(temp) + 3, "\"%s\"", temp);
                free(temp);
            }

            var->value = realloc(var->value, arr_len * sizeof(AST_Value));
            free(var->value[0].value);
            var->value[0].value = dup_int(arr_len);
            if (new_val->type == Value_Number)
                var->value[arr_len - 1] = (AST_Value) { new_val->type, dup_int(NUM(new_val->value)), 1, 0 };
            else 
                var->value[arr_len - 1] = (AST_Value) { new_val->type, strdup(new_val->value), 1, new_val->hash };

            if (new_val->mutable) free_ast_value(new_val);

            break;
        default: ERR("ERROR in %s on line %ld: Unsupported statement type `%s`\n", n->file, n->line, find_ast_type(n->type))
    }
    return NULL;
}

void interpret(Interpreter *interpreter) {
    while (interpreter->program_counter < interpreter->nodes.index) {
        do_statement(interpreter->nodes.data[interpreter->program_counter], interpreter);
        interpreter->program_counter++;
    }
}
