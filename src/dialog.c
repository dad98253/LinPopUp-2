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
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <pixmaps/icon_256.xpm>
#include <pixmaps/icon_16.xpm>




static GtkWidget *
create_alert_label (GtkWidget * box, char *this_text, gfloat xalignement, gfloat yalignement)
{
  GtkWidget *alert_label = gtk_label_new (this_text);
  gtk_misc_set_alignment (GTK_MISC (alert_label), xalignement, yalignement);
  gtk_box_pack_start (GTK_BOX (box), alert_label, FALSE, FALSE, 0);
  gtk_widget_show (alert_label);
  return alert_label;
}


GtkWidget *
create_dialog_window (GtkWindowType win_TYPE, GtkWindowPosition win_POS, char *win_title, GtkWidget ** this_box)
{
  GtkWidget *alert_window;
  GtkWidget *alert_frame;

  alert_window = gtk_window_new (win_TYPE);
  if (win_TYPE != GTK_WINDOW_POPUP)
    gtk_window_set_title (GTK_WINDOW (alert_window), win_title);
  if (win_POS)
    gtk_window_set_position (GTK_WINDOW (alert_window), win_POS);
  gtk_widget_realize (alert_window);
  gdk_window_set_decorations (alert_window->window, GDK_DECOR_BORDER | GDK_DECOR_TITLE);

  gtk_window_set_transient_for (GTK_WINDOW (alert_window), GTK_WINDOW (window));

  alert_frame = gtk_frame_new (NULL);
  gtk_widget_show (alert_frame);
  gtk_container_add (GTK_CONTAINER (alert_window), alert_frame);
  gtk_frame_set_shadow_type (GTK_FRAME (alert_frame), GTK_SHADOW_OUT);

  if (this_box != NULL)
    *this_box = v_box (alert_frame, FALSE, 0, 0);

  return alert_window;
}


static GtkWidget *
make_linpopup_icon (GtkWidget * this_window)
{
  GdkPixmap *pixmap;
  GdkBitmap *mask;
  GtkWidget *icon;

  if (user_rc.hcolors)
    {
      pixmap = gdk_pixmap_create_from_xpm_d (this_window->window, &mask, &this_window->style->bg[GTK_STATE_NORMAL], (gchar **) icon_256);
    }
  else
    {
      pixmap = gdk_pixmap_create_from_xpm_d (this_window->window, &mask, &this_window->style->bg[GTK_STATE_NORMAL], (gchar **) icon_16);
    }
  icon = gtk_image_new_from_pixmap (pixmap, mask);
  return icon;
}

GtkWidget *
program_icon (GtkWidget * this_box)
{
  GtkWidget *pixmap = make_linpopup_icon (window);
  gtk_box_pack_start (GTK_BOX (this_box), pixmap, FALSE, FALSE, 0);
  gtk_widget_show (pixmap);
  return pixmap;
}




/* MODAL ALERT OK_CANCEL */


static void
alert_first (GtkWidget * widget, int *data)
{
  *data = 1;
}

static void
alert_second (GtkWidget * widget, int *data)
{
  *data = 2;
}

int
alert_dialog (gchar * title, gchar * alert_message, gchar * first_button, gchar * second_button, int default_button)
{
  GtkWidget *alert_window;
  GtkWidget *alert_vbox;
  GtkWidget *alert_vbox1;
  GtkWidget *alert_hbox;
  GtkWidget *alert_button;
  int selected = default_button;

  alert_window = create_dialog_window (GTK_WINDOW_TOPLEVEL, GTK_WIN_POS_MOUSE, title, &alert_vbox);


  alert_hbox = h_box (alert_vbox, FALSE, 10, 10);

  program_icon (alert_hbox);

  alert_vbox1 = gtk_vbox_new (FALSE, 5);
  gtk_container_add (GTK_CONTAINER (alert_hbox), alert_vbox1);
  gtk_container_set_border_width (GTK_CONTAINER (alert_vbox1), 10);
  gtk_widget_show (alert_vbox1);

  create_alert_label (alert_vbox1, alert_message, 0, 0);

  hseparator (alert_vbox);

  alert_hbox = hbutton_box (alert_vbox);

  alert_button = box_button (alert_hbox, first_button);

  g_signal_connect (GTK_OBJECT (alert_button), "clicked", GTK_SIGNAL_FUNC (alert_first), &selected);
  g_signal_connect_swapped (GTK_OBJECT (alert_button), "clicked", GTK_SIGNAL_FUNC (gtk_main_quit), NULL);


  if (default_button == 1)
    {
      GTK_WIDGET_SET_FLAGS (alert_button, GTK_CAN_DEFAULT);
      gtk_widget_grab_default (alert_button);
    }

  alert_button = box_button (alert_hbox, second_button);
  g_signal_connect (GTK_OBJECT (alert_button), "clicked", GTK_SIGNAL_FUNC (alert_second), &selected);
  g_signal_connect_swapped (GTK_OBJECT (alert_button), "clicked", GTK_SIGNAL_FUNC (gtk_main_quit), NULL);


  if (default_button == 2)
    {
      GTK_WIDGET_SET_FLAGS (alert_button, GTK_CAN_DEFAULT);
      gtk_widget_grab_default (alert_button);
    }

  gtk_widget_show (alert_window);

  gtk_grab_add (alert_window);

  gtk_main ();

  gtk_grab_remove (alert_window);
  gtk_widget_destroy (alert_window);
  return selected;
}




/* ALERT OK */

void
alert_ok (gchar * title, gchar * alert_message, gchar * alert_button_label)
{
  GtkWidget *alert_window;
  GtkWidget *alert_vbox;
  GtkWidget *alert_hbox;
  GtkWidget *alert_button;

  alert_window = create_dialog_window (GTK_WINDOW_TOPLEVEL, GTK_WIN_POS_MOUSE, title, &alert_vbox);
  if (strlen (alert_message) < 40)
    gtk_widget_set_usize (GTK_WIDGET (alert_window), 250, 150);

  alert_hbox = h_box (alert_vbox, FALSE, 10, 10);

  program_icon (alert_hbox);

  create_alert_label (alert_hbox, alert_message, 0, 0);

  hseparator (alert_vbox);

  alert_hbox = hbutton_box (alert_vbox);

  alert_button = box_button (alert_hbox, alert_button_label);
  g_signal_connect_swapped (GTK_OBJECT (alert_button), "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy), GTK_OBJECT (alert_window));

  GTK_WIDGET_SET_FLAGS (alert_button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default (alert_button);

  gtk_widget_show (alert_window);
}



/* ABOUT DIALOG */

void
about_dialog (void)
{
  GtkWidget *alert_window;
  GtkWidget *alert_vbox;
  GtkWidget *alert_vbox1;
  GtkWidget *alert_hbox;
  GtkWidget *alert_button;
  GtkWidget *igloo_pixmap;
  GdkPixmap *pixmap = NULL;
  GdkBitmap *mask;

  char buf[128];
  char author_buf[128];
  char pixmap_path[1024];
  char *ptr;

  alert_window = create_dialog_window (GTK_WINDOW_TOPLEVEL, GTK_WIN_POS_MOUSE, _("About LinPopUp"), &alert_vbox);
  alert_hbox = h_box (alert_vbox, FALSE, 10, 10);

  gtk_misc_set_padding (GTK_MISC (program_icon (alert_hbox)), 0.0, 0.0);
  alert_vbox1 = v_box (alert_hbox, FALSE, 0, 0);

  a_little_version_infos (buf, sizeof(buf));
  ptr = strchr (buf, '-');
  *(ptr++) = 0;
  create_alert_label (alert_vbox1, buf, 0, 0);
  sprintf (author_buf, _("Author : Jean-Marc Jacquet"));
  create_alert_label (alert_vbox1, author_buf, 0, 0);
  create_alert_label (alert_vbox1, ++ptr, 0, 0);


  hseparator (alert_vbox);

  alert_hbox = h_box (alert_vbox, FALSE, 0, 0);

  g_snprintf (pixmap_path, sizeof(pixmap_path), "%s/little_igloo.xpm", PIXMAPS_PATH);

  if (access (pixmap_path, F_OK) == 0)
    {
      pixmap = gdk_pixmap_create_from_xpm (alert_window->window, &mask, &alert_window->style->bg[GTK_STATE_NORMAL], pixmap_path);
      igloo_pixmap = gtk_image_new_from_pixmap (pixmap, mask);
      alert_button = gtk_button_new ();

      gtk_container_add (GTK_CONTAINER (alert_button), igloo_pixmap);
      gtk_widget_show (igloo_pixmap);
    }
  else
    alert_button = gtk_button_new_with_label (HOME_URL);

  gtk_button_set_relief (GTK_BUTTON (alert_button), GTK_RELIEF_NONE);
  gtk_box_pack_start (GTK_BOX (alert_hbox), alert_button, TRUE, TRUE, 0);
  gtk_widget_show (alert_button);
  g_signal_connect (GTK_OBJECT (alert_button), "clicked", GTK_SIGNAL_FUNC (open_URL_from_widget), strdup (HOME_URL));

  hseparator (alert_vbox);

  alert_hbox = hbutton_box (alert_vbox);

  alert_button = box_button (alert_hbox, _("Close"));
  g_signal_connect_swapped (GTK_OBJECT (alert_button), "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy), GTK_OBJECT (alert_window));

  GTK_WIDGET_SET_FLAGS (alert_button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default (alert_button);

  gtk_widget_show (alert_window);

  if (pixmap != NULL)
    {
      gdk_pixmap_unref (pixmap);
      gdk_pixmap_unref (mask);
    }
}





/* ALERT OK TEXT */


void
alert_ok_text (gchar * tittle, gchar * message)
{
  GtkWidget *alert_window;
  GtkWidget *alert_vbox;
  GtkWidget *alert_hbox;
  GtkWidget *alert_text;
  GtkWidget *alert_button;

  alert_window = create_dialog_window (GTK_WINDOW_TOPLEVEL, GTK_WIN_POS_MOUSE, tittle, &alert_vbox);
  gtk_widget_set_usize (GTK_WIDGET (alert_window), 350, -1);
  gtk_container_set_border_width (GTK_CONTAINER (alert_vbox), 5);

  alert_text = gtk_text_view_new ();
  create_text_area (alert_text, alert_vbox);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (alert_text), GTK_WRAP_WORD);

  hseparator (alert_vbox);

  alert_hbox = h_box (alert_vbox, FALSE, 10, 5);
  gtk_box_set_child_packing (GTK_BOX (alert_vbox), alert_hbox, FALSE, FALSE, 0, GTK_PACK_START);

  program_icon (alert_hbox);

  alert_hbox = hbutton_box (alert_hbox);

  alert_button = box_button (alert_hbox, _("Ok"));
  g_signal_connect_swapped (GTK_OBJECT (alert_button), "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy), GTK_OBJECT (alert_window));

  GTK_WIDGET_SET_FLAGS (alert_button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default (alert_button);

  gtk_widget_show (alert_window);

  realize_text (alert_text, message);

  gtk_text_view_set_editable (GTK_TEXT_VIEW (alert_text), FALSE);
  gdk_window_raise (alert_window->window);
}



/* RESIZABLE TEXT DIALOG */


void
resizable_text_dialog (gchar * title, char *this_text, int width, int height)
{
  GtkWidget *alert_window;
  GtkWidget *alert_vbox;
  GtkWidget *alert_vbox1;
  GtkWidget *alert_hbox;
  GtkWidget *alert_button;
  GtkWidget *alert_text;
  GtkStyle *style;
  const GdkColor LCYAN = { 0, 0xbb00, 0xccee, 0xffff };

  alert_window = create_dialog_window (GTK_WINDOW_TOPLEVEL, GTK_WIN_POS_CENTER, title, &alert_vbox);
  gdk_window_set_decorations (alert_window->window, GDK_DECOR_BORDER | GDK_DECOR_TITLE | GDK_DECOR_RESIZEH);

  alert_vbox1 = v_box (alert_vbox, FALSE, 5, 10);

  alert_text = gtk_text_view_new ();
  create_text_area (alert_text, alert_vbox1);
  gtk_text_view_set_editable (GTK_TEXT_VIEW(alert_text), FALSE);
  gtk_widget_set_usize (alert_text, width, height);

  style = gtk_style_new ();
  style->base[GTK_STATE_NORMAL] = LCYAN;
  gtk_widget_set_style (alert_text, style);
  gtk_widget_show (alert_text);


  gtk_widget_show (alert_vbox1);

  hseparator (alert_vbox);

  alert_hbox = h_box (alert_vbox, FALSE, 0, 5);
  gtk_box_set_child_packing (GTK_BOX (alert_vbox), alert_hbox, FALSE, FALSE, 0, GTK_PACK_START);

  program_icon (alert_hbox);
  alert_hbox = hbutton_box (alert_hbox);

  alert_button = box_button (alert_hbox, _("Close"));
  g_signal_connect_swapped (GTK_OBJECT (alert_button), "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy), GTK_OBJECT (alert_window));
  GTK_WIDGET_SET_FLAGS (alert_button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default (alert_button);

  gtk_widget_show (alert_window);

  realize_text (alert_text, this_text);
}
