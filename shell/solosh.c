#include <solosh_errors.h>
#include <solosh_parser.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
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
			/*free(**iter); TODO MEMORY LEAK FIX*/
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

pid_t runcmd(char* cmd[], int input_file, int output_file, int block, int close_this, int close_this2)
{
	pid_t cpid = fork();

	error(cpid < 0, -1);
	
	if (cpid == 0)
	{
		if (close_this != -1)
			close(close_this);

		if (close_this2 != -1)
			close(close_this2);

		if (input_file != 0)
		{
			close(0);
			dup(input_file);
			close(input_file);
		}
		if (output_file != 1)
		{
			close(1);
			close(2);
			dup(output_file);
			dup(output_file);
			close(output_file);
		}
		execvp(cmd[0], cmd);
		exit(EXIT_FAILURE);
	}

	if (block)
		waitpid(cpid, NULL, 0);
	return cpid;
}

void destroy_pipes(int*** pipes, int n)
{
	int i, **p;

	if (pipes == NULL)
		return;

	p = *pipes;

	for (i = 0; i < n; i++)
	{
		if (p[i] != NULL)
		{
			close(p[i][0]);
			close(p[i][1]);
		}
		free(p[i]);
	}
	free(p);
	*pipes = NULL;
}

int** create_pipes(int n)
{
	int** pipes = (int**) malloc(sizeof(int*)*n), i;
	error(pipes == NULL, NULL);

	for (i = 0; i < n; i++)
	{
		pipes[i] = (int*) malloc(sizeof(int)*2);
		if (pipes[i] == NULL || pipe(pipes[i]) < 0)
		{
			destroy_pipes(&pipes, n);
			error(1, NULL);
		}
	}
	return pipes;
}

int runjob(JOB* job)
{
	char*** it;
	int ncmd = 0, i, **pipes;

	if (job == NULL || job->cmd == NULL)
		return -1;

	it = job->cmd;
	while (*it != NULL)
	{
		ncmd++;
		it++;
	}

	if (ncmd > 1)
	{
		pipes = create_pipes(ncmd-1);
		if (pipes == NULL)
			return -1;
	}

	for (i = 0; i < ncmd; i++)
	{
		int input, output, block = 0, close_this = -1, close_this2 = -1;
		
		if (i == 0)
		{
			if (job->inputfd != -1)
				input = job->inputfd;
			else
				input = 0;
		}
		else
		{
			input = pipes[i-1][0];
			close_this = pipes[i-1][1];
		}

		if (i == ncmd-1)
		{
			if (job->outputfd != -1)
				output = job->outputfd;
			else
				output = 1;
			block = job->blocking;
		}
		else
		{
			output = pipes[i][1];
			close_this2 = pipes[i][0];
		}
		
		/*printf("runcmd(%s, %d, %d, %d, %d, %d)\n", job->cmd[i][0], input, output, block, close_this, close_this2);*/
		runcmd(job->cmd[i], input, output, block, close_this, close_this2);
	}

	if (ncmd > 1)
		destroy_pipes(&pipes, ncmd-1);
	
	return 0;
}

void sigchld_handler(int sig, siginfo_t* info, void* u)
{
	waitpid(info->si_pid, NULL, 0);
	/*printf("%d ended.\n", info->si_pid);*/
}

int main()
{
	char str[1024];
	JOB* job = NULL;
	struct sigaction chld;

	memset(&chld, 0, sizeof(struct sigaction));
	chld.sa_flags |= SA_SIGINFO;
	chld.sa_sigaction = sigchld_handler;
	error(sigaction(SIGCHLD, &chld, NULL) < 0, -1);


	while(1)
	{
		while(fgets(str, 1024, stdin), !strlen(str));

		str[strlen(str)-1] = '\0';
		job = create_job(str);
/*
		printf("job name: %s\n", job->name);
		
		printf("job commands:\n");
		print_job_cmd(job->cmd);

		printf("job inputfd = %d\n", job->inputfd);
		printf("job outputfd = %d\n", job->outputfd);
		printf("job blocking = %d\n", job->blocking);
*/
		runjob(job);
		destroy_job(&job);
		str[0] = '\0';
	}
	return 0;
}
