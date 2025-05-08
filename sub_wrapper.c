#include <inttypes.h>
void forth_sub(int64_t *stack, int *sp);
asm(
    ".intel_syntax noprefix\n"
    ".section .text\n"
    ".global forth_sub\n"
    ".type forth_sub, @function\n"
    "forth_sub:\n"
    "    .cfi_startproc\n"
    "    push rbx\n"
    "    push rbp\n"
    "    push r12\n"
    "    push r13\n"
    "    test rsi, rsi\n"               /* Vérifier que rsi (sp) n'est pas NULL */
    "    jz .end\n"
    "    test rdi, rdi\n"               /* Vérifier que rdi (stack) n'est pas NULL */
    "    jz .end\n"
    "    mov rbx, [rsi]\n"              /* rbx = sp */
    "    cmp rbx, 2\n"                  /* Vérifie si sp >= 2 */
    "    jl .end\n"
    "    test rbx, rbx\n"               /* Vérifie que rbx n'est pas négatif */
    "    js .end\n"
    "    mov rax, [rdi + rbx*8 - 16]\n" /* rax = stack[sp-2] */
    "    mov r12, [rdi + rbx*8 - 8]\n"  /* r12 = stack[sp-1] (utilise r12 au lieu de rcx) */
    "    sub rax, r12\n"                /* rax = stack[sp-2] - stack[sp-1] */
    "    mov [rdi + rbx*8 - 16], rax\n" /* stack[sp-2] = résultat */
    "    dec rbx\n"                     /* sp -= 1 */
    "    mov [rsi], rbx\n"              /* Met à jour sp (rbx est 64 bits, compatible avec int) */
    ".end:\n"
    "    pop r13\n"
    "    pop r12\n"
    "    pop rbp\n"
    "    pop rbx\n"
    "    ret\n"
    "    .cfi_endproc\n"
    ".att_syntax\n"
);
