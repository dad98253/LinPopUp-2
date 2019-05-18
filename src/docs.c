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

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>



void
doc_show_text_file (int this_doc)
{
  FILE *in;
  struct stat stat_buf;
  gchar *doc_buf;
  char doc_path[1024];
  gchar *doc_filename[HOW_MANY_DOC] = { "COPYING", "README", "ChangeLog", "MANUAL" };
  gchar *doc_title[HOW_MANY_DOC] = { _("GNU General Public License"), _("LinPopUp - Readme"), _("LinPopUp - Changes"), _("LinPopUp - User's Manual") };
  int width[HOW_MANY_DOC] = { 600, 450, 450, 450 };
  int height[HOW_MANY_DOC] = { 350, 250, 250, 250 };


  g_snprintf (doc_path, sizeof(doc_path), "%s/%s", DOC_DIR, doc_filename[this_doc]);


  if (access (doc_path, F_OK) || stat (doc_path, &stat_buf))
    return;

#define DOC_LEN (off_t) stat_buf.st_size
  if ((doc_buf = (char *) g_malloc (DOC_LEN + 1)) == NULL)
    return;

  memset (doc_buf, 0, sizeof (doc_buf));

  in = fopen (doc_path, "r");
  fread (doc_buf, DOC_LEN, 1, in);
  fclose (in);
  doc_buf[DOC_LEN] = 0;

  resizable_text_dialog (doc_title[this_doc], doc_buf, width[this_doc], height[this_doc]);

  g_free (doc_buf);
}


void
doc_show_text_file_from_widget (GtkWidget * widget, gpointer data)
{
  doc_show_text_file ((int) GPOINTER_TO_INT (data));
}
