#Starting with a very simple Makefile. It will likely get more elaborate later on.

%.o: %.c
	gcc --ansi -Wall -I. -c $<

include runcmd.d

%.d: %.c
	gcc --ansi -Wall -I. -MM $< > $@


.PHONY: clean

clean:
	rm -f *.o *.d
