#This Makefile will be used only during initial development.

BIN=solosh
PARSER = solosh_parser

$(BIN) : % : %.o $(PARSER:%=%.o)
	gcc -Wall -Werror --ansi --pedantic-errors -D_POSIX_C_SOURCE=200809L -g -I. $^ -o $@

%.o: %.c
	gcc -Wall -Werror --ansi --pedantic-errors -D_POSIX_C_SOURCE=200809L -g -I. -c $<

include $(BIN:%=%.d) $(PARSER:%=%.d)

%.d: %.c
	gcc -Wall -Werror --ansi --pedantic-errors -D_POSIX_C_SOURCE=200809L -I. -MM $< > $@


.PHONY: clean

clean:
	rm -f $(BIN) $(BIN:%=%.o) $(PARSER:%=%.o)
