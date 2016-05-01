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


#Starting with a very simple Makefile. It will likely get more elaborate later on.

CPP_FLAGS = -ansi -Wall -pedantic-errors -D_POSIX_C_SOURCE=200809L
CC = gcc
AR = ar

all: lib test

#The library:
lib: libruncmd.a libruncmd.so

libruncmd.a: runcmd.o
	$(AR) rcs $@ $^

libruncmd.so: runcmd-pic.o
	$(CC) $(CPP_FLAGS) -shared $^ -o $@

#A test program:
test: test.o libruncmd.so
	$(CC) $(CPP_FLAGS) test.o -L. -lruncmd -o $@

test-static: test.o libruncmd.a
	$(CC) $(CPP_FLAGS) test.o -L. -Wl,-Bstatic -lruncmd -Wl,-Bdynamic -o $@	

#General rules: 
%.o: %.c %.d
	$(CC) $(CPP_FLAGS) -I. -c $<

%-pic.o: %.c %.d
	$(CC) $(CPP_FLAGS) -I. -fPIC -c $< -o $@	

%.d: %.c
	$(CC) $(CPP_FLAGS) -I. -MM $< > $@

include runcmd.d test.d

.PHONY: all lib clean install uninstall

clean:
	rm -f runcmd.o runcmd-pic.o runcmd.d libruncmd.so libruncmd.a test.o test.d test test-static runcmd.tgz

install: libruncmd.a libruncmd.so runcmd.h
	install -d $(PREFIX)/lib
	install -d $(PREFIX)/include
	install libruncmd.a  $(PREFIX)/lib
	install libruncmd.so $(PREFIX)/lib
	install runcmd.h $(PREFIX)/include

uninstall: 
	rm -rf $(PREFIX)/lib/libruncmd.a $(PREFIX)/lib/libruncmd.so $(PREFIX)/include/runcmd.h

dist: runcmd.c runcmd.h test.c Makefile README
	mkdir runcmd
	cp $^ runcmd
	tar zcvf runcmd.tgz runcmd
