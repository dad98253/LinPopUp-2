/* LinPopUp - A Linux enhanced port of Winpopup, running over Samba.
 * Copyright (c)1998-2000 Jean-Marc Jacquet <jm@littleigloo.org>  
 *           (c)2003-2005 Grigory Batalov <bga@altlinux.org>
 * Little Igloo Org  http://www.LittleIgloo.org
 *
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "protos.h"


void
fatal_error (int code, char *data)
{
  char error_message[250];
  char buf[128];
  switch (code)
    {
    case 1:
      g_snprintf (error_message, sizeof(error_message), "Can't read file %s", data);
      break;
    case 2:
      g_snprintf (error_message, sizeof(error_message), "Can't get Environment Variable %s", data);
      break;
    default:
      g_strlcpy (error_message, "Unknown error", sizeof(error_message));
    }
  printf ("\n== %s FATAL ERROR ==\n%s\nExit code (%i).\n\n", what_is_this_stupid_programm_that_have_crashed (buf, sizeof(buf)), error_message, code);
  exit (code);
}



void
vital_check_for_read_access (char *filename)
{
  if (access (filename, R_OK))
    fatal_error (1, filename);
}



int
lock_reg (int fd, int cmd, int type, off_t offset, int whence, off_t len)
{
  struct flock lock;

  lock.l_type = type;
  lock.l_start = offset;
  lock.l_whence = whence;
  lock.l_len = len;		/* 0 .. EOF */

  return (fcntl (fd, cmd, &lock));
}




/* EOF */
