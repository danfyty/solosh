/* t1.c - Test program 1

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
#include <sys/types.h>
#include <unistd.h>

#include <debug.h>

int main (int argc, char **argv)
{
  FILE *fp;
  int pid, val, *p;

  if (argc > 1)
    val = atoi (argv[1]);
  else
    val = 10;

  /* Save pid into a file. */

  pid = getpid();
  fp = fopen ("t1.log", "w");
  sysfatal (!fp);
  fprintf (fp, "%d\n", pid);
  fclose(fp);

  /* Cause a segmentation fault if val==15 */
  p = 0;
  if (val == 15)
    *p = 0;
  
  /* Return argv[1] if given. */

  if (argc == 1)
    return 10;
  else
    return atoi (argv[1]);

}
