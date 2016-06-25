/*   solosh_errors.h - couple of macros for treating errors
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

#ifndef SOLOSH_ERROS_H
#define SOLOSH_ERRORS_H

#include <errno.h>
#include <stdio.h>

#define error(expr, code) \
	do\
	{\
		if (expr)\
		{\
			printf("Error: %s\n", strerror(errno));\
			return code;\
		}\
	} while (0)

#define fatal_error(expr, status) \
	do\
	{\
		if (expr)\
		{\
			printf("Fatal error: %s\n", strerror(errno));\
			exit(status);\
		}\
	} while (0)

#endif
