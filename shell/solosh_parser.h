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
