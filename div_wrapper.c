#include <inttypes.h>
void forth_div(int64_t *stack, int *sp);
asm(
    ".intel_syntax noprefix\n"
    ".section .text\n"
    ".global forth_div\n"
    ".type forth_div, @function\n"
    "forth_div:\n"
    "    .cfi_startproc\n"
    "    push rbx\n"
    "    push rbp\n"
    "    push r12\n"
    "    push r13\n"
    "    push rdx\n"                    /* Sauvegarder rdx, modifié par cqo et idiv */
    "    test rsi, rsi\n"               /* Vérifier que rsi (sp) n'est pas NULL */
    "    jz .stack_empty\n"
    "    test rdi, rdi\n"               /* Vérifier que rdi (stack) n'est pas NULL */
    "    jz .stack_empty\n"
    "    mov rbx, [rsi]\n"              /* rbx = sp */
    "    cmp rbx, 2\n"                  /* Vérifie si sp >= 2 */
    "    jl .stack_empty\n"
    "    test rbx, rbx\n"               /* Vérifie que rbx n'est pas négatif */
    "    js .stack_empty\n"
    "    mov r12, [rdi + rbx*8 - 8]\n"  /* r12 = stack[sp-1] (diviseur) */
    "    cmp r12, 0\n"                  /* Vérifie si diviseur == 0 */
    "    je .div_by_zero\n"
    "    mov rax, [rdi + rbx*8 - 16]\n" /* rax = stack[sp-2] (dividende) */
    "    cqo\n"                         /* Étend le signe de rax dans rdx:rax */
    "    idiv r12\n"                    /* rax = rax / r12 (division entière) */
    "    mov [rdi + rbx*8 - 16], rax\n" /* stack[sp-2] = résultat */
    "    dec rbx\n"                     /* sp -= 1 */
    "    mov [rsi], rbx\n"              /* Met à jour sp (rbx est 64 bits, compatible avec int) */
    "    pop rdx\n"
    "    pop r13\n"
    "    pop r12\n"
    "    pop rbp\n"
    "    pop rbx\n"
    "    ret\n"
    ".stack_empty:\n"
    "    pop rdx\n"
    "    pop r13\n"
    "    pop r12\n"
    "    pop rbp\n"
    "    pop rbx\n"
    "    ret\n"
    ".div_by_zero:\n"
    "    pop rdx\n"
    "    pop r13\n"
    "    pop r12\n"
    "    pop rbp\n"
    "    pop rbx\n"
    "    ret\n"
    "    .cfi_endproc\n"
    ".att_syntax\n"
);
