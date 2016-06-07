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

/* JOB MANIPULATION */

typedef struct job
{
	char* name;
	char*** cmd;
	int inputfd, outputfd;
	int blocking;
	pid_t pid;
}JOB;

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
	job->pid = 0;

	cleancmd = clean_command(command);
	error(cleancmd == NULL, (free(job->name), (free(job), NULL)));

	job->cmd = make_cmd_array(cleancmd);
	error(job->cmd == NULL, (free(job->name), (free(job),(free(cleancmd),  NULL))));
	
	free(cleancmd);
	return job;
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

/* JOB LIST MANIPULATION */

#define INITIAL_JOB_LIST_CAPACITY 10
#define JL_GET 0
#define JL_DESTROY 1

typedef struct job_list
{
	JOB** v;
	int jobcount, capacity, last;
}JOB_LIST;

JOB_LIST* job_list(int cmd)
{
	static JOB_LIST* list = NULL;
	int i;

	switch(cmd)
	{
		default:
		case JL_GET:
			if (list != NULL)
				return list;
			list = (JOB_LIST*) malloc(sizeof(JOB_LIST));
			error(list == NULL, NULL);

			list->jobcount = 0;
			list->capacity = INITIAL_JOB_LIST_CAPACITY;
			list->last = -1;
			list->v = (JOB**) malloc(sizeof(JOB*)*(list->capacity));
			error(list->v == NULL, (free(list), NULL));
			break;

		case JL_DESTROY:
			if (list == NULL)
				return NULL;
			for (i = 0; i <= list->last; i++)
					destroy_job(&(list->v[i]));
			free(list->v);
			free(list);
			list = NULL;
			break;
	}
	return list;
}

int job_list_push(JOB_LIST* list, JOB* item)
{
	int pos;

	if (list == NULL || item == NULL)
		return -1;
	
	pos = list->last + 1;
	if (pos >= list->capacity)
	{
		JOB** newv;
		
		newv = (JOB**) realloc(list->v, sizeof(JOB*)*(2*list->capacity));
		error(newv == NULL, -1);
		list->v = newv;
		list->capacity *= 2;
	}

	list->v[pos] = item;
	list->last++;
	list->jobcount++;
	return 0;
}

void job_list_erase(JOB_LIST* list, JOB* item)
{
	int idx = 0;

	if (list == NULL || item == NULL)
		return;

	while (idx <= list->last && list->v[idx] != item)
			idx++;

	if (idx > list->last)
		return;
	
	destroy_job(&(list->v[idx]));
	list->jobcount--;

	if (list->jobcount == 0)
		list->last = -1;
}

/* PIPE MANIPULATION */

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

/* EXECUTION OF COMMANDS AND JOBS */

int exit_flag = 0;

int run_builtin_cmd(char* cmd[])
{
	int id, i;
	JOB_LIST* list;

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
			exit_flag = 1;
			break;

		/* fg */
		case 4:
			/* TODO */
			break;
		
		/* jobs */
		case 5:
			list = job_list(JL_GET);
			for (i = 0; i <= list->last; i++)
				printf("[%d] %s\n", i, list->v[i]->name);
			break;

		default:
			return -1;
	}
	return 0;
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
	pid_t lastpid = 0;

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
		lastpid = runcmd(job->cmd[i], input, output, block, pipes, ncmd-1);	
	}

	if (ncmd > 1)
		destroy_pipes(&pipes, ncmd-1);
	
	if (job->blocking)
		job_list_erase(job_list(JL_GET), job);
	else
		job->pid = lastpid;

	return 0;
}

/* SIGNAL HANDLERS */

void sigchld_handler(int sig, siginfo_t* info, void* u)
{
	JOB_LIST* list;
	int i;

	waitpid(info->si_pid, NULL, 0);
	
	list = job_list(JL_GET);
	if (list == NULL) return;
	for (i = 0; i <= list->last; i++)
	{
		if (list->v[i] != NULL && list->v[i]->pid == info->si_pid)
			job_list_erase(list, list->v[i]);
	}
	/*printf("%d ended.\n", info->si_pid);*/
}

/* MAIN PROGRAM */

int main()
{
	char str[1024];
	JOB* job = NULL;
	struct sigaction chld;
	JOB_LIST* list;

	memset(&chld, 0, sizeof(struct sigaction));
	chld.sa_flags |= SA_SIGINFO;
	chld.sa_sigaction = sigchld_handler;
	error(sigaction(SIGCHLD, &chld, NULL) < 0, -1);

	list = job_list(JL_GET);
	error(list == NULL, -1);

	while(!exit_flag)
	{
		char* aux;
		/*
		getcwd(str, 1024*sizeof(char));
		printf("@%s ", str);
		*/

		while(aux = fgets(str, 1024, stdin), !strlen(str) || aux == NULL);

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
		job_list_push(list, job);
		runjob(job);
		str[0] = '\0';
	}

	job_list(JL_DESTROY);
	return 0;
}
