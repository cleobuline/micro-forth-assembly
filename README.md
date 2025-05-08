# micro-forth-assemby
a minimal fort with assembly word
ubuntu x 86 

compile with 

gcc -c -fPIC add_wrapper.c -o add_wrapper.o

gcc -c -fPIC dup_wrapper.c -o dup_wrapper.o

gcc -c -fPIC drop_wrapper.c -o drop_wrapper.o

gcc -c -fPIC sub_wrapper.c -o sub_wrapper.o

gcc -c -fPIC mult_wrapper.c -o mult_wrapper.o

gcc -c -fPIC div_wrapper.c -o div_wrapper.o

gcc -c -fPIC rot_wrapper.c -o rot_wrapper.o

gcc -c -fPIC do_wrapper.c -o do_wrapper.o

gcc -c -fPIC loop_wrapper.c -o loop_wrapper.o

gcc -c -fPIC i_wrapper.c -o i_wrapper.o

gcc -c -fPIC dot_wrapper.S -o dot_wrapper.o


gcc -shared -fPIC -Wl,--export-dynamic add_wrapper.o dot_wrapper.o dup_wrapper.o mult_wrapper.o sub_wrapper.o swap_wrapper.o drop_wrapper.o rot_wrapper.o div_wrapper.o do_wrapper.o loop_wrapper.o i_wrapper.o -o libforth_primitives.so

export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH 

gcc -o forth forth.c -g  -ldl -L. -lforth_primitives


./forth 

sample execution 

Forth interpreter (tapez 'quit' pour quitter)

> : carre dup * ;
> 
  carre (compiled_func=0x7f842ce2d139)
  
Pile (sp=0): vide

> : cube dup carre * ;
> 
  carre (compiled_func=0x7f842ce2d139)
  
  cube (compiled_func=0x7f842ce28159)
  
Pile (sp=0): vide

> : test do i cube loop ;
> 
  carre (compiled_func=0x7f842ce2d139)
  
  cube (compiled_func=0x7f842ce28159)
  
  test (compiled_func=0x7f842cbf7179)
  
 
> 10 0 test
> 
Pile (sp=11): 11 0 1 8 27 64 125 216 343 512 729 

>
> produced file #include <inttypes.h>

void forth_dup(int64_t *stack, int *sp);
void forth_swap(int64_t *stack, int *sp);
void forth_add(int64_t *stack, int *sp);
int64_t forth_dot(int64_t *stack, int *sp);
void forth_drop(int64_t *stack, int *sp);
void forth_sub(int64_t *stack, int *sp);
void forth_rot(int64_t *stack, int *sp);
void forth_mult(int64_t *stack, int *sp);
void forth_div(int64_t *stack, int *sp);
void forth_do(int64_t *stack, int *sp, int64_t *control_stack, int *csp);
void forth_loop(int64_t *stack, int *sp, int64_t *control_stack, int *csp);
void forth_i(int64_t *stack, int *sp, int64_t *control_stack, int *csp);
void cube(int64_t *stack, int *sp, int64_t *control_stack, int *csp);

void test(int64_t *stack, int *sp, int64_t *control_stack, int *csp) {
    asm volatile (
        ".intel_syntax noprefix\n"
        "push rbp\n"
        "push rbx\n"
        "push r12\n"
        "push r13\n"
        "push r14\n"
        "push r15\n"
        "call forth_do\n"
        ".loop_start_0:\n"
        "call forth_i\n"
        "push rdi\n"
        "push rsi\n"
        "push rcx\n"
        "push rdx\n"
        "mov rdi, 0x556b0b6791e0\n"
        "mov rsi, 0x556b0b6799e0\n"
        "mov rcx, 0x556b0b679a00\n"
        "mov rdx, 0x556b0b679c00\n"
        "call cube\n"
        "pop rdx\n"
        "pop rcx\n"
        "pop rsi\n"
        "pop rdi\n"
        "call forth_loop\n"
        "cmp rax, 1\n"
        "je .loop_start_0\n"
        ".loop_end_0:\n"
        "pop r15\n"
        "pop r14\n"
        "pop r13\n"
        "pop r12\n"
        "pop rbx\n"
        "pop rbp\n"
        ".att_syntax\n"
        : "+S"(sp), "+d"(csp)
        : "D"(stack), "c"(control_stack)
        : "rax", "r8", "r9", "r10", "r11", "memory"
    );
}
 
