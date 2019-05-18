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

#include "protos.h"
#include <stdio.h>
#include <string.h>
#include "version.h"

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE (!FALSE)
#endif


char *
many_version_infos (char *buf, size_t bufsize)
{
  g_snprintf (buf, bufsize, _("LINPOPUP V%s - (c)LittleIgloo.org 1998-%s.\nCompiled by %s on %s the %s at %s,\nUsing %s and %s."), PROG_VERSION, VERSION_DATE, COMPILE_BY, COMPILE_HOST, COMPILE_DATE, COMPILE_TIME, CC_VERSION, GTK_VERSION);
  return buf;
}



char *
a_little_version_infos (char *buf, size_t bufsize)
{
  g_snprintf (buf, bufsize, "LINPOPUP  V%s - (c)Little Igloo Org 1998-%s", PROG_VERSION, VERSION_DATE);
  return buf;
}



char *
what_is_this_stupid_programm_that_have_crashed (char *buf, size_t bufsize)
{
  g_snprintf (buf, bufsize, "LINPOPUP  V%s", PROG_VERSION);
  return buf;
}



int
how_many_lines_of_code (void)
{
  return CODE_LINES;
}

char
samba_info (char *buf, size_t bufsize)
{
  FILE *output;
  char version_found = FALSE;


  buf[0] = 0;
  if ((output = popen ("smbclient -V 2> /dev/null", "r")) == NULL)
    return FALSE;

  while (fgets (buf, bufsize, output))
    if (strstr (buf, "ersion"))
      {
	version_found = TRUE;
	*strrchr (buf, '\n') = 0;
	memmove (buf, strstr (buf, "ersion") + 6, bufsize);
	break;
      }

  pclose (output);
  return version_found;
}

/* EOF */
