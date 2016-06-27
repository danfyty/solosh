/*   solosh_parser.c - the shell's internal parser
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
#include <fcntl.h>
#include <solosh_errors.h>
#include <solosh_parser.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define INITIAL_TOKEN_ARRAY_CAP 10
#define INITIAL_LINE_CAP 10

static const int nbcmd = 6;
static const char* builtin_cmd[] = {"bg", "cd", "exit", "fg", "jobs", "quit"};

char* read_line()
{
	char* line = (char*) malloc(sizeof(char)*INITIAL_LINE_CAP);
	int sz = 0, cap = INITIAL_LINE_CAP;
	int c;

	error(line == NULL, NULL);
	do
	{
		c = getchar();
		if (sz == cap)
		{
			char* newline = (char*) realloc(line, sizeof(char)*2*cap);
			error(newline == NULL, (free(line), NULL));
			line = newline;
			cap *= 2;
		}
		line[sz++] = c;
	} while (c != EOF && c != '\n');
	line[--sz] = '\0';

	if (sz == 0)
	{
		free(line);
		return NULL;
	}

	return line;
}

int get_builtin_cmd(const char* cmd)
{
	int i;
	
	if (cmd == NULL)
		return 0;
	
	for (i = 0; i < nbcmd; i++)
	{
		if (!strcmp(builtin_cmd[i], cmd))
			return i+1;
	}
	return 0;
}

static int is_blank(char c)
{
	char blank[] = SLSH_BLANK_CHARS;
	int i, len;

	len = strlen(blank);

	for (i = 0; i < len; i++)
	{
		if (c == blank[i])
			return 1;
	}
	return 0;
}

static char* trim_front(const char* str)
{
	int len = strlen(str), i;
	char* ret = (char*) malloc(sizeof(char)*(len+1));
	
	if (ret == NULL)
		return NULL;
	
	i = 0;
	while (i < len && is_blank(str[i]))
		i++;
	strcpy(ret, str+i);
	return ret;
}

char** split_around_blank(const char* str)
{
	int captok = INITIAL_TOKEN_ARRAY_CAP, ntok = 0;
	char** s, *cpstr;

	if (str == NULL)
		return NULL;

	cpstr = trim_front(str);
	error(cpstr == NULL, NULL);
	
	s = (char**) malloc(sizeof(char*)*captok);
	error(s == NULL, (free(cpstr), NULL));

	s[ntok++] = strtok(cpstr, SLSH_BLANK_CHARS);
	while ((s[ntok++] = strtok(NULL, SLSH_BLANK_CHARS)) != NULL)
	{
		if (ntok == captok)
		{
			char** new_s;

			captok*=2;
			new_s = (char**) realloc(s, sizeof(char*)*captok);
			error(new_s == NULL, (free(cpstr),(free(s),  NULL)));
			s = new_s;
		}
	}
	return s;
}

char*** make_cmd_array(const char* command)
{
	char** aux, *cpcommand, ***ret;
	int len, nprog = 0, i;

	if (command == NULL)
		return NULL;
	
	len = strlen(command);

	cpcommand = (char*) malloc(sizeof(char)*(len+1));
	error(cpcommand == NULL, NULL);
	strcpy(cpcommand, command);

	aux = (char**) malloc(sizeof(char*)*len);
	error(aux == NULL, (free(cpcommand),  NULL));

	aux[nprog++] = strtok(cpcommand, SLSH_PIPE);
	while ((aux[nprog++] = strtok(NULL, SLSH_PIPE)) != NULL);

	ret = (char***) malloc(sizeof(char**)*(nprog+1));
	error(ret == NULL, (free(cpcommand),(free(aux), NULL)));

	for (i = 0; i < nprog; i++)
		ret[i] = split_around_blank(aux[i]);
	
	ret[nprog] = NULL;
	free(aux);
	free(cpcommand);
	return ret;
}

void print_job_cmd(char*** cmd)
{
	char*** it, **it2;
	int first = 1;

	if (cmd == NULL)
		return;

	it = cmd;
	while (*it != NULL)
	{
		it2 = *it;
		first = 1;
		while (*it2 != NULL)
		{
			if (first)
				first = 0;
			else
				printf(" ");
			printf("%s", *it2);
			it2++;
		}
		printf("\n");
		it++;
	}
}

int get_io_redir_file(const char* command, int io)
{
	int i = 0, len, fnsize = 0, file;
	char* filename;

	if (io != SLSH_INPUT && io != SLSH_OUTPUT)
		return -1;
	
	len = strlen(command);
	filename = (char*) malloc(sizeof(char)*(len+1));
	error(filename == NULL, -1);

	while (i < len && command[i] != io)
		i++;
	
	if (i == len)
	{
		free(filename);
		return -1;
	}

	i++;
	while (i < len && is_blank(command[i]))
		i++;
	
	if (i == len)
	{
		free(filename);
		return -1;
	}

	while (i < len && !is_blank(command[i]))
	{
		filename[fnsize++] = command[i];
		i++;
	}

	filename[fnsize] = '\0';

	if (io == SLSH_INPUT)
		file = open(filename, O_RDONLY);
	else
		file = open(filename, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	
	free(filename);
	return file;	
}

int is_blocking(const char* command)
{
	int i = strlen(command) - 1;

	while (i)
	{
		if (command[i] == SLSH_NOBLOCK && is_blank(command[i-1]))
			return 0;
		i--;
	}
	return 1;
}

char* clean_command(const char* command)
{
	int len = strlen(command), i = 0;
	char* ret, *rret;
	
	ret = (char*) malloc(sizeof(char)*(len+1));
	error(ret == NULL, NULL);

	while (i < len && command[i] != SLSH_INPUT && command[i] != SLSH_OUTPUT && command[i] != SLSH_NOBLOCK)
	{
		ret[i] = command[i];
		i++;
	}
	
	ret[i] = '\0';
	
	rret = (char*) realloc(ret, sizeof(char)*i);

	if (rret != NULL)
		ret = rret;	
	
	return ret;
}
