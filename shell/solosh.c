#include <solosh_errors.h>
#include <solosh_parser.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

/* ------- JOBS -------*/

typedef struct job
{
	char* name;				/* The job's name as it will be showed by the 'jobs' built-in command. */
	char*** cmd;			/* The commands that make up the job. cmd[i][j] is the j+1-th argument of the i+1-th command and */
	int ncmd;				/* the 1st argument is the commands name. */
	int inputfd, outputfd;	/* Input and output file descriptors. */
	int blocking;			/* Whether the job is blocking (name not ended by '&') or not. */
	pid_t* pid;				/* Will hold the process IDs related to the job. */
	pid_t pgid;				/* Will hold the process group ID of the processes related to the job */
	int run_count;			/* Holds the number of running processes originated from the job */
	time_t lastmodified;		/* Used by bg and fg when executed with no argument */
}JOB;

JOB* create_job(const char* command);
void destroy_job(JOB** job);


/* ------- PIPES ------- */

int** create_pipes(int n);
void destroy_pipes(int*** pipes, int n);


/* ------- JOB LIST ------- */

#define INITIAL_JOB_LIST_CAPACITY 10

#define JL_GET 0								/* Actions for the job_list method. */
#define JL_DESTROY 1

/* Additions to the list are done by run_job (below). Deletions from the list are done by run_job */
/* (if the job is blocking) and by sigchld_handler (if the job is non-blocking). */

typedef struct job_list
{
	JOB** v;
	int jobcount, capacity, last;
}JOB_LIST;

JOB_LIST* job_list(int action);					/* The list is a singleton. This method is used for its creation, retrieval and destruction. */
int job_list_push(JOB* job);
void job_list_erase(const JOB* job);
JOB* job_list_find_by_pid(pid_t pid);
int job_list_find_lastmodified_id();

/* ------- RUN THINGS ------- */

int exit_flag = 0;								/* Tells the main loop when to stop looping. Set by run_builtin_cmd. */
int run_builtin_cmd(char* cmd[]);
pid_t run_cmd(char* cmd[], int input_file, int output_file, int** pipes, int npipes, pid_t session); 	/* The pipes are needed because they must */
int run_job(JOB* job);																					/* be destroyed in the child. */

/* ------- MANAGE RUNNING THINGS ------- */
void fg_wait(JOB* job);										/* This function does the waiting when there's a job on foreground */
void sigchld_handler(int sig, siginfo_t* info, void* u);	/* Handles SIGCHLD for non-blocking jobs */


/* ------- JOBS ------- */

JOB* create_job(const char* command)
{
	JOB* job = NULL;
	char* cleancmd, ***iter;

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
	error(cleancmd == NULL, (destroy_job(&job), NULL));

	job->cmd = make_cmd_array(cleancmd);
	free(cleancmd);
	error(job->cmd == NULL, (destroy_job(&job), NULL));

	job->ncmd = 0;
	iter = job->cmd;
	while (*iter != NULL)
	{
		job->ncmd++;
		iter++;
	}

	job->run_count = 0;
	
	job->pgid = 0;

	job->pid = (pid_t*) calloc(job->ncmd, sizeof(pid_t));
	error(job->pid == NULL, (destroy_job(&job), NULL));

	job->lastmodified = -1;
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
	free((*job)->pid);
	free(*job);
	*job = NULL;
}


/* ------- PIPES ------- */

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


/* ------- JOB LIST ------- */

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

int job_list_push(JOB* item)
{
	int pos;
	JOB_LIST* list = job_list(JL_GET);
	
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

void job_list_erase(const JOB* item)
{
	int idx = 0;
	JOB_LIST* list = job_list(JL_GET);
	
	if (list == NULL || item == NULL)
		return;

	while (idx <= list->last && list->v[idx] != item)
			idx++;

	if (idx > list->last)
		return;
	
	list->v[idx] = NULL;
	list->jobcount--;
	if (list->jobcount == 0)
		list->last = -1;
}

JOB* job_list_find_by_pid(pid_t pid)
{
	int idx, i;
	JOB_LIST* list = job_list(JL_GET);
	
	if (list == NULL)
		return NULL;

	for (idx = 0; idx <= list->last; idx++)
	{
		JOB* job = list->v[idx];
		if (job != NULL)
		{
			for (i = 0; i < job->ncmd; i++)
			{
				if (job->pid[i] == pid)
					return job;
			}
		}
	}
	return NULL;
}

int job_list_find_lastmodified_id()
{
	int idx, ret = -1;
	time_t last = -1;
	JOB_LIST* list = job_list(JL_GET);

	if (list == NULL)
		return -1;

	for (idx = 0; idx <= list->last; idx++)
	{
		JOB* job = list->v[idx];
		if (job != NULL && job->lastmodified > last)
		{
			ret = idx;
			last = job->lastmodified;
		}	
	}
	return ret;
}

/* ------- RUN THINGS -------  */

int run_builtin_cmd(char* cmd[])	/* TODO: make these work correctly inside pipes (io redirection?) */
{
	int id, i, jobid;
	JOB_LIST* list;
	char* path = NULL;

	if (cmd == NULL)
		return -1;
	
	id = get_builtin_cmd(cmd[0]);

	switch(id)
	{
		case CMD_BG:				
			list = job_list(JL_GET);
			
			if (cmd[1] != NULL)
				jobid = atoi(cmd[1]);
			else
				jobid = job_list_find_lastmodified_id();

			if (jobid >= 0 && jobid <= list->last && list->v[jobid] != NULL)
			{
				list->v[jobid]->lastmodified = time(NULL);
				kill(-list->v[jobid]->pgid, SIGCONT);
			}
			break;

		case CMD_CD:
			error(chdir(cmd[1]) < 0, -1);
			path = get_current_dir_name();
			error(path == NULL, -1);
			error(setenv("PWD", path, 1) < 0, (free(path), -1));
			free(path);
			break;

		case CMD_EXIT:
		case CMD_QUIT:
			exit_flag = 1;
			break;

		case CMD_FG:
			list = job_list(JL_GET);
			
			if (cmd[1] != NULL)
				jobid = atoi(cmd[1]);
			else
				jobid = job_list_find_lastmodified_id();

			if (jobid >= 0 && jobid <= list->last && list->v[jobid] != NULL)
			{
				list->v[jobid]->lastmodified = time(NULL);
				list->v[jobid]->blocking = 1;
				kill(-list->v[jobid]->pgid, SIGCONT);
				fg_wait(list->v[jobid]);
			}
			break;
		
		case CMD_JOBS:
			list = job_list(JL_GET);
			for (i = 0; i <= list->last; i++)
				if (list->v[i] != NULL)
					printf("[%d] %s\n", i, list->v[i]->name);
			break;

		default:
			return -1;
	}
	return 0;
}

pid_t run_cmd(char* cmd[], int input_file, int output_file, int** pipes, int npipes, pid_t pgid)
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
		fatal_error(setpgid(0, pgid) < 0, -1);	/* pgid == 0 -> new group with id equal to the current pid */

		if (input_file != 0)
		{
			close(0);
			fatal_error(dup(input_file) < 0, -1);
			close(input_file);
		}
		if (output_file != 1)
		{
			close(1);
			fatal_error(dup(output_file) < 0, -1);
			close(output_file);
		}
		destroy_pipes(&pipes, npipes);
		execvp(cmd[0], cmd);
		/* TODO: No such command error */
		exit(EXIT_FAILURE);
	}
	return cpid;
}

int run_job(JOB* job)
{
	int i, **pipes = NULL;

	if (job == NULL || job->cmd == NULL)
		return -1;

	if (job->ncmd > 1)
	{
		pipes = create_pipes(job->ncmd-1);
		if (pipes == NULL)
			return -1;
	}

	job->run_count = job->ncmd;
	job_list_push(job);
	job->pgid = 0;

	for (i = 0; i < job->ncmd; i++)
	{
		int input, output;
		
		if (i == 0)
		{
			if (job->inputfd != -1)
				input = job->inputfd;
			else
				input = 0;
		}
		else
			input = pipes[i-1][0];

		if (i == job->ncmd-1)
		{
			if (job->outputfd != -1)
				output = job->outputfd;
			else
				output = 1;
		}
		else
			output = pipes[i][1];
		
		job->pid[i] = run_cmd(job->cmd[i], input, output, pipes, job->ncmd-1, job->pgid);
		if (job->pid[i] <= 0)
			job->run_count--;		/* Built-in commands and failed executions are not running processes */
		else if (job->pgid == 0)
			job->pgid = job->pid[i];		
	}
	
	destroy_pipes(&pipes, job->ncmd-1);

	if (job->blocking)
		fg_wait(job);
	
	return 0;
}

/* ------- MANAGE RUNNING THINGS -------*/

void fg_wait(JOB* job)
{
	int i;

	tcsetpgrp(1, job->pgid);
	for (i = 0; i < job->ncmd && job->blocking; i++)
	{
		if (job->pid[i] > 0)
		{
			while (waitpid(job->pid[i], NULL, 0) < 0 && job->blocking); 
			/* A suspended job stops being blocking (sigchld_handler sets job->blocking to 0) 			*/
			/* The while loop is needed because if the child is killed then the wait will be canceled 	*/
			/* (returning -1) by the call to sigchld_handler. It must be called again, otherwise zombie */
			/* processes would remain: sigchld_handler doesn't wait for blocking jobs 					*/

		}
	}
	tcsetpgrp(1, getpgid(0));
	
	if (job->blocking)		/* Blocking job has terminated */
	{
		job_list_erase(job);
		destroy_job(&job);
	}
}

void sigchld_handler(int sig, siginfo_t* info, void* u)
{
	JOB* job;

	job = job_list_find_by_pid(info->si_pid);
	if (job == NULL)	
		return;

	switch(info->si_code)
	{
		case CLD_EXITED:
			if (!job->blocking)	
			{
				waitpid(info->si_pid, NULL, 0);
				if (--job->run_count == 0)
				{
					job_list_erase(job);
					destroy_job(&job);
				}
				
			}
			break;

		case CLD_KILLED:
			waitpid(info->si_pid, NULL, 0);
			job_list_erase(job);
			destroy_job(&job);
			break;

		case CLD_STOPPED:
			job->blocking = 0;		
			break;

		default:
			break;
	}
}


/* MAIN PROGRAM */

int main()
{
	char str[1024];
	JOB* job = NULL;
	struct sigaction chld, ttou;

	setpgid(0, 0);

	memset(&chld, 0, sizeof(struct sigaction));
	chld.sa_flags |= SA_SIGINFO;
	chld.sa_sigaction = sigchld_handler;
	error(sigaction(SIGCHLD, &chld, NULL) < 0, -1);

	memset(&ttou, 0, sizeof(struct sigaction));
	ttou.sa_handler = SIG_IGN;
	error(sigaction(SIGTTOU, &ttou, NULL) < 0, -1);

	while(!exit_flag)
	{
		char* aux;
		/*
		getcwd(str, 1024*sizeof(char));
		printf("@%s ", str);
		*/

		while(aux = fgets(str, 1024, stdin), !strlen(str) || aux == NULL);	/* TODO: not use static buffer ? */

		str[strlen(str)-1] = '\0';
		job = create_job(str);

		run_job(job);
		str[0] = '\0';
	}

	job_list(JL_DESTROY);
	return 0;
}
