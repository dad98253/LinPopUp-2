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


#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "protos.h"



extern int mozilla_remote (char *command);


void
execute_program (char *program, char *arg)
{
  pid_t pid;

  pid = fork ();
  if (pid == 0)
    {
      pid = fork ();
      if (pid == 0)
	{
	  execlp (program, program, arg, NULL);
	  _exit (0);
	}
      else
	{
	  _exit (0);
	}
    }

}





/* BROWSER functions */

/* PUBLIC:
 * open_web_browser
 * open_mail_client
 * open_new_message
 * open_news_client 
 */

void
open_URL (char *this_url)
{
  char remote_url_buf[2048];
  char program_url_buf[2048];

  if (USE_NETSCAPE)
    {
      if (this_url != NULL)
	{
	  g_snprintf (remote_url_buf, sizeof(remote_url_buf), "openURL(%s,new-window)", this_url);
	  g_snprintf (program_url_buf, sizeof(program_url_buf), "%s", this_url);
	}
      else
	{
	  g_snprintf (remote_url_buf, sizeof(remote_url_buf), "openURL(blank,new-window)");
	  g_snprintf (program_url_buf, sizeof(program_url_buf), " ");
	}

      if (!mozilla_remote (remote_url_buf))
	execute_program ("netscape", program_url_buf);

    }
  else
    {
      // Not Yet Implemented
    }


}


void
open_URL_from_widget (GtkWidget * widget, gpointer data)
{
  open_URL (data);
}



void
open_mail_client (void)
{
  if (USE_NETSCAPE)
    {
      if (!mozilla_remote ("openInbox"))
	execute_program ("netscape", "-mail");
    }
  else
    {
      // Not Yet Implemented
    }
}



void
open_new_message (void)
{
  if (USE_NETSCAPE)
    {
      if (!mozilla_remote ("composeMessage"))
	execute_program ("netscape", "mailto:");
    }
  else
    {
      // Not Yet Implemented
    }
}




void
open_news_client (void)
{

  if (USE_NETSCAPE)
    {
      if (!mozilla_remote ("openNewsgroups"))
	execute_program ("netscape", "-news");
    }
  else
    {
      // Not Yet Implemented
    }

}



/* EOF */
