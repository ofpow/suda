#pragma once

#define read_index (COMBYTE(code.data[i + 1], code.data[i + 2]))
#define emit_op_comment(_op) emit(0, "%s_label_%d: ; " #_op " %s:%ld", name, i, locs.data[i].file, locs.data[i].line)
#define VALUE_NUMBER 0
#define VALUE_STRING 1
#define VALUE_ARRAY 2

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
    emit(0, "format ELF64 executable 3");
    emit(0, "segment readable executable");
    emit(0, "entry start");
    emit(0, "start:");
    emit(8, "mov rbp, rsp");
    emit(8, "lea r15, [call_stack + 64*16]");
}

int64_t serialize_constant(Value val) {
    switch (val.type) {
        case Value_Number:
            return 0;
        case Value_String:
            int info = val.val.str.len;
            info = (info << 4);
            info = (info | VALUE_STRING);
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
            case OP_PRINTLN:
                emit_op_comment(OP_PRINTLN);
                emit(8, "pop rax"); // value
                emit(8, "pop rbx"); // metadata
                emit(8, "mov rcx, rbx");
                emit(8, "and rcx, 3");
                emit(8, "cmp rcx, VALUE_NUMBER");
                emit(8, "je %s_%d_println_num", name, i);
                emit(8, "cmp rcx, VALUE_STRING");
                emit(8, "je %s_%d_println_str", name, i);


                emit(0, "%s_%d_println_num:", name, i);
                emit(8, "mov rdi, rax");
                emit(8, "call println_num");
                emit(8, "jmp %s_%d_done", name, i);

                emit(0, "%s_%d_println_str:", name, i);
                emit(8, "shr rbx, 4");
                emit(8, "mov rdx, rbx");
                emit(8, "mov rsi, rax");
                emit(8, "mov rax, 1");
                emit(8, "syscall");
                emit(8, "mov rsi, STR_NEWLINE");
                emit(8, "mov rdx, 1");
                emit(8, "mov rax, 1");
                emit(8, "syscall");

                emit(0, "%s_%d_done:", name, i);
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

    //globals
    for (int i = 0; i < func->code.index; i += op_offsets[func->code.data[i]]) {
        if (func->code.data[i] == OP_DEFINE_GLOBAL) {
            emit(0, "%s: dq 0", func->constants.data[COMBYTE(func->code.data[i + 1], func->code.data[i + 2])].val.str.chars);
        }
    }

    //string constants
    for (int i = 0; i < funcs.index; i++) {
        Function *func = &funcs.data[i];
        for (int j = 0; j < func->constants.index; j++) {
            if (func->constants.data[j].type == Value_String) {

                fprintf(f, "STR_%s_%d: db ", func->name, j);
                string s = func->constants.data[j].val.str;

                for (size_t k = 0; k < s.len; k++)
                    fprintf(f, "%d, ", s.chars[k]);

                fprintf(f, "0\n");
            }
        }
    }
}

void emit_helpers() {
    emit(0, "println_num:");
    emit(8, "sub     rsp, 40");
    emit(8, "xor     r10d, r10d");
    emit(8, "test    rdi, rdi");
    emit(8, "jns     .L2");
    emit(8, "neg     rdi");
    emit(8, "mov     r10d, 1");
    emit(0, ".L2:");
    emit(8, "mov     QWORD [rsp+24], 0");
    emit(8, "mov     ecx, 1");
    emit(8, "mov     r9, 7378697629483820647");
    emit(8, "mov     QWORD [rsp+8], 0");
    emit(8, "mov     QWORD [rsp+16], 0");
    emit(8, "mov     BYTE [rsp+31], 10");
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

    system("fasm out.asm out > /dev/null");
}
