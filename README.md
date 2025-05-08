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
