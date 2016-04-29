# Project configuration

# 
# CUSTOMIZE FOR YOUR PROJECT
#

# Give your project a name, e.g.
 PROJECT = runcmd

# Name your project's binaires, eg.
# bin = foo bar

# For each item in $(bin), name the objects needed to build it, e.g.
# foo_obj = main_foo.o 
# bar_obj = main_bar.o 

# For each item in $(bin), name the project's libraries to which it shall be linked, e.g.
# foo_lib = libfoobar
# bar_lib = libfoobar 

# For each item in $(bin), name the other libraries to which it shall be 
# linked, e.g.
# foo_lib_other = 
# bar_lib_other = libm


# Name your project's libraries (to be installed), e.g.
# lib = libfoobar
lib = libruncmd

# For each item in $(lib), name the objects needed to build it, e.g.
# libfoobar_obj = foo.o bar.o
libruncmd_obj = runcmd.o

# For each item in $(lib), name the headers your project installs, e.g.
#libfoobar_h = foo.h
libruncmd_h = runcmd.h

# Name your project's auxuliary binaries (not to be installed), e.g.
# noinst_bin = example
noinst_bin = test-runcmd delay io segfault t1

# For each item in $(noinst_bin), name the objects needed to build it, e.g.
# example_obj = example.o 
delay_obj = delay.o
io_obj = io.o
segfault_obj = segfault.o
test-runcmd_obj = test-runcmd.o
t1_obj = t1.o

# For each item in $(noinst_bin), name the libraries to which it's to be 
# linked, e.g.
# example_lib = libfoobar
test-runcmd_lib = libruncmd

# Extra files to go in the distribution (add as needed), e.g.
# EXTRA_DIST = foo.txt

# List the automatically generated, distributed source files of any kind.
# These files are generated in the development environment but should
# be already available in the building enviroment such as configuration 
# scripts etc, e.g.
# gendist_files = config.h

gendist_files = debug.h Makefile

# Installation prefix, e.g.
PREFIX=./local

# Customize the building environemnt (modify as desired), e.g.
CC = gcc
CPP_FLAGS = -I. -Wall --ansi --pedantic-errors -D_POSIX_C_SOURCE=200112L 
C_FLAGS = 
LD_FLAGS = -L.
MAKE = make
AR = ar

# Name your test programns, if any e.g.
tests-bin = test-runcmd

# Housekeeping (name temporary files to clean) e.g.
# EXTRA_GARBAGE = *~
# EXTRA_GARBAGE = *~ \#*

