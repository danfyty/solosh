/* test-runcmd - Program to test runcmd function

   Copyright (c) 2014, Francisco José Monaco <moanco@icmc.usp.br>

   This file is part of POSIXeg

   POSIXeg is free software: you can redistribute it and/or modify
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



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <runcmd.h>
#include <debug.h>

#define LOGFILE "runcmd.log"
#define EXECYES 0
#define EXECFAIL 1

typedef struct test_case_t
{
  char *info;
  char *cmd;
} test_case_t;


int go_on = 1;

void finish (void)
{
  go_on = 0;
}

int noio[3];

int try_runcmd (const char* command, int *result, int *io)
{
  int pid, tio[3], tmp_result, nbytes;

  printf ("\nCommand %s%n", command, &nbytes);
  printf ("%*s", 20-nbytes, " ");

  /* Supress output if not redirecting. */
  if (!io)
    {
      tio[0]=dup(0); tio[1]=dup(1); tio[2]=dup(2);
      close (0); close(1); close (2);
      dup(noio[0]); dup(noio[1]); dup(noio[2]);
    }

  /* Run the command.*/

  runcmd_onexit = finish;

  pid = runcmd (command,  &tmp_result, io);

  /* Restore output if not redirecting. */
  if (!io)
    {
      close (0); close (1); close (2);
      dup(tio[0]); dup(tio[1]); dup(tio[2]);
      close (tio[0]); close(tio[1]); close (tio[2]);
    }

  printf ("(%5d, %8s, %3d, %3sblocking, exec %s)\n\n",
  	  pid,
  	  IS_NORMTERM(tmp_result) ? "normal" : "abnormal",
  	  EXITSTATUS(tmp_result),
  	  IS_NONBLOCK(tmp_result) ? "non" : "",
  	  IS_EXECOK(tmp_result) ? "success" : "failed");

  if (result)
    *result = tmp_result;

  return pid;

}

int check (const char* message, int expression)
{
  int nbytes;
  printf ("   whether %s%n", message, &nbytes);
  printf ("%*s\n", 60-nbytes, expression ? "OK":"NO");
  return expression ? 0 : 1;
}


/* Main function: returns successfully if all tests are ok;
   returns unsucessfully otherwise. */

int main (int argc, char **argv)
{
  /* int result, i; */

  /* Test cases. */

  /* char cmd1[] = "ls runcmd.c" ;     /\* File does exist. *\/ */
  /* char cmd2[] = "./segfault";       /\* Abnormal termination. *\/ */
  /* char cmd3[] = "ls nosuchfile";    /\* File does not exist. *\/ */
  /* char cmd4[] = "./io";  	    /\* Test IO redirection. *\/ */
  /* char cmd5[] = "./nosuchfile";     /\* Exec failed. *\/ */
  /* char cmd6[] = "./delay &";        /\* Test nonblock. *\/ */


  char cmd1[] = "./t1" ;          /* Exits 10. */
  char cmd2[] = "./t1 11";        /* Exits 11. */
  char cmd3[] = "./t1 15";        /* Segaful. */
  char cmd4[] = "./nosuchfile";   /* Not found. */
  char cmd5[] = "./t1 127";       /* Exits EXECFAILSTATUS */

  /* int io[3], io2[3], pid, rpid, nerrors; */
  int result, i, pid, io[3], rpid, nerrors;
  FILE *fp;

  nerrors = 0;


  /* Disable standard streams if not redirecting. */

  sysfatal ((noio[0] = open ("/dev/null", O_WRONLY)) <0);
  sysfatal ((noio[1] = open ("/dev/null", O_WRONLY)) <0);
  sysfatal ((noio[2] = open ("/dev/null", O_WRONLY)) <0);

  /* io[0]=1; io[1]=1; io[2]=2; */

  /* Check  */

  pid = try_runcmd (cmd3, &result, NULL);   /* Normal, success. */
  printf ("  Checking...\n");

  nerrors += check ("abnormal termination is correctly reported", 
		    (!IS_NORMTERM(result)) && 
		    (EXITSTATUS(result) == 0));
  /* Check */


  pid = try_runcmd (cmd1, &result, NULL);   /* Normal, success. */
  printf ("  Checking...\n");

  fp = fopen ("t1.log", "r");
  sysfatal (!fp);
  fscanf (fp, "%d", &rpid);

  nerrors += check ("normal termination is correctly reported", 
		    IS_NORMTERM(result));

  nerrors += check ("pid is correctly returned on success", 
		    (IS_NORMTERM(result)) && 
		    (pid == rpid));

  nerrors += check ("exit status is correctly reported on sucess", 
		    (IS_NORMTERM(result)) &&
		    (EXITSTATUS(result) == 10));

  nerrors += check ("exec success if correctly reported", 
		    (IS_NORMTERM(result)) &&
		    (IS_EXECOK(result)));


  fclose (fp);
  unlink ("t1.log");

  /* Check  */

  pid = try_runcmd (cmd2, &result, NULL);   /* Normal, success. */
  printf ("  Checking...\n");

  nerrors += check ("command line arguments are correctly read", EXITSTATUS(result) == 11);

  /* Check */
  
  pid = try_runcmd (cmd4, &result, NULL);   /* Normal, success. */
  printf ("  Checking...\n");

  nerrors += check ("exec failure is correctly reported (a)", 
		    (!IS_EXECOK(result)) &&
		    (EXITSTATUS(result) == EXECFAILSTATUS));
  
  /* Check */
  
  pid = try_runcmd (cmd5, &result, NULL);   /* Normal, success. */
  printf ("  Checking...\n");

  nerrors += check ("exec failure is not reported on sucess (b)", 
		    (IS_EXECOK(result)) &&
		    (EXITSTATUS(result) == EXECFAILSTATUS));

  /* Politeness. */

  for (i=0; i<3; i++)
    {
      close (noio[i]);
      close (io[i]);
    }


  return nerrors;


  
  /* Test redirection. */

  /* sysfatal ((fd = open ("in.txt", O_CREAT | O_TRUNC | O_RDWR,  S_IRUSR | S_IWUSR)) <0); */
  /* write (fd, "a", 1); */
  /* sysfatal(close(fd)<0); */

  /* sysfatal ((io2[0] = open ("in.txt", O_RDONLY)) <0); */
  /* sysfatal ((io2[1] = open ("out.txt", O_CREAT | O_TRUNC | O_RDWR,  S_IRUSR | S_IWUSR)) <0); */
  /* sysfatal ((io2[2] = open ("err.txt", O_CREAT | O_TRUNC | O_RDWR,  S_IRUSR | S_IWUSR)) <0); */


  /* result +=tryrun (&tc[3], io2, 1, 0, EXECYES);   /\* Normal, success. *\/ */

  /* /\* Test nonblock. *\/ */

  /* result +=tryrun (&tc[5], io2, 1, 0, EXECFAIL);   /\* Normal, success. *\/ */

  /* /\* Test whether exec failed. *\/ */

  /* result +=tryrun (&tc[5], io, 1, 0, EXECYES);   /\* Normal, success. *\/ */

  /* /\* Polite clean-up before leaving. *\/ */

  /* return result; */
}
