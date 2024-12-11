#pragma once

#define read_index (COMBYTE(code.data[i + 1], code.data[i + 2]))

FILE *f;

void emit_header() {
    fprintf(f, "format ELF64 executable 3\n");
    fprintf(f, "segment readable executable\n");
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
    fprintf(f, "entry start\n");
    fprintf(f, "start:\n");
}

void emit_func(char *name, Code code, Locations locs, Constants constants) {
    for (int i = 0; i < code.index; i++) {
        switch (code.data[i]) {
            case OP_CONSTANT:
                fprintf(f, "        push %ld\n", constants.data[read_index].val.num);
                i += 2;
                break;
            case OP_PRINTLN:
                fprintf(f, "        pop rdi\n");
                fprintf(f, "        call println_int\n");
                break;
            case OP_RETURN_NOTHING:
                continue;
            default:
                ERR("ERROR in %s on line %ld: cant emit asm for op type %s\n", locs.data[i].file, locs.data[i].line, find_op_code(code.data[i]))
                break;
        }
    }
}

void emit_footer() {
    fprintf(f, "        mov rax, 60\n");
    fprintf(f, "        mov rdi, 0\n");
    fprintf(f, "        syscall\n");
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

    fclose(f);

    system("fasm out.asm out > /dev/null");
}
