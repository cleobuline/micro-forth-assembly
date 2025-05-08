#include <inttypes.h>

// Déclaration explicite de la fonction
void forth_div(int64_t *stack, int *sp);

asm(
    ".intel_syntax noprefix\n"
    ".section .text\n"
    ".global forth_div\n"
    "forth_div:\n"
    "    push rbx\n"
    "    push rbp\n"                // Pour alignement 16 octets
    "    mov rbx, [rsi]\n"
    "    cmp rbx, 2\n"              // Vérifie si sp >= 2
    "    jl .stack_empty\n"
    "    mov rcx, [rdi + rbx*8 - 8]\n"  // rcx = stack[sp-1] (diviseur)
    "    cmp rcx, 0\n"              // Vérifie si diviseur == 0
    "    je .div_by_zero\n"
    "    mov rax, [rdi + rbx*8 - 16]\n" // rax = stack[sp-2] (dividende)
    "    cqo\n"                     // Étend le signe de rax dans rdx:rax
    "    idiv rcx\n"                // rax = rax / rcx (division entière)
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
    ".div_by_zero:\n"
    "    pop rbp\n"
    "    pop rbx\n"
    "    ret\n"
    ".att_syntax\n"
);