.intel_syntax noprefix
.section .text
.global forth_dot
.type forth_dot, @function
forth_dot:
    .cfi_startproc
    push rbx
    push rbp
    push r12
    push r13
    # Vérifier que rsi (sp) n'est pas NULL
    test rsi, rsi
    jz .stack_empty
    # Vérifier que rdi (stack) n'est pas NULL
    test rdi, rdi
    jz .stack_empty
    mov rbx, [rsi]          # rbx = sp
    cmp rbx, 1              # Vérifie si sp >= 1
    jl .stack_empty
    test rbx, rbx           # Vérifie que rbx n'est pas négatif
    js .stack_empty
    dec rbx                 # Décrémente sp
    mov rax, [rdi + rbx*8]  # rax = stack[sp-1]
    mov [rsi], rbx          # Met à jour sp
    mov r8, rax             # Sauvegarder la valeur dans r8
    # Sauvegarder les registres avant printf
    push rsi
    push rdi
    push rbx
    push rcx                # Sauvegarder rcx (control_stack)
    push rdx                # Sauvegarder rdx (csp)
    lea rdi, [rip + .fmt]   # Format pour printf
    mov rsi, r8             # Valeur à afficher
    xor rax, rax            # Pas d'arguments flottants
    call printf             # Appelle printf
    # Restaurer les registres dans l'ordre correct
    pop rdx                 # Restaurer rdx
    pop rcx                 # Restaurer rcx
    pop rbx
    pop rdi
    pop rsi
    mov rax, r8             # Retourne la valeur affichée
    pop r13
    pop r12
    pop rbp
    pop rbx
    ret
.stack_empty:
    mov rax, 0              # Retourne 0 si pile vide
    pop r13
    pop r12
    pop rbp
    pop rbx
    ret
    .cfi_endproc
    .size forth_dot, .-forth_dot
.section .rodata
.fmt:
    .string "%lld\n"
.att_syntax