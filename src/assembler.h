#pragma once

#define read_index (COMBYTE(code.data[i + 1], code.data[i + 2]))
#define emit_op_comment(_op) emit(0, "%s_label_%d: ; " #_op " %s:%ld", name, i, locs.data[i].file, locs.data[i].line)
#define VALUE_NUMBER 0
#define VALUE_STRING 1
#define VALUE_ARRAY 2
#define HEAP_SIZE 4096

#define emit_binary_op(_op)                                                                             \
    emit(8, "pop op2_value");                                                                           \
    emit(8, "pop op2_metadata");                                                                        \
    emit(8, "pop op1_value");                                                                           \
    emit(8, "pop op1_metadata");                                                                        \
                                                                                                        \
    emit(8, "mov rax, op1_metadata");                                                                   \
    emit(8, "mov rbx, op2_metadata");                                                                   \
    emit(8, "and rax, 3");                                                                              \
    emit(8, "and rbx, 3");                                                                              \
    emit(8, "xor rax, rbx");                                                                            \
    emit(8, "cmp rax, 0");                                                                              \
    emit(8, "jnz %s_%d_error", name, i);                                                                \
                                                                                                        \
    emit(8, "cmp rax, VALUE_NUMBER");                                                                   \
    emit(8, "jne %s_%d_error", name, i);                                                                \
                                                                                                        \
    emit(8, "mov rax, op1_value");                                                                      \
    emit(8, #_op " rax, op2_value");                                                                    \
    emit(8, "push 0");                                                                                  \
    emit(8, "push rax");                                                                                \
    emit(8, "jmp %s_%d_done", name, i);                                                                 \
                                                                                                        \
    emit(0, "%s_%d_error:", name, i);                                                                   \
    emit_error("ERROR in %s on line %ld: cant " #_op " those", locs.data[i].file, locs.data[i].line);   \
    emit(0, "%s_%d_done:", name, i);                                                                    \

Functions funcs;

void emit_header() {
    emit(0, "VALUE_NUMBER equ 0");
    emit(0, "VALUE_STRING equ 1");
    emit(0, "VALUE_ARRAY equ 2");
    emit(0, "op1_value equ r8");
    emit(0, "op1_metadata equ r9");
    emit(0, "op2_value equ r10");
    emit(0, "op2_metadata equ r11");
    emit(0, "suda_sp equ r15");
    emit(0, "suda_bp equ r14");
    emit(0, "ALLOC_PTR equ r13");
    emit(0, "HEAP_SIZE equ %d", HEAP_SIZE);

    emit(0, "macro suda_push _value, _metadata {");
    emit(8, "mov rbp, _metadata");
    emit(8, "mov [suda_sp], rbp");
    emit(8, "add suda_sp, 8");
    emit(8, "mov rbp, _value");
    emit(8, "mov [suda_sp], rbp");
    emit(8, "add suda_sp, 8");
    emit(0, "}");
    emit(0, "macro suda_pop _value, _metadata {");
    emit(8, "sub suda_sp, 8");
    emit(8, "mov rbp, [suda_sp]");
    emit(8, "mov _value, rbp");
    emit(8, "sub suda_sp, 8");
    emit(8, "mov rbp, [suda_sp]");
    emit(8, "mov _metadata, rbp");
    emit(0, "}");
    
    emit(0, "macro error {");
    emit(8, "WRITE STR_ERROR, 5");
    emit(8, "WRITE STR_NEWLINE, 1");
    emit(8, "mov rax, 60");
    emit(8, "mov rdi, 1");
    emit(8, "syscall");
    emit(0, "}");

    emit(0, "macro ERROR error_msg {");
    emit(8, "mov rsi, error_msg");
    emit(8, "mov rdi, 1");
    emit(8, "mov dl, [error_msg#_len]");
    emit(8, "movzx rdx, dl");
    emit(8, "mov rax, 1");
    emit(8, "syscall");
    emit(8, "mov rax, 60");
    emit(8, "mov rdi, 1");
    emit(8, "syscall");
    emit(0, "}");

    emit(0, "macro DEBUG debug_msg {");
    push_all_reg();
    emit(8, "mov rsi, debug_msg");
    emit(8, "mov rdi, 1");
    emit(8, "mov dl, [debug_msg#_len]");
    emit(8, "movzx rdx, dl");
    emit(8, "mov rax, 1");
    emit(8, "syscall");
    pop_all_reg();
    emit(0, "}");

    emit(0, "macro PRINTLN_NUM _num {");
    push_all_reg();
    emit(8, "mov rdi, _num");
    emit(8, "call write_num");
    emit(8, "WRITE STR_NEWLINE, 1");
    pop_all_reg();
    emit(0, "}");

    emit(0, "macro WRITE _val, _len {");
    emit(8, "pushfq");   
    emit(8, "push rax"); 
    emit(8, "push rdx"); 
    emit(8, "push rsi"); 
    emit(8, "push rdi"); 
    emit(8, "push r11"); 

    emit(8, "mov rdi, 1");
    emit(8, "mov rsi, _val");
    emit(8, "mov rdx, _len");
    emit(8, "mov rax, 1");
    emit(8, "syscall");

    emit(8, "pop r11"); 
    emit(8, "pop rdi"); 
    emit(8, "pop rsi"); 
    emit(8, "pop rdx"); 
    emit(8, "pop rax"); 
    emit(8, "popfq");
    emit(0, "}");

    emit(0, "format ELF64 executable 3");
    emit(0, "segment readable executable");
    emit(0, "entry start");
    emit(0, "start:");
    emit(8, "mov suda_sp, SUDA_STACK");
    emit(8, "mov suda_bp, SUDA_STACK");
    emit(8, "mov rsi, HEAP_SIZE");
    emit(8, "mov rdx, 3");
    emit(8, "mov r10, 0x22");
    emit(8, "mov r8, -1");
    emit(8, "mov r9, 0");
    emit(8, "mov rax, 9");
    emit(8, "syscall");
    emit(8, "mov [FROM_SPACE], rax");
    emit(8, "mov rax, 9");
    emit(8, "syscall");
    emit(8, "mov [TO_SPACE], rax");
    emit(8, "mov rax, [FROM_SPACE]");
    emit(8, "mov [HEAP_START], rax");
    emit(8, "mov rax, [FROM_SPACE]");
    emit(8, "mov ALLOC_PTR, rax");
    emit(8, "mov rax, [FROM_SPACE]");
    emit(8, "add rax, HEAP_SIZE");
    emit(8, "mov [HEAP_END], rax");
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

    for (int i = 0; i < code.index; i++) {
        switch (code.data[i]) {
            case OP_CONSTANT:
                emit_op_comment(OP_CONSTANT);
                Value val = constants.data[read_index];
                switch (val.type) {
                    case Value_Number:
                        emit(8, "suda_push %ld, %ld", val.val.num, serialize_constant(val));
                        break;
                    case Value_String:
                        emit(8, "suda_push STR_%s_%d, %ld", name, read_index, serialize_constant(val));
                        break;
                    default:
                        ERR("ERROR in %s on line %ld: cant emit asm for value type %s\n", locs.data[i].file, locs.data[i].line, find_value_type(val.type))
                }
                i += 2;
                break;
            case OP_ARRAY:
                emit_op_comment(OP_ARRAY);
                emit(8, "mov rax, %ld", serialize_constant(constants.data[read_index].val.array[0]));
                emit(8, "suda_push rax, ARR_%s_%d", name, read_index);
                i += 2;
                break;
            case OP_PRINTLN:
                emit_op_comment(OP_PRINTLN);
                emit(8, "suda_pop op1_value, op1_metadata");
                emit(8, "mov rdi, 1"); // dont print strings with quotes around them if they are not in array
                emit(8, "call print_value");
                emit(8, "WRITE STR_NEWLINE, 1");
                break;
            case OP_ADD:
                emit_op_comment(OP_ADD);
                emit(8, "suda_pop op2_value, op2_metadata");
                emit(8, "suda_pop op1_value, op1_metadata");
                
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
                emit(8, "suda_push rax, 0");
                emit(8, "jmp %s_%d_done", name, i);

                emit(0, "%s_%d_add_str:", name, i);
                emit(8, "mov rdi, op1_metadata");
                emit(8, "mov rax, op2_metadata");
                emit(8, "shr rdi, 4");
                emit(8, "shr rax, 4");
                emit(8, "add rdi, rax");
                emit(8, "mov rsi, rdi");
                emit(8, "call alloc");
                emit(8, "push rax");
                
                emit(8, "mov rsi, rax");
                emit(8, "mov rdi, op1_value");
                emit(8, "mov rdx, op1_metadata");
                emit(8, "shr rdx, 4");
                emit(8, "call memcpy");

                emit(8, "mov rdi, op2_value");
                emit(8, "mov rdx, op1_metadata");
                emit(8, "shr rdx, 4");
                emit(8, "add rsi, rdx");
                emit(8, "call memcpy");
                
                emit(8, "mov rbx, op1_metadata");
                emit(8, "mov rax, op2_metadata");
                emit(8, "shr rbx, 4");
                emit(8, "shr rax, 4");
                emit(8, "add rbx, rax");
                emit(8, "shl rbx, 4");
                emit(8, "or rbx, VALUE_STRING");
                
                emit(8, "pop rax");
                emit(8, "suda_push rax, rbx");

                emit(8, "jmp %s_%d_done", name, i);
                
                emit(0, "%s_%d_add_array:", name, i);

                emit(0, "%s_%d_error:", name, i);
                emit_error("ERROR in %s on line %ld: cant add those", locs.data[i].file, locs.data[i].line);
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
                emit(8, "suda_pop op2_value, op2_metadata");
                emit(8, "suda_pop op1_value, op1_metadata");
                
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
                emit(8, "suda_push 0, 0");
                emit(8, "jmp %s_%d_done", name, i);
                emit(0, "%s_%d_true:", name, i);
                emit(8, "suda_push 1, 0");
                emit(8, "jmp %s_%d_done", name, i);

                emit(0, "%s_%d_error:", name, i);
                emit_error("ERROR in %s on line %ld: cant is equal those", locs.data[i].file, locs.data[i].line);
                emit(0, "%s_%d_done:", name, i);
                break;
            case OP_LESS:
                emit_op_comment(OP_LESS);
                emit(8, "suda_pop op2_value, op2_metadata");
                emit(8, "suda_pop op1_value, op1_metadata");

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
                emit(8, "suda_push rax, 0");
                emit(8, "jmp %s_%d_done", name, i);
                
                emit(0, "%s_%d_error:", name, i);
                emit_error("ERROR in %s on line %ld: cant less than those", locs.data[i].file, locs.data[i].line);
                emit(0, "%s_%d_done:", name, i);
                break;
            case OP_LESS_EQUAL:
                emit_op_comment(OP_LESS_EQUAL);
                emit(8, "suda_pop op2_value, op2_metadata");
                emit(8, "suda_pop op1_value, op1_metadata");

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
                emit(8, "suda_push rax, 0");
                emit(8, "jmp %s_%d_done", name, i);
                
                emit(0, "%s_%d_error:", name, i);
                emit_error("ERROR in %s on line %ld: cant less equal those", locs.data[i].file, locs.data[i].line);
                emit(0, "%s_%d_done:", name, i);
                break;
            case OP_GREATER:
                emit_op_comment(OP_GREATER);
                emit(8, "suda_pop op2_value, op2_metadata");
                emit(8, "suda_pop op1_value, op1_metadata");

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
                emit(8, "suda_push rax, 0");
                emit(8, "jmp %s_%d_done", name, i);
                
                emit(0, "%s_%d_error:", name, i);
                emit_error("ERROR in %s on line %ld: cant greater than those", locs.data[i].file, locs.data[i].line);
                emit(0, "%s_%d_done:", name, i);
                break;
            case OP_GREATER_EQUAL:
                emit_op_comment(OP_GREATER_EQUAL);
                emit(8, "suda_pop op2_value, op2_metadata");
                emit(8, "suda_pop op1_value, op1_metadata");

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
                emit(8, "suda_push rax, 0");
                emit(8, "jmp %s_%d_done", name, i);
                
                emit(0, "%s_%d_error:", name, i);
                emit_error("ERROR in %s on line %ld: cant greater equal those", locs.data[i].file, locs.data[i].line);
                emit(0, "%s_%d_done:", name, i);
                break;
            case OP_DEFINE_GLOBAL:
                emit_op_comment(OP_DEFINE_GLOBAL);
                emit(8, "suda_pop [%s], [%s + 8]", constants.data[read_index].val.str.chars, constants.data[read_index].val.str.chars);
                i += 2;
                break;
            case OP_GET_GLOBAL:
                emit_op_comment(OP_GET_GLOBAL);
                emit(8, "suda_push [%s], [%s + 8]", constants.data[read_index].val.str.chars, constants.data[read_index].val.str.chars);
                i += 2;
                break;
            case OP_SET_GLOBAL:
                emit_op_comment(OP_SET_GLOBAL);
                emit(8, "suda_pop [%s], [%s + 8]", constants.data[read_index].val.str.chars, constants.data[read_index].val.str.chars);
                i += 2;
                break;
            case OP_START_IF:
                emit_op_comment(OP_START_IF);
                emit(8, "suda_pop op1_value, op1_metadata");
                emit(8, "cmp op1_value, 0");
                emit(8, "jz %s_label_%d", name, i + read_index);
                i += 2;
                break;
            case OP_JUMP_IF_FALSE:
                emit_op_comment(OP_JUMP_IF_FALSE);
                emit(8, "suda_pop op1_value, op1_metadata");
                emit(8, "cmp op1_value, 1");
                emit(8, "jnz %s_label_%d", name, i + read_index);
                i += 2;
                break;
            case OP_JUMP:
                emit_op_comment(OP_JUMP);
                emit(8, "jmp %s_label_%d", name, read_index);
                i += 2;
                break;
            case OP_GET_LOCAL:
                emit_op_comment(OP_GET_LOCAL);
                i++;
                emit(8, "suda_push [suda_bp + %d], [suda_bp + %d]", 16 * code.data[i] + 8, 16 * code.data[i]);
                break;
            case OP_SET_LOCAL:
                emit_op_comment(OP_SET_LOCAL);
                i++;
                emit(8, "suda_pop [suda_bp + %d], [suda_bp + %d]", 16 * code.data[i] + 8, 16 * code.data[i]);
                break;
            case OP_POP:
                emit_op_comment(OP_POP);
                emit(8, "sub suda_sp, %d", 16 * read_index);
                i += 2;
                break;
            case OP_GET_GLOBAL_GET_CONSTANT:{
                emit_op_comment(OP_GET_GLOBAL_GET_CONSTANT);
                emit(8, "suda_push [%s], [%s + 8]", constants.data[read_index].val.str.chars, constants.data[read_index].val.str.chars);
                i += 2;
                Value val = constants.data[read_index];
                switch (val.type) {
                    case Value_Number:
                        emit(8, "suda_push %ld, %ld", val.val.num, serialize_constant(val));
                        break;
                    case Value_String:
                        emit(8, "suda_push STR_%s_%d, %ld", name, read_index, serialize_constant(val));
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
                switch (val.type) {
                    case Value_Number:
                        emit(8, "suda_push %ld, %ld", val.val.num, serialize_constant(val));
                        break;
                    case Value_String:
                        emit(8, "suda_push STR_%s_%d, %ld", name, read_index, serialize_constant(val));
                        break;
                    default:
                        ERR("ERROR in %s on line %ld: cant emit asm for value type %s\n", locs.data[i].file, locs.data[i].line, find_value_type(val.type))
                }
                i += 2;
                break;}
            case OP_CALL:
                emit_op_comment(OP_CALL);
                emit(8, "push suda_bp");
                emit(8, "mov suda_bp, suda_sp");
                emit(8, "sub suda_bp, %d", funcs.data[read_index].arity * 16);
                emit(8, "call %s_label_0", funcs.data[read_index].name);
                emit(8, "pop suda_bp");
                emit(8, "cmp rax, 1");
                emit(8, "je %s_%d_no_return_value", name, i);
                emit(8, "suda_push op1_value, op1_metadata");
                emit(0, "%s_%d_no_return_value:", name, i);
                i += 2;
                break;
            case OP_RETURN:
                emit_op_comment(OP_RETURN);
                emit(8, "suda_pop op1_value, op1_metadata");
                emit(8, "ret");
                break;
            case OP_RETURN_NOTHING:
                emit_op_comment(OP_RETURN_NOTHING);
                emit(8, "mov rax, 1");
                emit(8, "ret");
                break;
            case OP_DONE:
                emit_op_comment(OP_DONE);
                emit(8, "mov rax, 60");
                emit(8, "mov rdi, 0");
                emit(8, "syscall");
                break;
            case OP_NOT:
                emit_op_comment(OP_NOT);
                emit(8, "suda_pop op1_value, op1_metadata");
                emit(8, "test op1_value, op1_value");
                emit(8, "setz al");
                emit(8, "xor al, 1");
                emit(8, "movzx rax, al");
                emit(8, "suda_push rax, 0");
                break;
            case OP_MODULO:
                emit_op_comment(OP_MODULO);
                emit(8, "suda_pop op2_value, op2_metadata");
                emit(8, "suda_pop op1_value, op1_metadata");
                emit(8, "mov rdi, op2_value");
                emit(8, "mov rdx, 0");
                emit(8, "div op1_value");
                emit(8, "suda_push rdx, 0");
                break;
            case OP_OR:
                emit_op_comment(OP_OR);
                emit(8, "suda_pop op2_value, op2_metadata");
                emit(8, "suda_pop op1_value, op1_metadata");
                emit(8, "mov rax, op1_value");
                emit(8, "or rax, op2_value");
                emit(8, "suda_push rax, 0");
                break;
            case OP_AND:
                emit_op_comment(OP_AND);
                emit(8, "suda_pop op2_value, op2_metadata");
                emit(8, "suda_pop op1_value, op1_metadata");
                emit(8, "mov rax, op1_value");
                emit(8, "and rax, op2_value");
                emit(8, "suda_push rax, 0");
                break;
            case OP_BIT_OR:
                emit_op_comment(OP_BIT_OR);
                emit(8, "suda_pop op2_value, op2_metadata");
                emit(8, "suda_pop op1_value, op1_metadata");
                emit(8, "mov rax, op1_value");
                emit(8, "or rax, op2_value");
                emit(8, "suda_push rax, 0");
                break;
            case OP_BIT_AND:
                emit_op_comment(OP_BIT_AND);
                emit(8, "suda_pop op2_value, op2_metadata");
                emit(8, "suda_pop op1_value, op1_metadata");
                emit(8, "mov rax, op1_value");
                emit(8, "and rax, op2_value");
                emit(8, "suda_push rax, 0");
                break;
            case OP_BIT_XOR:
                emit_op_comment(OP_BIT_XOR);
                emit(8, "suda_pop op2_value, op2_metadata");
                emit(8, "suda_pop op1_value, op1_metadata");
                emit(8, "mov rax, op1_value");
                emit(8, "xor rax, op2_value");
                emit(8, "suda_push rax, 0");
                break;
            case OP_BIT_NOT:
                emit_op_comment(OP_BIT_NOT);
                emit(8, "suda_pop op1_value, op1_metadata");
                emit(8, "not op1_value");
                emit(8, "suda_push op1_value, 0");
                break;
            case OP_LSHIFT:
                emit_op_comment(OP_LSHIFT);
                emit(8, "suda_pop op2_value, op2_metadata");
                emit(8, "suda_pop op1_value, op1_metadata");
                emit(8, "mov rax, op1_value");
                emit(8, "sal rax, op2_value");
                emit(8, "suda_push rax, 0");
                break;
            case OP_RSHIFT:
                emit_op_comment(OP_RSHIFT);
                emit(8, "suda_pop op2_value, op2_metadata");
                emit(8, "suda_pop op1_value, op1_metadata");
                emit(8, "mov rax, op1_value");
                emit(8, "sar rax, op2_value");
                emit(8, "suda_push rax, 0");
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
    emit(0, "FROM_SPACE: dq 0");
    emit(0, "TO_SPACE: dq 0");
    emit(0, "HEAP_START: rb 8");
    emit(0, "HEAP_END: rb 8");
    emit(0, "FREE_PTR: rb 8");
    emit(0, "SCAN_PTR: rb 8");
    emit(0, "GLOBALS_START:");

    //globals
    for (int i = 0; i < func->code.index; i += op_offsets[func->code.data[i]]) {
        if (func->code.data[i] == OP_DEFINE_GLOBAL) {
            emit(0, "%s: rb 16", func->constants.data[COMBYTE(func->code.data[i + 1], func->code.data[i + 2])].val.str.chars);
        }
    }
    emit(0, "GLOBALS_END:");

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

    //error messages
    for (int i = 0; i < error_msgs.index; i++) {
        emit(0, "ERROR_%d: db \"%s\", 10, 0", i, error_msgs.data[i]);
        emit(0, "ERROR_%d_len: db %ld", i, strlen(error_msgs.data[i]) + 1);
    }
    for (int i = 0; i < debug_msgs.index; i++) {
        int len = strlen(debug_msgs.data[i]);
        if (debug_msgs.data[i][len - 1] == '\n') {
            emit(0, "DEBUG_%d: db \"%.*s\", 10, 0", i, len - 1, debug_msgs.data[i]);
            emit(0, "DEBUG_%d_len: db %d", i, len);
        } else {
            emit(0, "DEBUG_%d: db \"%s\", 0", i, debug_msgs.data[i]);
            emit(0, "DEBUG_%d_len: db %d", i, len);
        }
    }
    emit(0, "SUDA_STACK: rb %d*16", STACK_SIZE);
}

void emit_asm(VM *vm) {
    error_msgs = (String_Array){
        calloc(10, sizeof(char*)),
        0,
        10
    };
    debug_msgs = (String_Array){
        calloc(10, sizeof(char*)),
        0,
        10
    };

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

    for (int i = 0; i < error_msgs.index; i++)
        free(error_msgs.data[i]);
    free(error_msgs.data);
    for (int i = 0; i < debug_msgs.index; i++)
        free(debug_msgs.data[i]);
    free(debug_msgs.data);
    fclose(f);
}
