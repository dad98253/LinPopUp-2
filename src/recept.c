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

#define _GNU_SOURCE

#include <stdio.h>
#include <error.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include "protos.h"
#include <gtk/gtk.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <syslog.h>

static GtkWidget *msg_file_sel;
static char SAVE_ALL_MSG;


void
update_main_statusbar (int message_number, char NEW)
{
  char buf[100];
  int f;

  for (f = 0; f < 4; f++)
    {
      gtk_statusbar_remove (GTK_STATUSBAR (main_statusbar[f]), main_context_id[f], main_message_id[f]);
      main_message_id[f] = gtk_statusbar_push (GTK_STATUSBAR (main_statusbar[f]), main_context_id[f], "");
    }

  gtk_statusbar_remove (GTK_STATUSBAR (main_statusbar[0]), main_context_id[0], main_message_id[0]);
  if (message_number)
    g_snprintf (buf, sizeof(buf), _("Message #%i"), message_number);
  else
    g_strlcpy (buf, _("No Message"), sizeof(buf));
  main_message_id[0] = gtk_statusbar_push (GTK_STATUSBAR (main_statusbar[0]), main_context_id[0], buf);

  if (NEW)
    {
      gtk_statusbar_remove (GTK_STATUSBAR (main_statusbar[1]), main_context_id[1], main_message_id[1]);
      main_message_id[1] = gtk_statusbar_push (GTK_STATUSBAR (main_statusbar[1]), main_context_id[1], _("NEW"));
    }

  if (total_number_of_messages)
    {
      g_snprintf (buf, sizeof(buf), _("Total Messages: %i"), total_number_of_messages);
      gtk_statusbar_remove (GTK_STATUSBAR (main_statusbar[2]), main_context_id[2], main_message_id[2]);
      main_message_id[2] = gtk_statusbar_push (GTK_STATUSBAR (main_statusbar[2]), main_context_id[2], buf);
    }

  if (new_messages)
    {
      g_snprintf (buf, sizeof(buf), _("New: %i"), new_messages);
      gtk_statusbar_remove (GTK_STATUSBAR (main_statusbar[3]), main_context_id[3], main_message_id[3]);
      main_message_id[3] = gtk_statusbar_push (GTK_STATUSBAR (main_statusbar[3]), main_context_id[3], buf);
    }
}



/* -- read from data file message #message_number -- */
void
read_message (int message_number, struct linpopup_message *this_message)
{
  int file_handle;
  char *cP;
  file_handle = open (DATA_FILE, O_RDONLY, DATA_PERM);
  /* lock it for reading .. */
  (void) lock_reg (file_handle, F_SETLKW, F_RDLCK, 0, SEEK_SET, 0);
  lseek (file_handle, (message_number - 1) * sizeof (*this_message), SEEK_SET);
  read (file_handle, this_message, sizeof (*this_message));
  while((cP = strchr(this_message->text, '\x14')) != NULL)
   {
    cP[0] = '\x0A';
   }
  /* unlock */
  (void) lock_reg (file_handle, F_SETLK, F_UNLCK, 0, SEEK_SET, 0);
  close (file_handle);
}



/* -- save to  data file message #message_number -- */
void
write_message (int message_number, struct linpopup_message *this_message)
{
  int file_handle;
  file_handle = open (DATA_FILE, O_WRONLY, DATA_PERM);
  /* lock it for writing .. */
  (void) lock_reg (file_handle, F_SETLKW, F_WRLCK, 0, SEEK_SET, 0);
  lseek (file_handle, (message_number - 1) * sizeof (*this_message), SEEK_SET);
  write (file_handle, this_message, sizeof (*this_message));
  /* unlock */
  (void) lock_reg (file_handle, F_SETLK, F_UNLCK, 0, SEEK_SET, 0);
  close (file_handle);
}



/* -- unset the is_new flag of message #message_number -- */
void
this_message_is_no_longer_new (int message_number)
{
  struct linpopup_message this_message;
  read_message (message_number, &this_message);
  this_message.is_new = FALSE;
  write_message (message_number, &this_message);
  new_messages--;
}



/* -- show_no_message -- */
void
show_no_message (void)
{
  char buf[1024];
  char *ptr;
  static GdkColor text_fg = { 0, 0x8000, 0x8000, 0x8000 };
  GdkColormap *colormap;
  GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (main_text));
  GtkTextIter start, end;
  GtkTextTag *tag;

  clear_text (main_text);
  colormap = gtk_widget_get_colormap (window);
  gdk_color_alloc (colormap, &text_fg);


  many_version_infos (buf, sizeof(buf));
  ptr = strchr (buf, '\n');

  gtk_text_buffer_get_end_iter (buffer, &end);
  gtk_text_buffer_insert (buffer, &end,
     _("Distributed under the terms of the GNU GPL.\n"), -1);

  *ptr = 0;
  gtk_label_set_text (GTK_LABEL (main_label_from), buf);

  if (samba_installed)
    g_snprintf (buf, sizeof(buf), _("Samba version %s installed.\n"), samba_version + 1);

  gtk_text_buffer_get_end_iter (buffer, &end);
  gtk_text_buffer_insert (buffer, &end, buf, -1);

  if (strstr (samba_version, " 1.") != NULL)
     { gtk_text_buffer_get_end_iter (buffer, &end);
       gtk_text_buffer_insert (buffer, &end,
          _("\nWARNING:\n\nDue to a security hole found in Samba message command you may install Samba version >= 2.0 Beta 2 when using LinPopUp."), -1);
      }

  if ((tag = gtk_text_tag_table_lookup (gtk_text_buffer_get_tag_table (buffer), "grey")) == NULL)
     tag = gtk_text_buffer_create_tag (buffer, "grey", "foreground-gdk", &text_fg, NULL);
  gtk_text_buffer_get_bounds (buffer, &start, &end);
  gtk_text_buffer_apply_tag (buffer, tag, &start, &end);

  current_showed_message = 0;
  update_main_statusbar (0, FALSE);
}



/* -- show message #message_number -- */
void
show_message (int message_number)
{
  struct linpopup_message this_message;
  char buf[250];
  char time_buf[30];
  struct tm *timeptr;
  static GdkColor header_fg = { 0, 0x8000, 0x0000, 0x0000 };
  GdkColormap *colormap = gtk_widget_get_colormap (window);

  gdk_color_alloc (colormap, &header_fg);

  read_message (message_number, &this_message);

  timeptr = localtime (&this_message.time);
  strftime (time_buf, sizeof (time_buf), "%a %d %b %Y  %X", timeptr);
  g_snprintf (buf, sizeof(buf), _("Received from %s at %s, %s"), this_message.sender,
     this_message.host, g_locale_to_utf8 (time_buf, -1, NULL, NULL, NULL));
  gtk_label_set_text (GTK_LABEL (main_label_from), buf);

  clear_text (main_text);
  realize_text (main_text, g_locale_to_utf8 (this_message.text, -1, NULL, NULL, NULL));

  if (this_message.is_new)
    {
      update_main_statusbar (message_number, TRUE);
      this_message_is_no_longer_new (message_number);
    }
  else
    update_main_statusbar (message_number, FALSE);

  gtk_window_deiconify (GTK_WINDOW (window));
  current_showed_message = message_number;
  g_strlcpy (user_rc.guess_adr, this_message.host, sizeof(user_rc.guess_adr));
}



/* -- show next message -- */
void
show_next_message (gpointer * data)
{
  if (!total_number_of_messages)
    {
      show_no_message ();
      return;
    }

  if (current_showed_message == total_number_of_messages)
    user_rc.wrap_to_first ? show_message (1) : show_message (total_number_of_messages);
  else
    show_message (current_showed_message + 1);

}



/* -- show previous message -- */
void
show_previous_message (gpointer * data)
{
  if (!total_number_of_messages)
    {
      show_no_message ();
      return;
    }

  if (current_showed_message == 1)
    user_rc.wrap_to_first ? show_message (total_number_of_messages) : show_message (1);
  else
    show_message (current_showed_message - 1);
}



/* -- count  messages in data file -- */
int
count_total_number_of_messages (void)
{
  struct stat data_file;
  int data_file_len;

  if (access (DATA_FILE, F_OK))
    return 0;

  stat (DATA_FILE, &data_file);
  data_file_len = ((off_t) data_file.st_size / sizeof (struct linpopup_message));
  return data_file_len;
}



/* -- save message #message_number in a human readable form -- */
static void
save_message (FILE * out, int message_number)
{
  struct linpopup_message this_message;
  char time_buf[30];
  struct tm *timeptr;
  gchar *sender, *host, *to;

  read_message (message_number, &this_message);
  timeptr = localtime (&this_message.time);
  strftime (time_buf, sizeof (time_buf), "%a %d %b %Y  %X", timeptr);

  sender = g_utf8_strup (this_message.sender, -1);
  host = g_utf8_strup (this_message.host, -1);
  to = g_utf8_strup (this_message.to, -1);

  fprintf (out, g_locale_from_utf8 (_("FROM : %s ON %s\nTO   : %s\nDATE : %s\n---\n\n%s\n=== END OF MESSAGE ===\n\n\n\n\n\n"),
     -1, NULL, NULL, NULL), sender, host, to, time_buf, this_message.text);

  g_free (sender);
  g_free (host);
  g_free (to);
}



/* == Message save fileselector  ==  */
static void
message_save_ok (GtkWidget * widget, GtkFileSelection * file_sel)
{
  gchar *filename;
  FILE *out;

  filename = (gchar *) gtk_file_selection_get_filename (GTK_FILE_SELECTION (file_sel));
  if (filename != NULL)
    {
      if (!access (filename, F_OK))
	{
	  char buf[1024];
	  g_snprintf (buf, sizeof(buf), _("\n %s already exists\nErase it anyway ?"), strrchr (filename, '/') + 1);
	  if (alert_dialog (_("Confirmation box"), buf, _("Yes"), _("No"), 2) == 2)
	    return;
	}
      if ((out = fopen (filename, "w")))
	{
	  if (SAVE_ALL_MSG)
	    {
	      int f;
	      for (f = 1; f <= total_number_of_messages; f++)
		{
		  fprintf (out, g_locale_from_utf8 (_("LINPOPUP MESSAGE #%i \n"),
                    -1, NULL, NULL, NULL), f);
		  save_message (out, f);
		}
	    }
	  else
	    {
	      fprintf (out, g_locale_from_utf8 (_("LINPOPUP MESSAGE\n"),
                -1, NULL, NULL, NULL));
	      save_message (out, current_showed_message);
	    }
	  fclose (out);
	}
      else
	{
	  char buf[1024];
	  g_snprintf (buf, sizeof(buf), _("\nCan not write %s!"), strrchr (filename, '/') + 1);
	  alert_ok (_("Error"), buf, _("Ok"));
	  return;
	}
    }
  gtk_widget_destroy (GTK_WIDGET (file_sel));
  msg_file_sel = NULL;
}



static void
message_save_cancel (GtkWidget * widget, GtkFileSelection * file_sel)
{
  gtk_widget_destroy (GTK_WIDGET (file_sel));
  msg_file_sel = NULL;
}



void
message_save (GtkWidget * widget, int data)
{
  if (!total_number_of_messages)
    return;

  if (msg_file_sel == NULL)
    {
      SAVE_ALL_MSG = (int) data;
      msg_file_sel = gtk_file_selection_new (_("Save Message..."));
      g_signal_connect (GTK_OBJECT (msg_file_sel), "destroy", GTK_SIGNAL_FUNC (message_save_cancel), msg_file_sel);
      g_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (msg_file_sel)->ok_button), "clicked", GTK_SIGNAL_FUNC (message_save_ok), msg_file_sel);
      g_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (msg_file_sel)->cancel_button), "clicked", GTK_SIGNAL_FUNC (message_save_cancel), msg_file_sel);
    }

  if (GTK_WIDGET_VISIBLE (msg_file_sel))
    return;
  else
    gtk_widget_show (msg_file_sel);
}



/* -- delete message current_showed message from  data file -- */
void
delete_showed_message (void)
{
  struct linpopup_message this_message;
  char *fname;
  int file_handle_temp, file_handle_data;
  int f;


  if (!total_number_of_messages)
    {
      show_no_message ();
      return;
    }

  if (asprintf (&fname, "%s/linpopup.XXXXXX", getenv ("TMPDIR") ?: P_tmpdir) < 0)
	error (EXIT_FAILURE, errno, "asprintf");

  if ((file_handle_temp = mkstemp (fname)) < 0)
	error (EXIT_FAILURE, errno, "mkstemp: %s", fname);

  if (unlink (fname) < 0)
	error (EXIT_FAILURE, errno, "unlink: %s", fname);

  file_handle_data = open (DATA_FILE, O_RDONLY, DATA_PERM);
  /* lock it for reading .. */
  (void) lock_reg (file_handle_data, F_SETLKW, F_RDLCK, 0, SEEK_SET, 0);


  lseek (file_handle_data, 0L, SEEK_SET);
  for (f = 1; f <= total_number_of_messages; f++)
    {
      read (file_handle_data, &this_message, sizeof (this_message));
      write (file_handle_temp, &this_message, sizeof (this_message));
    }

  /* unlock */
  (void) lock_reg (file_handle_data, F_SETLK, F_UNLCK, 0, SEEK_SET, 0);
  close (file_handle_data);

  lseek (file_handle_temp, 0, SEEK_SET);
  file_handle_data = open (DATA_FILE, O_RDWR | O_TRUNC, DATA_PERM);
  /* lock it for writing .. */
  (void) lock_reg (file_handle_data, F_SETLKW, F_WRLCK, 0, SEEK_SET, 0);

  for (f = 1; f <= total_number_of_messages; f++)
    {

      read (file_handle_temp, &this_message, sizeof (this_message));
      if (f != current_showed_message)
	write (file_handle_data, &this_message, sizeof (this_message));
      else if (this_message.is_new == TRUE)
	new_messages--;
    }

  /* unlock */
  (void) lock_reg (file_handle_data, F_SETLK, F_UNLCK, 0, SEEK_SET, 0);
  close (file_handle_data);
  if (close (file_handle_temp))
	error (EXIT_FAILURE, errno, "close: %s", fname);

  free (fname);
  fname = NULL;

  total_number_of_messages = count_total_number_of_messages ();
  current_showed_message--;
  show_next_message (NULL);

  if (!total_number_of_messages && user_rc.auto_iconify)
    gtk_window_iconify (GTK_WINDOW (window));
}



/* -- delete ALL messages in the data file -- */
void
delete_all_messages (void)
{
  int file_handle_data;

  if (!total_number_of_messages)
    return;
  if (alert_dialog (_("Confirm for deletion"), _("\nDo you really want\ndelete ALL messages?"), _("Yes"), _("No"), 2) == 2)
    return;
  file_handle_data = open (DATA_FILE, O_RDWR | O_TRUNC, DATA_PERM);
  close (file_handle_data);
  total_number_of_messages = count_total_number_of_messages ();
  new_messages = 0;
  show_no_message ();
}



/* -- return the numb of new message in data file -- */
int
are_there_new_messages_in_data_file (void)
{
  struct linpopup_message this_message;
  int file_handle;
  int compt;

  first_new_message = 0;
  new_messages = 0;

  if (access (DATA_FILE, F_OK))
    return 0;

  if ((file_handle = open (DATA_FILE, O_RDONLY, DATA_PERM)) == -1)
    strerror (errno);
  /* lock it for reading .. */
  (void) lock_reg (file_handle, F_SETLKW, F_RDLCK, 0, SEEK_SET, 0);

  for (compt = 1; compt <= total_number_of_messages; compt++)
    {
      read (file_handle, &this_message, sizeof (this_message));
      if (this_message.is_new == TRUE)
	{
	  new_messages++;
	  if (first_new_message == 0)
	    first_new_message = compt;
	}
    }

  /* unlock */
  (void) lock_reg (file_handle, F_SETLK, F_UNLCK, 0, SEEK_SET, 0);
  close (file_handle);
  return new_messages;
}



/* -- New message regular check  -- */
gint check_for_new_message (gpointer data)
{
  if (processing_new_message)
    return TRUE;

  total_number_of_messages = count_total_number_of_messages ();
  if (!are_there_new_messages_in_data_file ())
    return TRUE;

  if (user_rc.beep_new_message)
    gdk_beep ();

  processing_new_message = TRUE;
  show_message (first_new_message);
  processing_new_message = FALSE;

  return TRUE;
}





/* -- HERE IS THE FUNCTION INVOKED WHEN LAUNCHED BY SAMBA -- */
void
trap_message (int argc, char *argv[])
{
  FILE *in;
  int file_handle;
  int i;
  struct stat stat_buf;
  struct linpopup_message this_message;

  vital_check_for_read_access (argv[3]);
  memset (&this_message, 0, sizeof (this_message));

  stat (argv[3], &stat_buf);
  this_message.time = stat_buf.st_atime;
  this_message.is_new = TRUE;
  g_strlcpy (this_message.sender, argv[1], sizeof (this_message.sender));
  g_strlcpy (this_message.host, argv[2], sizeof (this_message.host));
  if (argc > 4)
    g_strlcpy (this_message.to, argv[4], sizeof (this_message.to));

  in = fopen (argv[3], "r");
  i = fread (this_message.text, 1, sizeof (this_message.text), in);
  fclose (in);
  this_message.text[i] = '\0';


  if (access (DATA_FILE, F_OK))
    file_handle = open (DATA_FILE, O_WRONLY | O_CREAT, DATA_PERM);
  else
    file_handle = open (DATA_FILE, O_WRONLY | O_APPEND, DATA_PERM);

  /* lock it for writing .. */
  (void) lock_reg (file_handle, F_SETLKW, F_WRLCK, 0, SEEK_SET, 0);

  write (file_handle, &this_message, sizeof (this_message));

  /* unlock */
  (void) lock_reg (file_handle, F_SETLK, F_UNLCK, 0, SEEK_SET, 0);

  close (file_handle);

  exit (0);
}

/* EOF */
