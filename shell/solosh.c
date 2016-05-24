#include <solosh_errors.h>
#include <solosh_parser.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct job
{
	char* name;
	char*** cmd;
	int inputfd, outputfd;
	int blocking;
}JOB;

void destroy_job(JOB** job)
{
	char*** iter;

	if (job == NULL || *job == NULL)
		return;

	if ((*job)->inputfd >= 0)
		close((*job)->inputfd);
	
	if ((*job)->outputfd >= 0)
		close((*job)->outputfd);
	
	iter = (*job)->cmd;
	if (iter != NULL)
	{
		while (*iter != NULL)
		{
			free(*iter);
			iter++;
		}
	}
	free((*job)->cmd);
	free((*job)->name);
	free(*job);
	*job = NULL;
}

JOB* create_job(const char* command)
{
	JOB* job = NULL;
	char* cleancmd;

	if (command == NULL)
		return NULL;

	job = (JOB*) malloc(sizeof(JOB));
	error(job == NULL, NULL);

	job->name = (char*) malloc(sizeof(char)*(strlen(command)+1));
	error(job->name == NULL, (free(job), NULL));
	strcpy(job->name, command);

	job->inputfd = get_io_redir_file(command, SLSH_INPUT);
	job->outputfd = get_io_redir_file(command, SLSH_OUTPUT);

	job->blocking = is_noblock(command);

	cleancmd = clean_command(command);
	error(cleancmd == NULL, (free(job->name), (free(job), NULL)));

	job->cmd = make_cmd_array(cleancmd);
	error(job->cmd == NULL, (free(job->name), (free(job),(free(cleancmd),  NULL))));
	
	free(cleancmd);
	return job;	
}

int main()
{
	char str[1024];
	JOB* job = NULL;

	while(fgets(str, 1024, stdin) != NULL)
	{
		str[strlen(str)-1] = '\0';
		job = create_job(str);
		printf("job name: %s\n", job->name);
		
		printf("job command: ");
		print_job_cmd(job->cmd);
		printf("\n");

		printf("job inputfd = %d\n", job->inputfd);
		printf("job outputfd = %d\n", job->outputfd);
		printf("job blocking = %d\n", job->blocking);
		
		destroy_job(&job);
	}
	return 0;
}
