#pragma once

#define read_index (COMBYTE(code.data[i + 1], code.data[i + 2]))
#define emit_op_comment(_name, _index, _op) emit(0, "%s_label_%d: ; " #_op " %s:%ld", _name, _index, locs.data[i].file, locs.data[i].line)

int op_offsets[] = {
    3, //OP_CONSTANT
    3, //OP_ARRAY
    1, //OP_PRINTLN
    1, //OP_PRINT
    3, //OP_DEFINE_GLOBAL
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

void emit(int indent, char *fmt, ...) {
    fprintf(f, "%*s", indent, "");
    va_list args;
    va_start(args, fmt);
    vfprintf(f, fmt, args);
    va_end(args);
    fprintf(f, "\n");
}

void emit_header() {
    fprintf(f, "format ELF64 executable 3\n");
    fprintf(f, "segment readable executable\n");
    fprintf(f, "entry start\n");
    fprintf(f, "start:\n");
}

void emit_func(char *name, Code code, Locations locs, Constants constants) {
    emit(0, "; FUNC %s:", name);
    for (int i = 0; i < code.index; i++) {
        switch (code.data[i]) {
            case OP_CONSTANT:
                emit_op_comment(name, i, OP_CONSTANT);
                emit(8, "push %ld", constants.data[read_index].val.num);
                i += 2;
                break;
            case OP_PRINTLN:
                emit_op_comment(name, i, OP_PRINTLN);
                emit(8, "pop rdi");
                emit(8, "call println_int");
                break;
            case OP_ADD:
                emit_op_comment(name, i, OP_ADD);
                emit(8, "pop rax");
                emit(8, "pop rbx");
                emit(8, "add rax, rbx");
                emit(8, "push rax");
                break;
            case OP_SUBTRACT:
                emit_op_comment(name, i, OP_SUBTRACT);
                emit(8, "pop rbx");
                emit(8, "pop rax");
                emit(8, "sub rax, rbx");
                emit(8, "push rax");
                break;
            case OP_MULTIPLY:
                emit_op_comment(name, i, OP_MULTIPLY);
                emit(8, "pop rbx");
                emit(8, "pop rax");
                emit(8, "imul rax, rbx");
                emit(8, "push rax");
                break;
            case OP_DIVIDE:
                emit_op_comment(name, i, OP_DIVIDE);
                emit(8, "mov rdx, 0");
                emit(8, "pop rbx");
                emit(8, "pop rax");
                emit(8, "idiv rbx");
                emit(8, "push rax");
                break;
            case OP_GREATER:
                emit_op_comment(name, i, OP_GREATER);
                emit(8, "pop rbx");
                emit(8, "pop rax");
                emit(8, "cmp rax, rbx");
                emit(8, "setg al");
                emit(8, "movzx rax, al");
                emit(8, "push rax");
                break;
            case OP_GREATER_EQUAL:
                emit_op_comment(name, i, OP_GREATER_EQUAL);
                emit(8, "pop rbx");
                emit(8, "pop rax");
                emit(8, "cmp rax, rbx");
                emit(8, "setge al");
                emit(8, "movzx rax, al");
                emit(8, "push rax");
                break;
            case OP_LESS:
                emit_op_comment(name, i, OP_LESS);
                emit(8, "pop rbx");
                emit(8, "pop rax");
                emit(8, "cmp rax, rbx");
                emit(8, "setl al");
                emit(8, "movzx rax, al");
                emit(8, "push rax");
                break;
            case OP_LESS_EQUAL:
                emit_op_comment(name, i, OP_LESS_EQUAL);
                emit(8, "pop rbx");
                emit(8, "pop rax");
                emit(8, "cmp rax, rbx");
                emit(8, "setle al");
                emit(8, "movzx rax, al");
                emit(8, "push rax");
                break;
            case OP_IS_EQUAL:
                emit_op_comment(name, i, OP_IS_EQUAL);
                emit(8, "pop rbx");
                emit(8, "pop rax");
                emit(8, "cmp rax, rbx");
                emit(8, "sete al");
                emit(8, "movzx rax, al");
                emit(8, "push rax");
                break;
            case OP_NOT_EQUAL:
                emit_op_comment(name, i, OP_NOT_EQUAL);
                emit(8, "pop rbx");
                emit(8, "pop rax");
                emit(8, "cmp rax, rbx");
                emit(8, "setne al");
                emit(8, "movzx rax, al");
                emit(8, "push rax");
                break;
            case OP_START_IF:
                emit_op_comment(name, i, OP_START_IF);
                emit(8, "pop rax");
                emit(8, "cmp rax, 0");
                emit(8, "jz %s_label_%d", name, i + read_index);
                i += 2;
                break;
            case OP_JUMP_IF_FALSE:
                emit_op_comment(name, i, OP_JUMP_IF_FALSE);
                emit(8, "pop rax");
                emit(8, "cmp rax, 1");
                emit(8, "jnz %s_label_%d", name, i + read_index);
                i += 2;
                break;
            case OP_JUMP:
                emit_op_comment(name, i, OP_JUMP);
                emit(8, "jmp %s_label_%d", name, read_index);
                i += 2;
                break;
            case OP_DONE:
                emit_op_comment(name, i, OP_DONE);
                emit(8, "mov rax, 60");
                emit(8, "mov rdi, 0");
                emit(8, "syscall");
                break;
            case OP_DEFINE_GLOBAL:
                emit_op_comment(name, i, OP_DEFINE_GLOBAL);
                emit(8, "pop qword [%s]", constants.data[read_index].val.str.chars);
                i += 2;
                break;
            case OP_GET_GLOBAL:
                emit_op_comment(name, i, OP_GET_GLOBAL);
                emit(8, "push qword [%s]", constants.data[read_index].val.str.chars);
                i += 2;
                break;
            default:
                ERR("ERROR in %s on line %ld: cant emit asm for op type %s\n", locs.data[i].file, locs.data[i].line, find_op_code(code.data[i]))
                break;
        }
    }
}

void emit_globals(Function *func) {
    emit(0, "segment readable writeable");
    for (int i = 0; i < func->code.index; i += op_offsets[func->code.data[i]]) {
        if (func->code.data[i] == OP_DEFINE_GLOBAL) {
            emit(8, "%s: dq 0", func->constants.data[COMBYTE(func->code.data[i + 1], func->code.data[i + 2])].val.str.chars);
        }
    }
}

void emit_footer() {
    fprintf(f, "println_int:\n");
    fprintf(f, "        sub     rsp, 40\n");
    fprintf(f, "        xor     r10d, r10d\n");
    fprintf(f, "        test    rdi, rdi\n");
    fprintf(f, "        jns     .L2\n");
    fprintf(f, "        neg     rdi\n");
    fprintf(f, "        mov     r10d, 1\n");
    fprintf(f, ".L2:\n");
    fprintf(f, "        mov     QWORD [rsp+24], 0\n");
    fprintf(f, "        mov     ecx, 1\n");
    fprintf(f, "        mov     r9, 7378697629483820647\n");
    fprintf(f, "        mov     QWORD [rsp+8], 0\n");
    fprintf(f, "        mov     QWORD [rsp+16], 0\n");
    fprintf(f, "        mov     BYTE [rsp+31], 10\n");
    fprintf(f, ".L3:\n");
    fprintf(f, "        mov     rax, rdi\n");
    fprintf(f, "        mov     rsi, rcx\n");
    fprintf(f, "        add     rcx, 1\n");
    fprintf(f, "        imul    r9\n");
    fprintf(f, "        mov     rax, rdi\n");
    fprintf(f, "        mov     r8, rsi\n");
    fprintf(f, "        sar     rax, 63\n");
    fprintf(f, "        not     r8\n");
    fprintf(f, "        sar     rdx, 2\n");
    fprintf(f, "        sub     rdx, rax\n");
    fprintf(f, "        lea     rax, [rdx+rdx*4]\n");
    fprintf(f, "        add     rax, rax\n");
    fprintf(f, "        sub     rdi, rax\n");
    fprintf(f, "        add     edi, 48\n");
    fprintf(f, "        mov     BYTE [rsp+32+r8], dil\n");
    fprintf(f, "        mov     rdi, rdx\n");
    fprintf(f, "        test    rdx, rdx\n");
    fprintf(f, "        jne     .L3\n");
    fprintf(f, "        test    r10d, r10d\n");
    fprintf(f, "        je      .L4\n");
    fprintf(f, "        not     rcx\n");
    fprintf(f, "        mov     BYTE [rsp+32+rcx], 45\n");
    fprintf(f, "        lea     rcx, [rsi+2]\n");
    fprintf(f, ".L4:\n");
    fprintf(f, "        mov     eax, 24\n");
    fprintf(f, "        mov     rdx, rcx\n");
    fprintf(f, "        mov     edi, 1\n");
    fprintf(f, "        sub     rax, rcx\n");
    fprintf(f, "        lea     rsi, [rsp+8+rax]\n");
    fprintf(f, "        mov     rax, 1\n");
    fprintf(f, "        syscall\n");
    fprintf(f, "        add     rsp, 40\n");
    fprintf(f, "        ret\n");
}

void emit_asm(VM *vm) {
    f = fopen("out.asm", "w"),

    emit_header();

    for (int i = 0; i < vm->funcs.index; i++)
        emit_func(
            vm->funcs.data[i].name,
            vm->funcs.data[i].code,
            vm->funcs.data[i].locs,
            vm->funcs.data[i].constants
        );

    emit_footer();

    emit_globals(&vm->funcs.data[0]);

    fclose(f);

    system("fasm out.asm out > /dev/null");
}
