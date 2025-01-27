#pragma once

#define push_all_reg()   \
    emit(8, "push rax"); \
    emit(8, "push rbx"); \
    emit(8, "push rcx"); \
    emit(8, "push rdx"); \
    emit(8, "push rbp"); \
    emit(8, "push rsp"); \
    emit(8, "push rsi"); \
    emit(8, "push rdi"); \
    emit(8, "push r8");  \
    emit(8, "push r9");  \
    emit(8, "push r10"); \
    emit(8, "push r11"); \
    emit(8, "push r12"); \
    emit(8, "push r13"); \
    emit(8, "push r14"); \
    emit(8, "push r15")

#define pop_all_reg()   \
    emit(8, "pop r15"); \
    emit(8, "pop r14"); \
    emit(8, "pop r13"); \
    emit(8, "pop r12"); \
    emit(8, "pop r11"); \
    emit(8, "pop r10"); \
    emit(8, "pop r9");  \
    emit(8, "pop r8");  \
    emit(8, "pop rdi"); \
    emit(8, "pop rsi"); \
    emit(8, "pop rsp"); \
    emit(8, "pop rbp"); \
    emit(8, "pop rdx"); \
    emit(8, "pop rcx"); \
    emit(8, "pop rbx"); \
    emit(8, "pop rax")


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

#define emit_error(...) \
    error_index = append_error_msg(__VA_ARGS__);\
    emit(8, "ERROR ERROR_%d", error_index)\

#define emit_debug(...) \
    debug_index = append_debug_msg(__VA_ARGS__);\
    emit(8, "DEBUG DEBUG_%d", debug_index)\

FILE *f;
String_Array error_msgs;
int error_index;
String_Array debug_msgs;
int debug_index;

__attribute__((format(printf, 1, 2)))
int append_error_msg(char *fmt, ...) {
    char *s = malloc(254);
    va_list args;
    va_start(args, fmt);
    vsnprintf(s, 254, fmt, args);
    va_end(args);

    append(error_msgs, s);

    return error_msgs.index - 1;
}

__attribute__((format(printf, 1, 2)))
int append_debug_msg(char *fmt, ...) {
    char *s = malloc(254);
    va_list args;
    va_start(args, fmt);
    vsnprintf(s, 254, fmt, args);
    va_end(args);

    append(debug_msgs, s);

    return debug_msgs.index - 1;
}

__attribute__((format(printf, 2, 3)))
void emit(int indent, char *fmt, ...) {
    fprintf(f, "%*s", indent, "");
    va_list args;
    va_start(args, fmt);
    vfprintf(f, fmt, args);
    va_end(args);
    fprintf(f, "\n");
}


void emit_write_num(void) {
    // num rdi
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
    emit(8, "sub rdx, 1");
    emit(8, "mov     edi, 1");
    emit(8, "sub     rax, rcx");
    emit(8, "lea     rsi, [rsp+8+rax]");
    emit(8, "mov     rax, 1");
    emit(8, "syscall");
    emit(8, "add     rsp, 40");
    emit(8, "ret");
}

void emit_print_value(void) {
    // op1_metadata, op1_value
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

void emit_alloc(void) {
    // size rdi
    emit(0, "alloc:");
    emit(8, "mov rax, [ALLOC_PTR]");
    emit(8, "add rax, rdi");
    emit(8, "cmp rax, [HEAP_END]");
    emit(8, "jge collect");
    emit(8, "mov rax, [ALLOC_PTR]");
    emit(8, "add rax, rdi");
    emit(8, "cmp rax, [HEAP_END]");
    emit(8, "jge alloc_error");
    emit(8, "mov rax, [ALLOC_PTR]");
    emit(8, "add [ALLOC_PTR], rdi");
    emit(8, "ret");
    emit(0, "alloc_error:");
    emit_error("ERROR couldnt alloc");
}

void emit_memcpy(void) {
    //src rdi, dest rsi, len rdx
    emit(0, "memcpy:");
    emit(8, "mov rax, 0");
    emit(0, "memcpy_start:");
    emit(8, "mov BYTE cl, [rdi + rax]");
    emit(8, "mov [rsi + rax], cl");
    emit(8, "add rax, 1");
    emit(8, "cmp rax, rdx");
    emit(8, "jne memcpy_start");
    emit(8, "ret");
}

void emit_memset(void) {
    //byte dil, dest rsi, len rdx
    emit(0, "memset:");
    emit(8, "mov rax, 0");
    emit(0, "memset_start:");
    emit(8, "mov [rsi + rax], dil");
    emit(8, "add rax, 1");
    emit(8, "cmp rax, rdx");
    emit(8, "jne memset_start");
    emit(8, "ret");
}

void emit_collect(void) {
    emit(0, "collect:");
    push_all_reg();

    emit(8, "mov rax, [FROM_SPACE]");
    emit(8, "mov rbx, [TO_SPACE]");
    emit(8, "mov [FROM_SPACE], rbx");
    emit(8, "mov [TO_SPACE], rax");
    emit(8, "mov [FREE_PTR], rbx");
    emit(8, "mov [SCAN_PTR], rbx");

    emit(0, "scan_globals:");
    emit_debug("VALUE: ");
    emit(8, "suda_pop op1_value, op1_metadata");
    emit(8, "call print_value");
    emit(8, "WRITE STR_NEWLINE, 1");
    emit(8, "cmp suda_sp, SUDA_STACK");
    emit(8, "jg scan_globals");
    emit_error("FOOOOOOO");
    pop_all_reg();
}

void emit_helpers() {
    emit_write_num();
    emit_print_value();
    emit_alloc();
    emit_memcpy();
    emit_memset();
    emit_collect();
}
