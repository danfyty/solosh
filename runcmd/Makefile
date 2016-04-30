#Starting with a very simple Makefile. It will likely get more elaborate later on.

CPP_FLAGS = -ansi -Wall -pedantic-errors -D_POSIX_C_SOURCE=200809L

#The library:
lib: libruncmd.a libruncmd.so

libruncmd.a: runcmd.o
	ar rcs $@ $^

libruncmd.so: runcmd-pic.o
	gcc $(CPP_FLAGS) -shared $^ -o $@

#A test program:
test: test.o libruncmd.so
	gcc $(CPP_FLAGS) test.o -L. -lruncmd -o $@

test-static: test.o libruncmd.a
	gcc $(CPP_FLAGS) test.o -L. -Wl,-Bstatic -lruncmd -Wl,-Bdynamic -o $@	

#General rules: 
%.o: %.c %.d
	gcc $(CPP_FLAGS) -I. -c $<

%-pic.o: %.c %.d
	gcc $(CPP_FLAGS) -I. -fPIC -c $< -o $@	

%.d: %.c
	gcc $(CPP_FLAGS) -I. -MM $< > $@



.PHONY: clean

clean:
	rm -f *.o *.d *.so *.a runcmd test test-static

include runcmd.d test.d
