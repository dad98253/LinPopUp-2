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
#include <gdk/gdk.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>


static GtkWidget *msg_file_sel;
static GtkWidget *send_header;
static GtkWidget *send_combo;
static GList *combo_items;




static void save_send_history (void);
static void update_send_history (char *this_name);




int
send_send (GtkWidget * widget, gpointer * data)
{
  gchar *header_utf8 = NULL, *header_locale = NULL, *header_copy = NULL;
  gchar *ptr = NULL, *ptr1 = NULL, *comma = NULL;
  gchar *message_utf8 = NULL, *message_locale = NULL, *message_prefix = NULL;
  char *command_line = NULL, textbuf[2048];
  guint header_length, message_length;
  gchar IS_TRUNCATED = FALSE;
  gchar MULTI_RECIPIENTS = FALSE;
  GtkTextIter start, end;
  GtkTextBuffer *buffer;

  char *fname;
  int fd;
  FILE *output;

  ptr = (gchar *) gtk_entry_get_text (GTK_ENTRY (send_header));
  header_copy = g_strdup(ptr);
  header_locale = g_locale_from_utf8 (ptr, -1, NULL, NULL, NULL);

  // add myself to sendlist
  if (user_rc.self_send)
    { ptr = g_strconcat (header_locale, ",", user_rc.samba_nbiosname, NULL);
      g_free(header_locale);
      header_locale = ptr;
     }
  header_length = strlen (header_locale);

  if (!header_length)
    { alert_ok (_(" Error"), _("\nPlease enter a destination."), _("Ok"));
      return FALSE;
     }

  // destroy erroneous embedded single-quotes
  while ((ptr = strchr (header_locale, '\'')) != NULL)
    *ptr = '-';

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (send_text));
  gtk_text_buffer_get_bounds (buffer, &start, &end);  
  message_length = gtk_text_buffer_get_char_count (buffer);

  // multi recipients delimited with comma
  if (strchr (header_locale, ',') != NULL)
     { ptr = g_locale_from_utf8 (_("To: "), -1, NULL, NULL, NULL);
       message_prefix = g_strconcat (" echo '[ ", ptr, header_locale, " ]\n';", NULL);
       message_length += strlen (message_prefix);
       MULTI_RECIPIENTS = TRUE;
       g_free(ptr);
      }

  // samba accept up to 1600 characters
  if (message_length > 1600)
    { IS_TRUNCATED = TRUE;
      // reserve 100 chars from 1600
      gtk_text_buffer_get_iter_at_offset (buffer, &end, 1500);
      // line alignment
      gtk_text_buffer_get_iter_at_line (buffer, &end, gtk_text_iter_get_line (&end));
      // get message part
      message_utf8 = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);

      gtk_text_buffer_move_mark_by_name (buffer, "insert", &start);
      gtk_text_buffer_move_mark_by_name (buffer, "selection_bound", &end);
      // delete this part from window
      gtk_text_buffer_delete_selection (buffer, FALSE, TRUE);
      
      // translate message to locale and add trailing banner
      ptr = g_locale_from_utf8 (message_utf8, -1, NULL, NULL, NULL);
      ptr1 = g_locale_from_utf8 (_("\n=== TO BE CONTINUED ==="), -1, NULL, NULL, NULL);
      message_locale = g_strconcat (ptr, ptr1, NULL);
      g_free(ptr);
      g_free(ptr1);
     }
  else
    { message_utf8 = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
      message_locale = g_locale_from_utf8 (message_utf8, -1, NULL, NULL, NULL);
     }

  // begin sending to recipients
  ptr = header_locale;
  while (ptr < header_locale + header_length && ptr != NULL)
    {
      if ((comma = strchr (ptr, ',')) != NULL)
	 *comma = '\0';

      if (asprintf (&fname, "%s/linpopup.XXXXXX", getenv ("TMPDIR") ?: P_tmpdir) < 0)
	 error (EXIT_FAILURE, errno, "asprintf");

      if ((fd = mkstemp (fname)) < 0)
	 error (EXIT_FAILURE, errno, "mkstemp: %s", fname);

      if (write (fd, message_locale, strlen (message_locale)) < 0)
	 error (EXIT_FAILURE, errno, "write: %s", fname);
      
      asprintf (&command_line, "(%s cat %s ) | smbclient -M \"%s\" -U \"%s\" -n \"%s\" %s 2>&1",
	    (MULTI_RECIPIENTS ? message_prefix : ""), fname, ptr, user_rc.samba_username,
	    user_rc.samba_nbiosname, (strlen(user_rc.samba_misc) ? user_rc.samba_misc : ""));

      output = popen (command_line, "r");
      textbuf[fread (textbuf, 1, sizeof (textbuf), output)] = '\0';
      pclose (output);
      free(command_line);

      if (unlink (fname) < 0)
	 error (EXIT_FAILURE, errno, "unlink: %s", fname);

      if (close (fd))
	 error (EXIT_FAILURE, errno, "close: %s", fname);

      free (fname);
      fname = NULL;

      /* -- sommaire investigation of the output -- */
      if (strstr (textbuf, "Control-D\nsent") && strstr (textbuf, "bytes"))
	{
	  if (!MULTI_RECIPIENTS && user_rc.confirm)
	    alert_ok ("LinPopUp", _("\n\nMessage succesfully sent."), _("Ok"));
	}
      else if (strstr (textbuf, "sh: smbclient: "))
	{
	  /*
	   * don't check for "command not found", as that message can be
	   * different if another LANGuage is used than C / english
	   */
	  alert_ok (_("Error"), _("\n`smbclient' command cannot be run;\nnot installed?"), _("Ok"));
	  return FALSE;
	}
      else if (strstr (textbuf, "nknown host"))
	{
	  header_utf8 = g_locale_to_utf8 (ptr, -1, NULL, NULL, NULL);
	  snprintf (textbuf, sizeof(textbuf), _("\n\nUnknown host : %s"), header_utf8);
	  g_free(header_utf8);
	  alert_ok (_("Error"), textbuf, _("Ok"));
	}
      else if (strstr (textbuf, "error connecting to") || (strstr (textbuf, "onnection to") && strstr (textbuf, "failed")))
	{
	  header_utf8 = g_locale_to_utf8 (ptr, -1, NULL, NULL, NULL);
	  snprintf (textbuf, sizeof(textbuf), _("\n\nHost unreachable :\n %s"), header_utf8);
	  g_free(header_utf8);
	  alert_ok (_("Error"), textbuf, _("Ok"));
	}
      else if (strstr (textbuf, "ERRmsgoff"))
	{
	  header_utf8 = g_locale_to_utf8 (ptr, -1, NULL, NULL, NULL);
	  snprintf (textbuf, sizeof(textbuf), _("\n\n%s can't receive\nwinpopup messages."), header_utf8);
	  g_free(header_utf8);
	  alert_ok (_("Error"), textbuf, _("Ok"));
	}
      else if (!MULTI_RECIPIENTS)
	alert_ok_text (_("LinPopup - smbclient output"), textbuf);

      ptr = (comma != NULL) ? comma + 1 : header_locale + header_length;
    }

  update_send_history (header_copy);
  g_free(header_copy);
  save_send_history ();

  if (message_prefix != NULL)
     g_free(message_prefix);
  g_free(message_locale);
  g_free(header_locale);

  if (!IS_TRUNCATED) {
     gtk_widget_destroy (send_window);
     send_window = NULL;
  }
  return TRUE;
}



static void
send_cancel (GtkWidget * widget, gpointer * data)
{
  /* -- if a message was entered, asking for confirmation -- */
  if (gtk_text_buffer_get_char_count (gtk_text_view_get_buffer (GTK_TEXT_VIEW (send_text))))
    if (alert_dialog ("", _("\nA new message was entered.\n\nCancel anyway?"), _("Yes"), _("No"), 2) == 2)
      return;
  gtk_widget_destroy (send_window);
  send_window = NULL;
}



static void
message_load_ok (GtkWidget * widget, GtkFileSelection * file_sel)
{
  gchar *filename;

  filename = (gchar *) gtk_file_selection_get_filename (GTK_FILE_SELECTION (file_sel));
  if (filename != NULL)
    {
      FILE *in;
      char buf[1024];
      GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (send_text));
      GtkTextIter end;

      in = fopen (filename, "r");
      if (in == NULL)
	{
	  g_snprintf (buf, sizeof(buf), _("\nUnable to Open '%s'\n"), filename);
	  alert_ok (_("File Error"), buf, _("Ok"));
	}
      else
	{
	  while (fgets (buf, 1024, in))
	    { gtk_text_buffer_get_end_iter (buffer, &end);
	      gtk_text_buffer_insert (buffer, &end,
	         g_locale_to_utf8 (buf, -1, NULL, NULL, NULL), -1);
	     }
	  fclose (in);
	}
    }
  gtk_widget_destroy (GTK_WIDGET (file_sel));
  msg_file_sel = NULL;
}



static void
message_load_cancel (GtkWidget * widget, GtkFileSelection * file_sel)
{
  gtk_widget_destroy (GTK_WIDGET (file_sel));
  msg_file_sel = NULL;
}



void
message_load (GtkWidget * widget, gpointer data)
{
  if (msg_file_sel == NULL)
    {
      msg_file_sel = gtk_file_selection_new (_("Load Message text..."));
      g_signal_connect ((gpointer) msg_file_sel, "destroy", G_CALLBACK (message_load_cancel), GTK_OBJECT (msg_file_sel));
      g_signal_connect ((gpointer) GTK_FILE_SELECTION (msg_file_sel)->ok_button, "clicked", G_CALLBACK (message_load_ok), GTK_OBJECT (msg_file_sel));
      g_signal_connect ((gpointer) GTK_FILE_SELECTION (msg_file_sel)->cancel_button, "clicked", G_CALLBACK (message_load_cancel), GTK_OBJECT (msg_file_sel));
     }
  if (GTK_WIDGET_VISIBLE (msg_file_sel))
    return;
  else
    gtk_widget_show (msg_file_sel);
}



void
reply_message (void)
{
  if (current_showed_message)
    {
      is_it_a_reply = TRUE;
      send_message ();
    }
}






static void
update_send_history (char *this_name)
{
  char *ptr;
  GList *list = combo_items;

  while (list)
    {
      ptr = list->data;
      if (!strncmp (ptr, this_name, 100)) /* max sender and host length */
	{
	  gtk_entry_set_text (GTK_ENTRY (send_header), this_name);
	  return;
	}
      list = list->next;
    }

  if (combo_items == NULL)
    combo_items = g_list_append (combo_items, g_strdup(this_name));
  else
    combo_items = g_list_insert (combo_items, g_strdup(this_name), 0);
  gtk_combo_set_popdown_strings (GTK_COMBO (send_combo), combo_items);
}




static void
load_send_history (void)
{
  FILE *history_file;
  char this_path[1024];
  char this_line[256];
  char *ptr;

  if (!(ptr = getenv ("HOME")))
    fatal_error (2, "HOME");
  g_snprintf (this_path, sizeof(this_path), "%s/.linpopup.history", ptr);

  if (access (this_path, F_OK))
    return;
  if ((history_file = fopen (this_path, "r")) == NULL)
    return;

  while (fgets (this_line, sizeof (this_line), history_file) != NULL)
    {
      if (this_line[strlen (this_line) - 1] == '\n')
	this_line[strlen (this_line) - 1] = 0;

      if (strlen (this_line))
	combo_items = g_list_append (combo_items, g_strdup (this_line));
    }
  fclose (history_file);

  gtk_combo_set_popdown_strings (GTK_COMBO (send_combo), combo_items);
}





static void
save_send_history (void)
{
  FILE *history_file;
  char this_path[1024];
  GList *list = combo_items;
  char *ptr;
  int compt = MAX_SEND_HISTORY_LINES;

  if (!(ptr = getenv ("HOME")))
    fatal_error (2, "HOME");
  g_snprintf (this_path, sizeof(this_path), "%s/.linpopup.history", ptr);

  if ((history_file = fopen (this_path, "w")) == NULL)
    return;

  while (list && compt--)
    {
      ptr = list->data;
      fprintf (history_file, "%s\n", ptr);
      list = list->next;
    }

  fflush (history_file);
  fclose (history_file);
}







void
send_message (void)
{
  GtkWidget *send_vbox;
  GtkWidget *send_statusbar;
  GtkWidget *send_frame;
  GtkWidget *send_table;
  GtkWidget *send_button_send;
  GtkWidget *send_button_cancel;
  int send_context_id;
  gint x, y, w, h;

  msg_file_sel = NULL;

  if (send_window != NULL)
    {
      gtk_widget_show (send_window);
      return;
    }

  /* -- First, the window -- */
  send_window = create_dialog_window (GTK_WINDOW_TOPLEVEL, 0, "LinPopUp", &send_vbox);
  gdk_window_get_origin (window->window, &x, &y);
  gtk_window_move (GTK_WINDOW (send_window), x, y);

  gdk_drawable_get_size (window->window, &w, &h);
  gtk_window_set_default_size (GTK_WINDOW (send_window), w, h);

  gdk_window_set_decorations (send_window->window, GDK_DECOR_BORDER | GDK_DECOR_TITLE | GDK_DECOR_RESIZEH);

  send_text = gtk_text_view_new ();


  /* -- Top frame first packed to the vbox -- */
  send_frame = create_frame (send_vbox, _("Send to:"), FALSE, TRUE, GTK_SHADOW_ETCHED_IN);
  gtk_container_set_border_width (GTK_CONTAINER (send_frame), 2);


  /* -- Creating frame's widgets -- */
  send_combo = gtk_combo_new ();
  gtk_combo_set_use_arrows_always (GTK_COMBO (send_combo), TRUE);
  gtk_widget_show (send_combo);
  combo_items = NULL;
  send_header = GTK_COMBO (send_combo)->entry;
  gtk_entry_set_editable (GTK_ENTRY (send_header), 1);
  load_send_history ();


  send_button_send = gtk_button_new_with_label (_("Send"));
  g_signal_connect (GTK_OBJECT (send_button_send), "clicked", GTK_SIGNAL_FUNC (send_send), NULL);
  gtk_widget_show (send_button_send);

  send_button_cancel = gtk_button_new_with_label (_("Cancel"));
  g_signal_connect (GTK_OBJECT (send_button_cancel), "clicked", GTK_SIGNAL_FUNC (send_cancel), NULL);
  gtk_widget_show (send_button_cancel);

/*-- attaching frame's widget to a table, and packing it to the frame --*/
  send_table = gtk_table_new (2, 4, TRUE);
  gtk_table_set_col_spacings (GTK_TABLE (send_table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (send_table), 5);
  gtk_container_set_border_width (GTK_CONTAINER (send_table), 10);
  gtk_table_attach (GTK_TABLE (send_table), send_combo, 0, 3, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
  gtk_table_attach (GTK_TABLE (send_table), send_button_send, 3, 4, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
  gtk_table_attach (GTK_TABLE (send_table), send_button_cancel, 3, 4, 1, 2, GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);
  gtk_container_add (GTK_CONTAINER (send_frame), send_table);
  gtk_widget_show (send_table);
  gtk_widget_show (send_frame);

  /* -- create a menu below the top frame via create_send_menu defined in menu.c-- */
  create_send_menu (send_vbox);

  /* -- and a text area via create_text defined in text.c -- */
  create_text_area (send_text, send_vbox);
  gtk_text_view_set_editable (GTK_TEXT_VIEW (send_text), TRUE);
  gtk_text_view_set_left_margin (GTK_TEXT_VIEW (send_text), 3);
  gtk_text_view_set_right_margin (GTK_TEXT_VIEW (send_text), 3);
  gtk_widget_grab_focus (send_text);

  /* -- and finally a statusbar -- */
  send_statusbar = gtk_statusbar_new ();
  gtk_box_pack_start (GTK_BOX (send_vbox), send_statusbar, FALSE, TRUE, 0);
  gtk_widget_show (send_statusbar);
  send_context_id = gtk_statusbar_get_context_id (GTK_STATUSBAR (send_statusbar), " ");

  /* -- show the window -- */
  gtk_widget_show (send_window);

  /* -- connecting the send_cancel function to the delete event -- */
  g_signal_connect (GTK_OBJECT (send_window), "delete_event", GTK_SIGNAL_FUNC (send_cancel), NULL);


  /* -- fill the deliver adress if required -- */
  switch (user_rc.deliver)
    {
    case DHOST:
      gtk_entry_set_text (GTK_ENTRY (send_header), user_rc.deliver_adr);
      break;
    case GUESS:
      if (!strcmp (user_rc.guess_adr, "?"))
	gtk_entry_set_text (GTK_ENTRY (send_header), user_rc.deliver_adr);
      else
	gtk_entry_set_text (GTK_ENTRY (send_header), user_rc.guess_adr);
      break;
    case BLANK:
      gtk_entry_set_text (GTK_ENTRY (send_header), "");
    default:
      break;
    }


  /* -- If this is a reply, fill the destination and text field -- */
  if (is_it_a_reply)
    {
      struct linpopup_message this_message;
      char buf[2045];
      int char_pos_text, char_pos_buf;
      GdkColormap *colormap;
      GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (send_text));
      GtkTextIter start, end;
      GtkTextTag *tag;

      colormap = gtk_widget_get_colormap (send_window);
      gdk_color_alloc (colormap, &user_rc.quoted_text_col);

      read_message (current_showed_message, &this_message);
      gtk_entry_set_text (GTK_ENTRY (send_header), this_message.host);	//We answer to a host not to a user

      g_snprintf (buf, sizeof(buf), "%c %s \n%c ---\n", user_rc.quote, user_rc.quote_string, user_rc.quote);
      strexpand (buf, "HOST", this_message.host, sizeof(buf));
      strexpand (buf, "USER", this_message.sender, sizeof(buf));

      gtk_text_buffer_set_text (buffer, buf, -1);

      char_pos_buf = 0;
      buf[char_pos_buf++] = user_rc.quote;
      buf[char_pos_buf++] = 32;

      for (char_pos_text = 0; char_pos_text < strlen (this_message.text); char_pos_text++)
	{
	  buf[char_pos_buf++] = this_message.text[char_pos_text];
	  if (this_message.text[char_pos_text] == '\n')
	    {
	      buf[char_pos_buf++] = user_rc.quote;
	      buf[char_pos_buf++] = ' ';
	    }
	}

      buf[char_pos_buf++] = '\n';
      buf[char_pos_buf++] = user_rc.quote;
      buf[char_pos_buf] = '\0';

      /* -- quoted text appears in user defined color -- */
      g_strlcat (buf, " ---\n", sizeof(buf));
      gtk_text_buffer_get_end_iter (buffer, &end);
      gtk_text_buffer_insert (buffer, &end, g_locale_to_utf8 (buf, -1, NULL, NULL, NULL), -1);

      if ((tag = gtk_text_tag_table_lookup (gtk_text_buffer_get_tag_table (buffer), "quote")) == NULL)
         tag = gtk_text_buffer_create_tag (buffer, "quote", "foreground-gdk", &user_rc.quoted_text_col, NULL);
      gtk_text_buffer_get_bounds (buffer, &start, &end);
      gtk_text_buffer_apply_tag (buffer, tag, &start, &end);

      gtk_text_view_set_editable (GTK_TEXT_VIEW (send_text), TRUE);
      gtk_widget_grab_focus (send_text);

      is_it_a_reply = FALSE;
    }
}

/*  EOF */
