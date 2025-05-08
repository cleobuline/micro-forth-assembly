#include <inttypes.h>

asm(
    ".intel_syntax noprefix\n"
    ".section .text\n"
    ".global forth_rot\n"
    "forth_rot:\n"
    "    push rbx\n"
    "    mov rbx, [rsi]\n"
    "    cmp rbx, 3\n"              // Vérifie si sp >= 3
    "    jl .stack_empty\n"
    "    mov rax, [rdi + rbx*8 - 8]\n"  // rax = stack[sp-1] (c)
    "    mov rcx, [rdi + rbx*8 - 16]\n" // rcx = stack[sp-2] (b)
    "    mov rdx, [rdi + rbx*8 - 24]\n" // rdx = stack[sp-3] (a)
    "    mov [rdi + rbx*8 - 8], rdx\n"  // stack[sp-1] = a
    "    mov [rdi + rbx*8 - 16], rax\n" // stack[sp-2] = c
    "    mov [rdi + rbx*8 - 24], rcx\n" // stack[sp-3] = b
    "    mov [rsi], ebx\n"          // Met à jour sp (même valeur)
    "    pop rbx\n"
    "    ret\n"
    ".stack_empty:\n"
    "    pop rbx\n"
    "    ret\n"
    ".att_syntax\n"
);