#include <fcntl.h>
#include <solosh_errors.h>
#include <solosh_parser.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define INITIAL_TOKEN_ARRAY_CAP 10

char** split_around_blank(const char* str)
{
	int len, captok = INITIAL_TOKEN_ARRAY_CAP, ntok = 0;
	char** s, *cpstr;

	if (str == NULL)
		return NULL;

	len = strlen(str);

	cpstr = (char*) malloc(sizeof(char)*(len+1));
	error(cpstr == NULL, NULL);
	strcpy(cpstr, str);
	
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
	char* aux, *cpcommand, ***ret;
	int len, nprog = 0, capprog = INITIAL_TOKEN_ARRAY_CAP, i;

	if (command == NULL)
		return NULL;
	
	len = strlen(command);

	cpcommand = (char*) malloc(sizeof(char)*(len+1));
	error(cpcommand == NULL, NULL);
	strcpy(cpcommand, command);

	ret = (char***) malloc(sizeof(char**)*capprog);
	error(ret == NULL, (free(cpcommand), NULL));

	aux = strtok(cpcommand, SLSH_PIPE);
	while (aux != NULL)
	{
		ret[nprog++] = split_around_blank(aux);
		if (nprog == capprog)
		{
			char*** new_ret;

			capprog *= 2;
			new_ret = (char***) realloc(ret, sizeof(char**)*capprog);
			if (new_ret == NULL)
			{
				free(cpcommand);
				for (i = 0; i < nprog; i++)
					free(ret[i]);
				free(ret);
				return NULL;
			}
			ret = new_ret;
		}
		aux = strtok(NULL, "|");
	}
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
		it2 = (*it);
		while(*it2 != NULL)
		{
			if (first)
				first = 0;
			else
				printf(" ");
			printf("%s", *it2);
			it2++;
		}
		it++;
	}
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
		file = open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	
	free(filename);
	return file;	
}

int is_noblock(const char* command)
{
	int i = strlen(command) - 1;

	while (i)
	{
		if (command[i] == SLSH_NOBLOCK && is_blank(command[i-1]))
			return 1;
		i--;
	}
	return 0;
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
