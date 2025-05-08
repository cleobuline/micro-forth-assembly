#include <inttypes.h>

// Déclaration explicite de la fonction
void forth_loop(int64_t *stack, int *sp, int64_t *control_stack, int *csp);

asm(
    ".intel_syntax noprefix\n"
    ".section .text\n"
    ".global forth_loop\n"
    "forth_loop:\n"
    "    push rbx\n"
    "    push rbp\n"
    "    push r12\n"
    "    mov r12, rcx\n"            // r12 = control_stack
    "    mov rax, [rdx]\n"          // rax = csp
    "    cmp rax, 2\n"              // Vérifie si csp >= 2
    "    jl .control_stack_empty\n"
    "    sub rax, 2\n"              // rax = csp - 2
    "    mov rbx, [r12 + rax*8 + 8]\n" // rbx = index
    "    inc rbx\n"                 // index += 1
    "    mov [r12 + rax*8 + 8], rbx\n" // Met à jour index
    "    cmp rbx, [r12 + rax*8]\n"  // Compare index avec limit
    "    jl .loop_continue\n"       // Si index < limit, continue
    "    mov [rdx], rax\n"          // Met à jour csp (pop control stack)
    "    mov rax, 0\n"              // Retourner 0 (fin de la boucle)
    "    pop r12\n"
    "    pop rbp\n"
    "    pop rbx\n"
    "    ret\n"
    ".loop_continue:\n"
    "    add rax, 2\n"              // Restaure csp
    "    mov [rdx], rax\n"          // Met à jour csp
    "    mov rax, 1\n"              // Retourner 1 (continuer la boucle)
    "    pop r12\n"
    "    pop rbp\n"
    "    pop rbx\n"
    "    ret\n"
    ".control_stack_empty:\n"
    "    mov rax, 0\n"              // Retourner 0 (erreur, fin)
    "    pop r12\n"
    "    pop rbp\n"
    "    pop rbx\n"
    "    ret\n"
    ".att_syntax\n"
);