#ifndef SOLOSH_PARSER_H
#define SOLOSH_PARSER_H

#define SLSH_BLANK_CHARS " \t"
#define SLSH_PIPE "|"

#define SLSH_INPUT '<'
#define SLSH_OUTPUT '>'
#define SLSH_NOBLOCK '&'

char** split_around_blank(const char* str);
char*** make_cmd_array(const char* command);
void print_job_cmd(char*** cmd);
int get_io_redir_file(const char* command, int io);
char* clean_command(const char* command);
int is_noblock(const char* command);

#endif
