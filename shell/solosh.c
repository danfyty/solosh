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

const int nbcmd = 6;
const char* builtin_cmd[] = {"bg", "cd", "exit", "fg", "jobs", "quit"};

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

int run_builtin_cmd(char* cmd[])
{
	int id;

	if (cmd == NULL)
		return -1;
	
	id = get_builtin_cmd(cmd[0]);

	switch(id)
	{
		/* bg */
		case 1:
			/* TODO*/
			break;

		/* cd */
		case 2:
			error(chdir(cmd[1]) < 0, -1);
			break;

		/* exit and quit */
		case 3:
		case 6:
			exit(0);
			break;

		/* fg */
		case 4:
			/* TODO */
			break;
		
		/* jobs */
		case 5:
			/* TODO */
			break;

		default:
			return -1;
	}
	return 0;
}

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
			free(**iter);
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

	job->blocking = is_blocking(command);

	cleancmd = clean_command(command);
	error(cleancmd == NULL, (free(job->name), (free(job), NULL)));

	job->cmd = make_cmd_array(cleancmd);
	error(job->cmd == NULL, (free(job->name), (free(job),(free(cleancmd),  NULL))));
	
	free(cleancmd);
	return job;
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

pid_t runcmd(char* cmd[], int input_file, int output_file, int block, int** pipes, int npipes)
{
	pid_t cpid;

	if (cmd == NULL)
		return -1;

	if (get_builtin_cmd(cmd[0]))
		return run_builtin_cmd(cmd);

	cpid = fork();

	error(cpid < 0, -1);
	
	if (cpid == 0)
	{
		if (input_file != 0)
		{
			close(0);
			error(dup(input_file) < 0, (exit(EXIT_FAILURE), -1));
			close(input_file);
		}
		if (output_file != 1)
		{
			close(1);
			close(2);
			error(dup(output_file) < 0, (exit(EXIT_FAILURE), -1));
			error(dup(output_file) < 0, (exit(EXIT_FAILURE), -1));
			close(output_file);
		}
		destroy_pipes(&pipes, npipes);
		execvp(cmd[0], cmd);
		/* TODO: No such command error */
		exit(EXIT_FAILURE);
	}

	if (block)
		waitpid(cpid, NULL, 0);
	return cpid;
}


int runjob(JOB* job)
{
	char*** it;
	int ncmd = 0, i, **pipes = NULL;

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
		int input, output, block = 0;
		
		if (i == 0)
		{
			if (job->inputfd != -1)
				input = job->inputfd;
			else
				input = 0;
		}
		else
			input = pipes[i-1][0];

		if (i == ncmd-1)
		{
			if (job->outputfd != -1)
				output = job->outputfd;
			else
				output = 1;
			block = job->blocking;
		}
		else
			output = pipes[i][1];
		
		/*printf("runcmd(%s, %d, %d, %d)\n", job->cmd[i][0], input, output, block);*/
		runcmd(job->cmd[i], input, output, block, pipes, ncmd-1);
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
		char* aux;
		/*
		getcwd(str, 1024*sizeof(char));
		printf("@%s ", str);
		*/

		while(aux = fgets(str, 1024, stdin), !strlen(str) && aux != NULL);

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
