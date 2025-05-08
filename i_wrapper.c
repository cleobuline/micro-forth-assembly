#include <inttypes.h>

// Déclaration explicite de la fonction
void forth_i(int64_t *stack, int *sp, int64_t *control_stack, int *csp);

asm(
    ".intel_syntax noprefix\n"
    ".section .text\n"
    ".global forth_i\n"
    "forth_i:\n"
    "    push rbx\n"
    "    push rbp\n"
    "    push r12\n"
    "    push r13\n"                // Alignement de la pile (4 registres = 32 octets)
    "    mov r12, rcx\n"            // r12 = control_stack
    "    mov rax, [rdx]\n"          // rax = csp
    "    cmp rax, 2\n"              // Vérifie si csp >= 2
    "    jl .control_stack_empty\n"
    "    mov rbx, [rsi]\n"          // rbx = sp
    "    cmp rbx, 256\n"            // Vérifie si sp < 256 (éviter débordement)
    "    jge .stack_full\n"
    "    mov r13, [r12 + rax*8 - 8]\n" // r13 = control_stack[csp-1] (index)
    "    mov [rdi + rbx*8], r13\n"  // stack[sp] = index
    "    inc rbx\n"                 // sp += 1
    "    mov [rsi], rbx\n"          // Met à jour sp
    "    pop r13\n"
    "    pop r12\n"
    "    pop rbp\n"
    "    pop rbx\n"
    "    ret\n"
    ".control_stack_empty:\n"
    "    pop r13\n"
    "    pop r12\n"
    "    pop rbp\n"
    "    pop rbx\n"
    "    ret\n"
    ".stack_full:\n"
    "    pop r13\n"
    "    pop r12\n"
    "    pop rbp\n"
    "    pop rbx\n"
    "    ret\n"
    ".att_syntax\n"
);