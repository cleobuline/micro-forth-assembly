#include <inttypes.h>
void forth_add(int64_t *stack, int *sp);
asm(
    ".intel_syntax noprefix\n"
    ".section .text\n"
    ".global forth_add\n"
    "forth_add:\n"
    "    push rbx\n"
    "    mov rbx, [rsi]\n"
    "    cmp rbx, 2\n"              // Vérifie si sp >= 2
    "    jl .stack_empty\n"
    "    mov rax, [rdi + rbx*8 - 8]\n"  // rax = stack[sp-1]
    "    mov rcx, [rdi + rbx*8 - 16]\n" // rcx = stack[sp-2]
    "    add rax, rcx\n"            // rax = stack[sp-1] + stack[sp-2]
    "    mov [rdi + rbx*8 - 16], rax\n" // stack[sp-2] = résultat
    "    dec rbx\n"                 // sp -= 1
    "    mov [rsi], ebx\n"          // Met à jour sp
    "    pop rbx\n"
    "    ret\n"
    ".stack_empty:\n"
    "    pop rbx\n"
    "    ret\n"
    ".att_syntax\n"
);