#    Makefile for building libruncmd
#    Copyright (C) 2016 Rodrigo Weigert <rodrigo.weigert@usp.br>
#
#    This file is part of SoloSH.
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.

#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.

#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.

CPP_FLAGS = -ansi -Wall -pedantic-errors -D_POSIX_C_SOURCE=200809L
CC = gcc
AR = ar
TEST_BIN = test


all: lib $(TEST_BIN)

#The library:
lib: libruncmd.a libruncmd.so

libruncmd.a: runcmd.o
	$(AR) rcs $@ $^

libruncmd.so: runcmd-pic.o
	$(CC) $(CPP_FLAGS) -shared $^ -o $@

.SECONDEXPANSION:

#Test programs:

$(TEST_BIN): $$@.o libruncmd.so
	$(CC) $(CPP_FLAGS) -Wl,-rpath='$$ORIGIN' $@.o -L. -lruncmd -o $@

$(TEST_BIN:%=%-static): $$(patsubst %-static, %.o, $$@) libruncmd.a
	$(warning $<)
	$(CC) $(CPP_FLAGS) $< -L. -Wl,-Bstatic -lruncmd -Wl,-Bdynamic -o $@	

#General rules: 
%.o: %.c %.d
	$(CC) $(CPP_FLAGS) -I. -c $<

%-pic.o: %.c %.d
	$(CC) $(CPP_FLAGS) -I. -fPIC -c $< -o $@	

%.d: %.c
	$(CC) $(CPP_FLAGS) -I. -MM $< > $@

include runcmd.d $(TEST_BIN:%=%.d)

.PHONY: all lib clean install uninstall

clean:
	rm -f runcmd.o runcmd-pic.o runcmd.d libruncmd.so libruncmd.a $(TEST_BIN) $(TEST_BIN:%=%-static) $(TEST_BIN:%=%.o) $(TEST_BIN:%=%.d) runcmd.tgz
	rm -rf runcmd

install: libruncmd.a libruncmd.so runcmd.h
	install -d $(PREFIX)/lib
	install -d $(PREFIX)/include
	install libruncmd.a  $(PREFIX)/lib
	install libruncmd.so $(PREFIX)/lib
	install runcmd.h $(PREFIX)/include

uninstall: 
	rm -rf $(PREFIX)/lib/libruncmd.a $(PREFIX)/lib/libruncmd.so $(PREFIX)/include/runcmd.h

dist: runcmd.c runcmd.h debug.h $(TEST_BIN:%=%.c) Makefile README
	rm -rf runcmd
	mkdir runcmd
	cp $^ runcmd
	tar zcvf runcmd.tgz runcmd

distcheck: dist
	if make -C runcmd; then echo "\nruncmd.tgz is ready for distribution"; \
	else echo "\nSomething wrong with runcmd.tgz"; fi; echo
