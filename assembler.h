#pragma once

#define read_index (COMBYTE(code.data[i + 1], code.data[i + 2]))
#define emit_op_comment(_op) emit(0, "%s_label_%d: ; " #_op " %s:%ld", name, i, locs.data[i].file, locs.data[i].line)
#define VALUE_NUMBER 0
#define VALUE_STRING 1
#define VALUE_ARRAY 2

#define emit_binary_op(_op)              \
    emit(8, "pop op2_value");            \
    emit(8, "pop op2_metadata");         \
    emit(8, "pop op1_value");            \
    emit(8, "pop op1_metadata");         \
                                         \
    emit(8, "mov rax, op1_metadata");    \
    emit(8, "mov rbx, op2_metadata");    \
    emit(8, "and rax, 3");               \
    emit(8, "and rbx, 3");               \
    emit(8, "xor rax, rbx");             \
    emit(8, "cmp rax, 0");               \
    emit(8, "jnz %s_%d_error", name, i); \
                                         \
    emit(8, "cmp rax, VALUE_NUMBER");    \
    emit(8, "jne %s_%d_error", name, i); \
                                         \
    emit(8, "mov rax, op1_value");       \
    emit(8, #_op " rax, op2_value");     \
    emit(8, "push 0");                   \
    emit(8, "push rax");                 \
    emit(8, "jmp %s_%d_done", name, i);  \
                                         \
    emit(0, "%s_%d_error:", name, i);    \
    emit(8, "ERROR");                    \
    emit(0, "%s_%d_done:", name, i);     \

Functions funcs;

int op_offsets[] = {
    3, //OP_CONSTANT
    3, //OP_ARRAY
    1, //OP_PRINTLN
    1, //OP_PRINT
    3, //OP_DEFINE_GLOBAL
    2, //OP_DEFINE_LOCAL
    3, //OP_SET_GLOBAL
    3, //OP_GET_GLOBAL
    2, //OP_SET_LOCAL
    2, //OP_GET_LOCAL
    1, //OP_ADD
    1, //OP_SUBTRACT
    1, //OP_MULTIPLY
    1, //OP_DIVIDE
    1, //OP_MODULO
    1, //OP_BIT_AND
    1, //OP_BIT_OR
    1, //OP_BIT_XOR
    1, //OP_BIT_NOT
    1, //OP_LSHIFT
    1, //OP_RSHIFT
    1, //OP_POWER
    1, //OP_LESS
    1, //OP_LESS_EQUAL
    1, //OP_GREATER
    1, //OP_GREATER_EQUAL
    1, //OP_IS_EQUAL
    1, //OP_AND
    1, //OP_OR
    1, //OP_NOT
    1, //OP_NOT_EQUAL
    3, //OP_JUMP_IF_FALSE
    3, //OP_START_IF
    3, //OP_JUMP
    1, //OP_GET_ELEMENT
    3, //OP_SET_ELEMENT_GLOBAL
    3, //OP_SET_ELEMENT_LOCAL
    3, //OP_APPEND_GLOBAL
    2, //OP_APPEND_LOCAL
    3, //OP_POP
    1, //OP_CAST_STR
    1, //OP_CAST_NUM
    3, //OP_CALL
    3, //OP_CALL_NATIVE
    1, //OP_RETURN
    1, //OP_RETURN_NOTHING
    3, //OP_BREAK
    3, //OP_CONTINUE
    6, //OP_FOR
    7, //OP_ENUMERATE
    5, //OP_GET_GLOBAL_GET_CONSTANT
    4, //OP_GET_LOCAL_GET_CONSTANT
    1, //OP_DONE
};

FILE *f;

__attribute__((format(printf, 2, 3)))
void emit(int indent, char *fmt, ...) {
    fprintf(f, "%*s", indent, "");
    va_list args;
    va_start(args, fmt);
    vfprintf(f, fmt, args);
    va_end(args);
    fprintf(f, "\n");
}

void emit_header() {
    emit(0, "VALUE_NUMBER equ 0");
    emit(0, "VALUE_STRING equ 1");
    emit(0, "VALUE_ARRAY equ 2");
    emit(0, "op1_value equ r8");
    emit(0, "op1_metadata equ r9");
    emit(0, "op2_value equ r10");
    emit(0, "op2_metadata equ r11");
    
    emit(0, "macro ERROR {");
    emit(8, "mov rdx, 5");
    emit(8, "mov rsi, STR_ERROR");
    emit(8, "mov rax, 1");
    emit(8, "syscall");
    emit(8, "mov rsi, STR_NEWLINE");
    emit(8, "mov rdx, 1");
    emit(8, "mov rax, 1");
    emit(8, "syscall");
    emit(8, "mov rax, 60");
    emit(8, "mov rdi, 1");
    emit(8, "syscall");
    emit(0, "}");

    emit(0, "macro PRINTLN_NUM _num {");
    emit(8, "push rdi");
    emit(8, "push rsi");
    emit(8, "push rdx");
    emit(8, "push rax");
    emit(8, "mov rdi, _num");
    emit(8, "call write_num");
    emit(8, "mov rsi, STR_NEWLINE");
    emit(8, "mov rdx, 1");
    emit(8, "mov rax, 1");
    emit(8, "syscall");
    emit(8, "pop rax");
    emit(8, "pop rdx");
    emit(8, "pop rsi");
    emit(8, "pop rdi");
    emit(0, "}");

    emit(0, "macro WRITE _val, _len {");
    emit(8, "push rsi");
    emit(8, "push rdx");
    emit(8, "push rax");
    emit(8, "mov rsi, _val");
    emit(8, "mov rdx, _len");
    emit(8, "mov rax, 1");
    emit(8, "syscall");
    emit(8, "pop rax");
    emit(8, "pop rdx");
    emit(8, "pop rsi");
    emit(0, "}");

    emit(0, "format ELF64 executable 3");
    emit(0, "segment readable executable");
    emit(0, "entry start");
    emit(0, "start:");
    emit(8, "mov rbp, rsp");
    emit(8, "lea r15, [call_stack + 64*16]");
}

u_int64_t serialize_constant(Value val) {
    u_int64_t info = 0;
    switch (val.type) {
        case Value_Number:
            return info;
        case Value_String:
            info = val.val.str.len;
            info = (info << 4);
            info = (info | VALUE_STRING);
            return info;
        case Value_Array:
            info = ARRAY_LEN(val.val.num) - 1;
            info = (info << 36);
            info = (info | (ARRAY_SIZE(val.val.num) << 4));
            info = (info | VALUE_ARRAY);
            return info;
        default: ERR("cant serialize value type %s\n", find_value_type(val.type))
    }
    return 0;
}

void emit_func(char *name, Code code, Locations locs, Constants constants) {
    
    if (strcmp(name, "MAIN")) {
        emit(0, "fn_%s:", name);
        emit(8, "sub r15, 8");
        emit(8, "pop qword [r15]");
    }

    for (int i = 0; i < code.index; i++) {
        switch (code.data[i]) {
            case OP_CONSTANT:
                emit_op_comment(OP_CONSTANT);
                Value val = constants.data[read_index];
                emit(8, "push %ld", serialize_constant(val));
                switch (val.type) {
                    case Value_Number:
                        emit(8, "push %ld", val.val.num);
                        break;
                    case Value_String:
                        emit(8, "push STR_%s_%d", name, read_index);
                        break;
                    default:
                        ERR("ERROR in %s on line %ld: cant emit asm for value type %s\n", locs.data[i].file, locs.data[i].line, find_value_type(val.type))
                }
                i += 2;
                break;
            case OP_ARRAY:
                emit_op_comment(OP_ARRAY);
                emit(8, "mov rax, %ld", serialize_constant(constants.data[read_index].val.array[0]));
                emit(8, "push rax");
                emit(8, "push ARR_%s_%d", name, read_index);
                i += 2;
                break;
            case OP_PRINTLN:
                emit_op_comment(OP_PRINTLN);
                emit(8, "pop op1_value");
                emit(8, "pop op1_metadata"); 
                emit(8, "mov rdi, 1");
                emit(8, "call print_value");
                emit(8, "WRITE STR_NEWLINE, 1");
                break;
            case OP_ADD:
                emit_op_comment(OP_ADD);
                emit(8, "pop op2_value");
                emit(8, "pop op2_metadata");
                emit(8, "pop op1_value");
                emit(8, "pop op1_metadata");
                
                emit(8, "mov rax, op1_metadata");
                emit(8, "mov rbx, op2_metadata");
                emit(8, "and rax, 3");
                emit(8, "and rbx, 3");
                emit(8, "xor rax, rbx");
                emit(8, "cmp rax, 0");
                emit(8, "jnz %s_%d_error", name, i);

                emit(8, "mov rax, op1_metadata");
                emit(8, "and rax, 3");
                
                emit(8, "cmp rax, VALUE_NUMBER");
                emit(8, "je %s_%d_add_num", name, i);
                emit(8, "cmp rax, VALUE_STRING");
                emit(8, "je %s_%d_add_str", name, i);
                emit(8, "cmp rax, VALUE_ARRAY");
                emit(8, "je %s_%d_add_array", name, i);

                emit(0, "%s_%d_add_num:", name, i);
                emit(8, "add rax, op1_value");
                emit(8, "add rax, op2_value");
                emit(8, "push 0");
                emit(8, "push rax");
                emit(8, "jmp %s_%d_done", name, i);

                emit(0, "%s_%d_add_str:", name, i);
                emit(0, "%s_%d_add_array:", name, i);

                emit(0, "%s_%d_error:", name, i);
                emit(8, "ERROR");
                emit(0, "%s_%d_done:", name, i);
                break;
            case OP_SUBTRACT:
                emit_op_comment(OP_SUBTRACT);
                emit_binary_op(sub);
                break;
            case OP_MULTIPLY:
                emit_op_comment(OP_MULTIPLY);
                emit_binary_op(imul);
                break;
            case OP_DIVIDE:
                emit_op_comment(OP_DIVIDE);
                emit_binary_op(div);
                break;
            case OP_IS_EQUAL:
                emit_op_comment(OP_IS_EQUAL);
                emit(8, "pop op2_value");
                emit(8, "pop op2_metadata");
                emit(8, "pop op1_value");
                emit(8, "pop op1_metadata");
                
                // make sure ops are same type
                emit(8, "mov rax, op1_metadata");
                emit(8, "mov rbx, op2_metadata");
                emit(8, "and rax, 3");
                emit(8, "and rbx, 3");
                emit(8, "xor rax, rbx");
                emit(8, "cmp rax, 0");
                emit(8, "jnz %s_%d_false", name, i);
                
                emit(8, "mov rax, op1_metadata");
                emit(8, "and rax, 3");
                emit(8, "cmp rax, VALUE_NUMBER");
                emit(8, "je %s_%d_is_equal_num", name, i);
                emit(8, "cmp rax, VALUE_STRING");
                emit(8, "je %s_%d_is_equal_str", name, i);

                emit(0, "%s_%d_is_equal_num:", name, i);
                emit(8, "cmp op1_value, op2_value");
                emit(8, "je %s_%d_true", name, i);
                emit(8, "jmp %s_%d_false", name, i);

                emit(0, "%s_%d_is_equal_str:", name, i);
                emit(8, "jmp %s_%d_error", name, i);

                emit(0, "%s_%d_false:", name, i);
                emit(8, "push 0");
                emit(8, "push 0");
                emit(8, "jmp %s_%d_done", name, i);
                emit(0, "%s_%d_true:", name, i);
                emit(8, "push 0");
                emit(8, "push 1");
                emit(8, "jmp %s_%d_done", name, i);

                emit(0, "%s_%d_error:", name, i);
                emit(8, "ERROR");
                emit(0, "%s_%d_done:", name, i);
                break;
            case OP_LESS:
                emit_op_comment(OP_LESS);
                emit(8, "pop op2_value");
                emit(8, "pop op2_metadata");
                emit(8, "pop op1_value");
                emit(8, "pop op1_metadata");

                emit(8, "mov rax, op1_metadata");
                emit(8, "mov rbx, op2_metadata");
                emit(8, "and rax, 3");
                emit(8, "and rbx, 3");
                emit(8, "cmp rax, VALUE_NUMBER");
                emit(8, "jne %s_%d_error", name, i);
                emit(8, "cmp rbx, VALUE_NUMBER");
                emit(8, "jne %s_%d_error", name, i);

                emit(8, "cmp op1_value, op2_value");
                emit(8, "setl al");
                emit(8, "movzx rax, al");
                emit(8, "push 0");
                emit(8, "push rax");
                emit(8, "jmp %s_%d_done", name, i);
                
                emit(0, "%s_%d_error:", name, i);
                emit(8, "ERROR");
                emit(0, "%s_%d_done:", name, i);
                break;
            case OP_LESS_EQUAL:
                emit_op_comment(OP_LESS_EQUAL);
                emit(8, "pop op2_value");
                emit(8, "pop op2_metadata");
                emit(8, "pop op1_value");
                emit(8, "pop op1_metadata");

                emit(8, "mov rax, op1_metadata");
                emit(8, "mov rbx, op2_metadata");
                emit(8, "and rax, 3");
                emit(8, "and rbx, 3");
                emit(8, "cmp rax, VALUE_NUMBER");
                emit(8, "jne %s_%d_error", name, i);
                emit(8, "cmp rbx, VALUE_NUMBER");
                emit(8, "jne %s_%d_error", name, i);

                emit(8, "cmp op1_value, op2_value");
                emit(8, "setle al");
                emit(8, "movzx rax, al");
                emit(8, "push 0");
                emit(8, "push rax");
                emit(8, "jmp %s_%d_done", name, i);
                
                emit(0, "%s_%d_error:", name, i);
                emit(8, "ERROR");
                emit(0, "%s_%d_done:", name, i);
                break;
            case OP_GREATER:
                emit_op_comment(OP_GREATER);
                emit(8, "pop op2_value");
                emit(8, "pop op2_metadata");
                emit(8, "pop op1_value");
                emit(8, "pop op1_metadata");

                emit(8, "mov rax, op1_metadata");
                emit(8, "mov rbx, op2_metadata");
                emit(8, "and rax, 3");
                emit(8, "and rbx, 3");
                emit(8, "cmp rax, VALUE_NUMBER");
                emit(8, "jne %s_%d_error", name, i);
                emit(8, "cmp rbx, VALUE_NUMBER");
                emit(8, "jne %s_%d_error", name, i);

                emit(8, "cmp op1_value, op2_value");
                emit(8, "setg al");
                emit(8, "movzx rax, al");
                emit(8, "push 0");
                emit(8, "push rax");
                emit(8, "jmp %s_%d_done", name, i);
                
                emit(0, "%s_%d_error:", name, i);
                emit(8, "ERROR");
                emit(0, "%s_%d_done:", name, i);
                break;
            case OP_GREATER_EQUAL:
                emit_op_comment(OP_GREATER_EQUAL);
                emit(8, "pop op2_value");
                emit(8, "pop op2_metadata");
                emit(8, "pop op1_value");
                emit(8, "pop op1_metadata");

                emit(8, "mov rax, op1_metadata");
                emit(8, "mov rbx, op2_metadata");
                emit(8, "and rax, 3");
                emit(8, "and rbx, 3");
                emit(8, "cmp rax, VALUE_NUMBER");
                emit(8, "jne %s_%d_error", name, i);
                emit(8, "cmp rbx, VALUE_NUMBER");
                emit(8, "jne %s_%d_error", name, i);

                emit(8, "cmp op1_value, op2_value");
                emit(8, "setge al");
                emit(8, "movzx rax, al");
                emit(8, "push 0");
                emit(8, "push rax");
                emit(8, "jmp %s_%d_done", name, i);
                
                emit(0, "%s_%d_error:", name, i);
                emit(8, "ERROR");
                emit(0, "%s_%d_done:", name, i);
                break;
            case OP_DEFINE_GLOBAL:
                emit_op_comment(OP_DEFINE_GLOBAL);
                emit(8, "pop qword [%s]", constants.data[read_index].val.str.chars);
                emit(8, "pop qword [%s + 8]", constants.data[read_index].val.str.chars);
                i += 2;
                break;
            case OP_GET_GLOBAL:
                emit_op_comment(OP_GET_GLOBAL);
                emit(8, "push qword [%s + 8]", constants.data[read_index].val.str.chars);
                emit(8, "push qword [%s]", constants.data[read_index].val.str.chars);
                i += 2;
                break;
            case OP_SET_GLOBAL:
                emit_op_comment(OP_SET_GLOBAL);
                emit(8, "pop qword [%s]", constants.data[read_index].val.str.chars);
                emit(8, "pop qword [%s + 8]", constants.data[read_index].val.str.chars);
                i += 2;
                break;
            case OP_START_IF:
                emit_op_comment(OP_START_IF);
                emit(8, "pop op1_value");
                emit(8, "pop op1_metadata");
                emit(8, "cmp op1_value, 0");
                emit(8, "jz %s_label_%d", name, i + read_index);
                i += 2;
                break;
            case OP_JUMP_IF_FALSE:
                emit_op_comment(OP_JUMP_IF_FALSE);
                emit(8, "pop op1_value");
                emit(8, "pop op1_metadata");
                emit(8, "cmp op1_value, 1");
                emit(8, "jnz %s_label_%d", name, i + read_index);
                i += 2;
                break;
            case OP_JUMP:
                emit_op_comment(OP_JUMP);
                emit(8, "jmp %s_label_%d", name, read_index);
                i += 2;
                break;
            case OP_DEFINE_LOCAL:
                emit_op_comment(OP_DEFINE_LOCAL);
                emit(8, "pop op1_value");
                emit(8, "pop op1_metadata");
                emit(8, "sub rsp, 16");
                emit(8, "mov qword [rbp + %d], op1_value", 16 * code.data[i + 1]);
                emit(8, "mov qword [rbp + %d], op1_metadata", 16 * code.data[++i] + 8);
                break;
            case OP_GET_LOCAL:
                emit_op_comment(OP_GET_LOCAL);
                emit(8, "push qword [rbp + %d]", 16 * code.data[i + 1] + 8);
                emit(8, "push qword [rbp + %d]", 16 * code.data[++i]);
                break;
            case OP_SET_LOCAL:
                emit_op_comment(OP_SET_LOCAL);
                emit(8, "pop qword [rbp + %d]", 16 * code.data[i + 1]);
                emit(8, "pop qword [rbp + %d]", 16 * code.data[++i] + 8);
                break;
            case OP_POP:
                emit_op_comment(OP_POP);
                emit(8, "add rsp, %d", 16 * read_index);
                i += 2;
                break;
            case OP_GET_GLOBAL_GET_CONSTANT:{
                emit_op_comment(OP_GET_GLOBAL_GET_CONSTANT);
                emit(8, "push qword [%s + 8]", constants.data[read_index].val.str.chars);
                emit(8, "push qword [%s]", constants.data[read_index].val.str.chars);
                i += 2;
                Value val = constants.data[read_index];
                emit(8, "push %ld", serialize_constant(val));
                switch (val.type) {
                    case Value_Number:
                        emit(8, "push %ld", val.val.num);
                        break;
                    case Value_String:
                        emit(8, "push STR_%s_%d", name, read_index);
                        break;
                    default:
                        ERR("ERROR in %s on line %ld: cant emit asm for value type %s\n", locs.data[i].file, locs.data[i].line, find_value_type(val.type))
                }
                i += 2;
                break;}
            case OP_GET_LOCAL_GET_CONSTANT: {
                emit_op_comment(OP_GET_LOCAL_GET_CONSTANT);
                emit(8, "push qword [rbp + %d]", 16 * code.data[i + 1] + 8);
                emit(8, "push qword [rbp + %d]", 16 * code.data[++i]);
                Value val = constants.data[read_index];
                emit(8, "push %ld", serialize_constant(val));
                switch (val.type) {
                    case Value_Number:
                        emit(8, "push %ld", val.val.num);
                        break;
                    case Value_String:
                        emit(8, "push STR_%s_%d", name, read_index);
                        break;
                    default:
                        ERR("ERROR in %s on line %ld: cant emit asm for value type %s\n", locs.data[i].file, locs.data[i].line, find_value_type(val.type))
                }
                i += 2;
                break;}
            case OP_CALL:
                emit_op_comment(OP_CALL);
                emit(8, "sub r15, 8");
                emit(8, "mov [r15], rbp");
                emit(8, "lea rbp, [rsp + %d]", 16 * (funcs.data[read_index].arity - 1));
                emit(8, "call fn_%s", funcs.data[read_index].name);
                emit(8, "mov rbp, [r15]");
                emit(8, "add r15, 8");
                emit(8, "add rsp, %d", funcs.data[read_index].arity * 16);
                emit(8, "push op1_metadata");
                emit(8, "push op1_value");
                i += 2;
                break;
            case OP_RETURN:
                emit_op_comment(OP_RETURN);
                emit(8, "pop op1_value");
                emit(8, "pop op1_metadata");
                emit(8, "push qword [r15]");
                emit(8, "add r15, 8");
                emit(8, "ret");
                break;
            case OP_RETURN_NOTHING:
                emit_op_comment(OP_RETURN_NOTHING);
                emit(8, "push qword [r15]");
                emit(8, "add r15, 8");
                emit(8, "ret");
                break;
            case OP_DONE:
                emit_op_comment(OP_DONE);
                emit(8, "mov rax, 60");
                emit(8, "mov rdi, 0");
                emit(8, "syscall");
                break;
            default:
                ERR("ERROR in %s on line %ld: cant emit asm for op type %s\n", locs.data[i].file, locs.data[i].line, find_op_code(code.data[i]))
                break;
        }
    }
}

void emit_footer(Function *func) {
    emit(0, "segment readable writeable");
    emit(0, "call_stack: rb 64*16");
    emit(0, "STR_NEWLINE: db 10, 0");
    emit(0, "STR_ERROR: db \"ERROR\", 0");
    emit(0, "STR_ARRAY_START: db \"[\", 0");
    emit(0, "STR_ARRAY_SEP: db \", \", 0");
    emit(0, "STR_ARRAY_END: db \"]\", 0");
    emit(0, "STR_QUOTE: db 34, 0");

    //globals
    for (int i = 0; i < func->code.index; i += op_offsets[func->code.data[i]]) {
        if (func->code.data[i] == OP_DEFINE_GLOBAL) {
            emit(0, "%s: rb 16", func->constants.data[COMBYTE(func->code.data[i + 1], func->code.data[i + 2])].val.str.chars);
        }
    }

    //string and array constants
    for (int i = 0; i < funcs.index; i++) {
        Function *func = &funcs.data[i];
        for (int j = 0; j < func->constants.index; j++) {
            if (func->constants.data[j].type == Value_String) {

                fprintf(f, "STR_%s_%d: db ", func->name, j);
                string s = func->constants.data[j].val.str;
                
                fprintf(f, "\"%.*s\", 0\n", (int)s.len, s.chars);

            } else if (func->constants.data[j].type == Value_Array) {
                Value *arr = &func->constants.data[j].val.array[0];
                fprintf(f, "ARR_%s_%d: dq ", func->name, j);
                for (u_int32_t k = 1; k < ARRAY_LEN(arr[0].val.num); k++) {
                    switch (arr[k].type) {
                        case Value_Number:
                            fprintf(f, "%ld, %ld, ", serialize_constant(arr[k]), arr[k].val.num);
                            break;
                        case Value_String:
                            fprintf(f, "%ld, ARR_STR_%s_%d_%d, ", serialize_constant(arr[k]), func->name, j, k);
                            break;
                        default:
                            ERR("ERROR cant serialize type %s as part of array", find_value_type(arr[k].type))
                    }
                }
                fprintf(f, "0\n");
                for (u_int32_t k = 1; k < ARRAY_LEN(arr[0].val.num); k++) {
                    if (arr[k].type == Value_String) {
                        fprintf(f, "ARR_STR_%s_%d_%d: db ", func->name, j, k);
                        string s = arr[k].val.str;

                        fprintf(f, "\"%.*s\", 0\n", (int)s.len, s.chars);
                    }
                }
            }
        }
    }
}

void emit_helpers() {
    emit(0, "write_num:");
    emit(8, "sub     rsp, 40");
    emit(8, "xor     r10d, r10d");
    emit(8, "test    rdi, rdi");
    emit(8, "jns     .L2");
    emit(8, "neg     rdi");
    emit(8, "mov     r10d, 1");
    emit(0, ".L2:");
    emit(8, "mov     QWORD [rsp+8], 0");
    emit(8, "mov     ecx, 1");
    emit(8, "mov     r9, 7378697629483820647");
    emit(8, "mov     QWORD [rsp+16], 0");
    emit(8, "mov     QWORD [rsp+24], 0");
    emit(0, ".L3:");
    emit(8, "mov     rax, rdi");
    emit(8, "mov     rsi, rcx");
    emit(8, "add     rcx, 1");
    emit(8, "imul    r9");
    emit(8, "mov     rax, rdi");
    emit(8, "mov     r8, rsi");
    emit(8, "sar     rax, 63");
    emit(8, "not     r8");
    emit(8, "sar     rdx, 2");
    emit(8, "sub     rdx, rax");
    emit(8, "lea     rax, [rdx+rdx*4]");
    emit(8, "add     rax, rax");
    emit(8, "sub     rdi, rax");
    emit(8, "add     edi, 48");
    emit(8, "mov     BYTE [rsp+32+r8], dil");
    emit(8, "mov     rdi, rdx");
    emit(8, "test    rdx, rdx");
    emit(8, "jne     .L3");
    emit(8, "test    r10d, r10d");
    emit(8, "je      .L4");
    emit(8, "not     rcx");
    emit(8, "mov     BYTE [rsp+32+rcx], 45");
    emit(8, "lea     rcx, [rsi+2]");
    emit(0, ".L4:");
    emit(8, "mov     eax, 24");
    emit(8, "mov     rdx, rcx");
    emit(8, "mov     edi, 1");
    emit(8, "sub     rax, rcx");
    emit(8, "lea     rsi, [rsp+8+rax]");
    emit(8, "mov     rax, 1");
    emit(8, "syscall");
    emit(8, "add     rsp, 40");
    emit(8, "ret");

    emit(0, "print_value:");
    emit(8, "mov rcx, op1_metadata");
    emit(8, "and rcx, 3");

    emit(8, "cmp rcx, VALUE_NUMBER");
    emit(8, "je print_num");
    emit(8, "cmp rcx, VALUE_STRING");
    emit(8, "je print_str");
    emit(8, "cmp rcx, VALUE_ARRAY");
    emit(8, "je print_arr");

    emit(0, "print_num:");
    emit(8, "mov rdi, op1_value");
    emit(8, "call write_num");
    emit(8, "jmp print_done");

    emit(0, "print_str:");
    emit(8, "shr op1_metadata, 4");
    emit(8, "test rdi, rdi");
    emit(8, "jz write_quotes");
    emit(8, "WRITE op1_value, op1_metadata");
    emit(8, "jmp print_done");
    emit(0, "write_quotes:");
    emit(8, "WRITE STR_QUOTE, 1");
    emit(8, "WRITE op1_value, op1_metadata");
    emit(8, "WRITE STR_QUOTE, 1");
    emit(8, "jmp print_done");

    emit(0, "print_arr:");
    emit(8, "WRITE STR_ARRAY_START, 1");
    emit(8, "mov rax, op1_value");
    emit(8, "mov rbx, op1_metadata");
    emit(8, "shr rbx, 36");
    emit(8, "sub rbx, 1");
    emit(8, "imul rbx, 16");
    emit(8, "mov rcx, rax");
    emit(8, "add rcx, rbx");

    emit(0, "arr_start:");
    emit(8, "cmp rax, rcx");
    emit(8, "jge arr_done");
    emit(8, "mov op2_metadata, [rax]");
    emit(8, "mov op2_value, [rax + 8]");

    emit(8, "push rax");
    emit(8, "push rbx");
    emit(8, "push rcx");
    emit(8, "push op1_value");
    emit(8, "push op1_metadata");
    emit(8, "mov op1_value, op2_value");
    emit(8, "mov op1_metadata, op2_metadata");
    emit(8, "mov rdi, op1_metadata");
    emit(8, "and rdi, 3");
    emit(8, "xor rdi, VALUE_STRING");
    emit(8, "call print_value");
    emit(8, "WRITE STR_ARRAY_SEP, 2");
    emit(8, "pop op1_metadata");
    emit(8, "pop op1_value");
    emit(8, "pop rcx");
    emit(8, "pop rbx");
    emit(8, "pop rax");
    emit(8, "add rax, 16");
    emit(8, "jmp arr_start");

    emit(0, "arr_done:");
    emit(8, "mov op1_metadata, [rax]");
    emit(8, "mov op1_value, [rax + 8]");
    emit(8, "mov rdi, op1_metadata");
    emit(8, "and rdi, 3");
    emit(8, "xor rdi, VALUE_STRING");
    emit(8, "call print_value");
    emit(8, "WRITE STR_ARRAY_END, 1");

    emit(0, "print_done:");
    emit(8, "ret");
}

void emit_asm(VM *vm) {
    f = fopen("out.asm", "w");

    funcs = vm->funcs;

    emit_header();

    for (int i = 0; i < vm->funcs.index; i++)
        emit_func(
            vm->funcs.data[i].name,
            vm->funcs.data[i].code,
            vm->funcs.data[i].locs,
            vm->funcs.data[i].constants
        );

    emit_helpers();

    emit_footer(&vm->funcs.data[0]);

    fclose(f);
}
