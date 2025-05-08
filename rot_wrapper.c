#include <inttypes.h>
void forth_rot(int64_t *stack, int *sp);
asm(
    ".intel_syntax noprefix\n"
    ".section .text\n"
    ".global forth_rot\n"
    ".type forth_rot, @function\n"
    "forth_rot:\n"
    "    .cfi_startproc\n"
    "    push rbx\n"
    "    push rbp\n"
    "    push r12\n"
    "    push r13\n"
    "    push rcx\n"                    /* Sauvegarder rcx, utilisé pour stack[sp-2] */
    "    push rdx\n"                    /* Sauvegarder rdx, utilisé pour stack[sp-3] */
    "    test rsi, rsi\n"               /* Vérifier que rsi (sp) n'est pas NULL */
    "    jz .stack_empty\n"
    "    test rdi, rdi\n"               /* Vérifier que rdi (stack) n'est pas NULL */
    "    jz .stack_empty\n"
    "    mov rbx, [rsi]\n"              /* rbx = sp */
    "    cmp rbx, 3\n"                  /* Vérifie si sp >= 3 */
    "    jl .stack_empty\n"
    "    test rbx, rbx\n"               /* Vérifie que rbx n'est pas négatif */
    "    js .stack_empty\n"
    "    mov rax, [rdi + rbx*8 - 8]\n"  /* rax = stack[sp-1] (c) */
    "    mov rcx, [rdi + rbx*8 - 16]\n" /* rcx = stack[sp-2] (b) */
    "    mov rdx, [rdi + rbx*8 - 24]\n" /* rdx = stack[sp-3] (a) */
    "    mov [rdi + rbx*8 - 8], rdx\n"  /* stack[sp-1] = a */
    "    mov [rdi + rbx*8 - 16], rax\n" /* stack[sp-2] = c */
    "    mov [rdi + rbx*8 - 24], rcx\n" /* stack[sp-3] = b */
    "    mov [rsi], rbx\n"              /* Met à jour sp (rbx est 64 bits, inchangé) */
    "    pop rdx\n"
    "    pop rcx\n"
    "    pop r13\n"
    "    pop r12\n"
    "    pop rbp\n"
    "    pop rbx\n"
    "    ret\n"
    ".stack_empty:\n"
    "    pop rdx\n"
    "    pop rcx\n"
    "    pop r13\n"
    "    pop r12\n"
    "    pop rbp\n"
    "    pop rbx\n"
    "    ret\n"
    "    .cfi_endproc\n"
    ".att_syntax\n"
);
