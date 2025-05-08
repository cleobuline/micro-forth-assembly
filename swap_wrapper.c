#include <inttypes.h>

asm(
    ".intel_syntax noprefix\n"
    ".section .text\n"
    ".global forth_swap\n"
    "forth_swap:\n"
    "    push rbx\n"
    "    mov rbx, [rsi]\n"
    "    cmp rbx, 2\n"              // Vérifie si sp >= 2
    "    jl .stack_empty\n"
    "    mov rax, [rdi + rbx*8 - 8]\n" // rax = stack[sp-1]
    "    mov rcx, [rdi + rbx*8 - 16]\n" // rcx = stack[sp-2]
    "    mov [rdi + rbx*8 - 8], rcx\n"  // stack[sp-1] = rcx
    "    mov [rdi + rbx*8 - 16], rax\n" // stack[sp-2] = rax
    "    mov [rsi], ebx\n"          // Met à jour sp (même valeur)
    "    pop rbx\n"
    "    ret\n"
    ".stack_empty:\n"
    "    pop rbx\n"
    "    ret\n"
    ".att_syntax\n"
);