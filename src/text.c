/* LinPopUp - A Linux enhanced port of Winpopup, running over Samba.
 * Copyright (c)1998-2000 Jean-Marc Jacquet <jm@littleigloo.org>  
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
#include <gdk/gdkkeysyms.h>
#include <gtk/gtkstyle.h>



void
clear_text (GtkWidget * this_text)
{
  gtk_text_buffer_set_text (gtk_text_view_get_buffer (GTK_TEXT_VIEW (this_text)), "", -1 );
}



void
realize_text (GtkWidget * this_text, gpointer data)
{
  gtk_text_buffer_set_text (gtk_text_view_get_buffer (GTK_TEXT_VIEW (this_text)), data, -1 );
}



void
text_copy_to_clipboard (GtkWidget * widget, GtkWidget * this_text)
{
  gtk_text_buffer_copy_clipboard (gtk_text_view_get_buffer (GTK_TEXT_VIEW (this_text)),
      gtk_clipboard_get(GDK_SELECTION_CLIPBOARD));
}


void
text_cut_to_clipboard (GtkWidget * widget, GtkWidget * this_text)
{
  gtk_text_buffer_cut_clipboard (gtk_text_view_get_buffer (GTK_TEXT_VIEW (this_text)),
      gtk_clipboard_get(GDK_SELECTION_CLIPBOARD), TRUE);
}


void
text_paste_from_clipboard (GtkWidget * widget, GtkWidget * this_text)
{
  gtk_text_buffer_paste_clipboard (gtk_text_view_get_buffer (GTK_TEXT_VIEW (this_text)),
      gtk_clipboard_get(GDK_SELECTION_CLIPBOARD), NULL, TRUE);
}


void
text_select_all (GtkWidget * widget, GtkWidget * this_text)
{
  GtkTextIter start, end;
  GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (this_text));
  gtk_text_buffer_get_bounds (buffer, &start, &end);
  gtk_text_buffer_move_mark_by_name (buffer, "insert", &start);
  gtk_text_buffer_move_mark_by_name (buffer, "selection_bound", &end);
}





void
create_text_area (GtkWidget * this_text, GtkWidget * vbox)
{
  GtkWidget *scrolledwindow;
  static GdkColor text_bg = { 0, 0xffec, 0xffec, 0xffec };
  GtkStyle *style;

  scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow);
  gtk_box_pack_start (GTK_BOX (vbox), scrolledwindow, TRUE, TRUE, 0);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_SHADOW_IN);

  gtk_widget_show (this_text);
  gtk_container_add (GTK_CONTAINER (scrolledwindow), this_text);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (this_text), GTK_WRAP_WORD);

  style = gtk_style_new ();
  style->base[GTK_STATE_NORMAL] = text_bg;
  gtk_widget_set_style (GTK_WIDGET (this_text), style);
}

/* EOF */
