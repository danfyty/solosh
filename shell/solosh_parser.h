/*   solosh_parser.h - parser header
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

#ifndef SOLOSH_PARSER_H
#define SOLOSH_PARSER_H

#define SLSH_BLANK_CHARS " \t"
#define SLSH_PIPE "|"

#define SLSH_INPUT '<'
#define SLSH_OUTPUT '>'
#define SLSH_NOBLOCK '&'

enum
{
	CMD_BG = 1,
	CMD_CD,
	CMD_EXIT,
	CMD_FG,
	CMD_JOBS,
	CMD_QUIT
};

char** split_around_blank(const char* str);
char*** make_cmd_array(const char* command);
void print_job_cmd(char*** cmd);
int get_io_redir_file(const char* command, int io);
char* clean_command(const char* command);
int is_blocking(const char* command);
int get_builtin_cmd(const char* command);

#endif
