##   Makefile.am - Automake script for project's libraries.
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

## Reminder for versioning
##
## Versioning: CURRENT:REVISION:AGE
## CURRENT The latest interface implemented.
## REVISION The implementation number of CURRENT (read: number of bugs fixed...)
## AGE The number of interfaces implemented, minus one.
## The library supports all interfaces between CURRENT − AGE and CURRENT.
##
## If you have 
## not changed the interface (bug fixes)         CURRENT : REVISION+1 : AGE
## augmented the interface (new functions)       CURRENT+1 : 0 : AGE+1
## broken old interface (e.g. removed functions) CURRENT+1 : 0 : 0

RUNCMD_VERSIONING = 0:0:0 

lib_LTLIBRARIES = libruncmd.la

libruncmd_la_SOURCES = runcmd.c
libruncmd_la_LDFLAGS = -version-info $(RUNCMD_VERSIONING)
libruncmd_la_CPPFLAGS = --ansi -Wall --pedantic-errors -D_POSIX_C_SOURCE=200809L

include_HEADERS = runcmd.h debug.h
