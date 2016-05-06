/*   runcmd.h - header for libruncmd
     Copyright (C) 2016 Rodrigo Weigert <rodrigo.weigert@usp.br>

     This file is part of SoloSH.

     This program is free software: you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation, either version 3 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef RUNCMD_H
#define RUNCMD_H

#define EXECFAILSTATUS 127
#define EXITSTATUSBYTE 0xFF

#define NORMTERM (1 << 8)
#define EXECOK (1 << 9)
#define NONBLOCK (1 << 10)

#define EXITSTATUS(ret) ((ret & EXITSTATUSBYTE))
#define IS_NORMTERM(ret) ((ret & NORMTERM) && 1)
#define IS_EXECOK(ret) ((ret & EXECOK) && 1)
#define IS_NONBLOCK(ret) ((ret & NONBLOCK) && 1)

int runcmd(const char *command, int *result, const int* io);
extern void (*runcmd_onexit)(void);

#endif
