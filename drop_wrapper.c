#include <inttypes.h>

// Déclaration explicite de la fonction
void forth_drop(int64_t *stack, int *sp);

asm(
    ".intel_syntax noprefix\n"
    ".section .text\n"
    ".global forth_drop\n"
    "forth_drop:\n"
    "    push rbx\n"
    "    mov rbx, [rsi]\n"
    "    cmp rbx, 1\n"              // Vérifie si sp >= 1
    "    jl .stack_empty\n"
    "    dec rbx\n"                 // Décrémente sp
    "    mov [rsi], ebx\n"          // Met à jour sp
    "    pop rbx\n"
    "    ret\n"
    ".stack_empty:\n"
    "    pop rbx\n"
    "    ret\n"
    ".att_syntax\n"
);