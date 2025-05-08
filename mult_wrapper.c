#include <inttypes.h>

asm(
    ".intel_syntax noprefix\n"
    ".section .text\n"
    ".global forth_mult\n"
    "forth_mult:\n"
    "    push rbx\n"
    "    push rbp\n"                // Pour alignement 16 octets
    "    mov rbx, [rsi]\n"
    "    cmp rbx, 2\n"              // Vérifie si sp >= 2
    "    jl .stack_empty\n"
    "    mov rax, [rdi + rbx*8 - 8]\n"  // rax = stack[sp-1]
    "    mov rcx, [rdi + rbx*8 - 16]\n" // rcx = stack[sp-2]
    "    imul rax, rcx\n"           // rax = stack[sp-2] * stack[sp-1]
    "    mov [rdi + rbx*8 - 16], rax\n" // stack[sp-2] = résultat
    "    dec rbx\n"                 // sp -= 1
    "    mov [rsi], ebx\n"          // Met à jour sp avec ebx (int)
    "    pop rbp\n"
    "    pop rbx\n"
    "    ret\n"
    ".stack_empty:\n"
    "    pop rbp\n"
    "    pop rbx\n"
    "    ret\n"
    ".att_syntax\n"
);