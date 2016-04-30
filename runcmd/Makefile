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

include runcmd.d test.d

.PHONY: clean install uninstall

clean:
	rm -f *.o *.d *.so *.a runcmd test test-static runcmd.tgz

install: libruncmd.a libruncmd.so runcmd.h
	install -d $(PREFIX)/lib
	install -d $(PREFIX)/include
	install libruncmd.a  $(PREFIX)/lib
	install libruncmd.so $(PREFIX)/lib
	install runcmd.h $(PREFIX)/include

uninstall: 
	rm -rf $(PREFIX)/lib/libruncmd.a $(PREFIX)/lib/libruncmd.so $(PREFIX)/include/runcmd.h

dist: runcmd.c runcmd.h test.c Makefile
	mkdir runcmd
	cp $^ runcmd
	tar zcvf runcmd.tgz runcmd
