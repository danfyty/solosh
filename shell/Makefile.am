##   Makefile.am - Automake script for src dir.
##
##   Copyright (c) 2014, Francisco Jose Monaco  <moanco@icmc.usp.br>
##
##   This file is part of POSIXeg.
##
##   POSIXeg is free software: you can redistribute it and/or modify
##   it under the terms of the GNU General Public License as published by
##   the Free Software Foundation, either version 3 of the License, or
##   (at your option) any later version.
##
##   This program is distributed in the hope that it will be useful,
##   but WITHOUT ANY WARRANTY; without even the implied warranty of
##   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##   GNU General Public License for more details.
##
##   You should have received a copy of the GNU General Public License
##   along with this program.  If not, see <http://www.gnu.org/licenses/>.

bin_PROGRAMS = solosh

solosh_SOURCES = solosh.c solosh_parser.c
solosh_CPPFLAGS = -Wall -Werror --ansi --pedantic-errors -D_POSIX_C_SOURCE=200809L -I.

include_HEADERS = solosh_parser.h
