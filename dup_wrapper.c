#include <inttypes.h>

asm(
    ".intel_syntax noprefix\n"
    ".section .text\n"
    ".global forth_dup\n"
    "forth_dup:\n"
    "    push rbx\n"
    "    mov rbx, [rsi]\n"
    "    cmp rbx, 1\n"              // Vérifie si sp >= 1
    "    jl .stack_empty\n"
    "    cmp rbx, 255\n"            // Vérifie débordement
    "    jge .stack_full\n"
    "    dec rbx\n"                 // Accède à stack[sp-1]
    "    mov rax, [rdi + rbx*8]\n"
    "    inc rbx\n"                 // Restaure sp
    "    mov [rdi + rbx*8], rax\n"  // Duplique
    "    inc rbx\n"                 // Incrémente sp
    "    mov [rsi], ebx\n"          // Met à jour sp
    "    pop rbx\n"
    "    ret\n"
    ".stack_empty:\n"
    "    pop rbx\n"
    "    ret\n"
    ".stack_full:\n"
    "    pop rbx\n"
    "    ret\n"
    ".att_syntax\n"
);