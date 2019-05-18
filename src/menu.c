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
#include <gdk/gdkkeysyms.h>
#include <string.h>



static void
menu_separator (GtkWidget * menu)
{
  GtkWidget *menu_item = gtk_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
  gtk_widget_set_sensitive (menu_item, FALSE);
  gtk_widget_show (menu_item);
}

static void
menu_tearoff (GtkWidget * menu)
{
  GtkWidget *menu_item = gtk_tearoff_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
  gtk_widget_show (menu_item);
}

static GtkWidget *
create_menu_item (GtkWidget * menu, gchar * label, gint sensitive_flag, GtkSignalFunc this_func, gpointer this_func_data)
{
  GtkWidget *menu_item = gtk_menu_item_new_with_label (label);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
  gtk_widget_set_sensitive (menu_item, sensitive_flag);
  gtk_widget_show (menu_item);
  g_signal_connect (GTK_OBJECT (menu_item), "activate", GTK_SIGNAL_FUNC (this_func), this_func_data);
  return menu_item;
}






/* MAIN MENU */



void
create_main_menu (GtkWidget * vbox)
{
  GtkWidget *menu;
  GtkWidget *menu_bar;
  GtkWidget *file_menu;
  GtkWidget *menu_item;
  GtkAccelGroup *accel_ = gtk_accel_group_new ();
  if (GTK_WIDGET_TOPLEVEL (gtk_widget_get_toplevel (vbox)))
     gtk_window_add_accel_group (GTK_WINDOW (gtk_widget_get_toplevel (vbox)), accel_);



  menu_bar = gtk_menu_bar_new ();
  gtk_box_pack_start (GTK_BOX (vbox), menu_bar, FALSE, TRUE, 0);
  gtk_widget_show (menu_bar);



  /* MESSAGE MENU */
  file_menu = gtk_menu_item_new_with_label (_("Message"));
  gtk_menu_shell_append (GTK_MENU_SHELL (menu_bar), file_menu);
  gtk_widget_show (file_menu);

  menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (file_menu), menu);


  menu_tearoff (menu);

  menu_item = create_menu_item (menu, _("New"), TRUE, GTK_SIGNAL_FUNC (send_message), NULL);
  ACCEL (menu_item, 'M', GDK_CONTROL_MASK);
  menu_item = create_menu_item (menu, _("Reply"), TRUE, GTK_SIGNAL_FUNC (reply_message), NULL);
  ACCEL (menu_item, 'R', GDK_CONTROL_MASK);

  menu_separator (menu);

  menu_item = create_menu_item (menu, _("Next"), TRUE, GTK_SIGNAL_FUNC (show_next_message), NULL);
  ACCEL (menu_item, 'N', GDK_CONTROL_MASK);
  menu_item = create_menu_item (menu, _("Previous"), TRUE, GTK_SIGNAL_FUNC (show_previous_message), NULL);
  ACCEL (menu_item, 'P', GDK_CONTROL_MASK);

  menu_separator (menu);

  menu_item = create_menu_item (menu, _("Save As..."), TRUE, GTK_SIGNAL_FUNC (message_save), NULL);
  ACCEL (menu_item, 'S', GDK_CONTROL_MASK);
  create_menu_item (menu, _("Save All Messages"), TRUE, GTK_SIGNAL_FUNC (message_save), GINT_TO_POINTER (TRUE));

  menu_separator (menu);

  menu_item = create_menu_item (menu, _("Delete"), TRUE, GTK_SIGNAL_FUNC (delete_showed_message), NULL);
  ACCEL (menu_item, 'D', GDK_CONTROL_MASK);
  create_menu_item (menu, _("Delete All"), TRUE, GTK_SIGNAL_FUNC (delete_all_messages), NULL);

  menu_separator (menu);

  menu_item = create_menu_item (menu, _("Quit"), TRUE, GTK_SIGNAL_FUNC (gtk_main_quit), NULL);
  ACCEL (menu_item, 'Q', GDK_CONTROL_MASK);

  gtk_widget_show (menu_item);

  menu_separator (menu);
  menu_separator (menu);




  /*  EDIT MENU  */
  file_menu = gtk_menu_item_new_with_label (_("Edit"));
  gtk_menu_shell_append (GTK_MENU_SHELL (menu_bar), file_menu);
  gtk_widget_show (file_menu);

  menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (file_menu), menu);

  menu_tearoff (menu);

  menu_item = create_menu_item (menu, _("Select All"), TRUE, GTK_SIGNAL_FUNC (text_select_all), main_text);
  ACCEL (menu_item, 'A', GDK_CONTROL_MASK);
  menu_item = create_menu_item (menu, _("Copy to Clipboard"), TRUE, GTK_SIGNAL_FUNC (text_copy_to_clipboard), main_text);
  ACCEL (menu_item, 'C', GDK_CONTROL_MASK);

  menu_separator (menu);

  menu_item = create_menu_item (menu, _("Preferences..."), TRUE, GTK_SIGNAL_FUNC (custom_notebook), NULL);

  menu_separator (menu);
  menu_separator (menu);





/* WINDOW MENU  */
  file_menu = gtk_menu_item_new_with_label (_("Window"));
  gtk_menu_shell_append (GTK_MENU_SHELL (menu_bar), file_menu);
  gtk_widget_show (file_menu);

  menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (file_menu), menu);

  menu_tearoff (menu);

  menu_item = create_menu_item (menu, _("Save Size"), TRUE, GTK_SIGNAL_FUNC (save_geom), NULL);
  ACCEL (menu_item, 'G', GDK_CONTROL_MASK);
  menu_item = create_menu_item (menu, _("Reset default Size"), TRUE, GTK_SIGNAL_FUNC (reset_geom), NULL);
  menu_separator (menu);
  menu_separator (menu);



  /* HELP MENU */
  file_menu = gtk_menu_item_new_with_label (_("Help"));
  gtk_menu_shell_append (GTK_MENU_SHELL (menu_bar), file_menu);
  gtk_widget_show (file_menu);

  menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (file_menu), menu);
  gtk_menu_item_set_right_justified (GTK_MENU_ITEM (file_menu), TRUE);

  menu_tearoff (menu);

  menu_item = create_menu_item (menu, _("About LinPopUp..."), TRUE, GTK_SIGNAL_FUNC (about_dialog), NULL);

  menu_separator (menu);

  create_menu_item (menu, _("License"), TRUE, GTK_SIGNAL_FUNC (doc_show_text_file_from_widget), GINT_TO_POINTER (DOC_COPYING));

  menu_separator (menu);

  create_menu_item (menu, _("Readme"), TRUE, GTK_SIGNAL_FUNC (doc_show_text_file_from_widget), GINT_TO_POINTER (DOC_README));
  create_menu_item (menu, _("What's New"), TRUE, GTK_SIGNAL_FUNC (doc_show_text_file_from_widget), GINT_TO_POINTER (DOC_CHANGES));
  create_menu_item (menu, _("User's Manual"), TRUE, GTK_SIGNAL_FUNC (doc_show_text_file_from_widget), GINT_TO_POINTER (DOC_MANUAL));
  create_menu_item (menu, _("LinPopUp Forum"), TRUE, GTK_SIGNAL_FUNC (open_URL_from_widget), strdup (FORUMS_URL));

  menu_separator (menu);
  create_menu_item (menu, _("Get Last Version"), TRUE, GTK_SIGNAL_FUNC (open_URL_from_widget), strdup (HOME_URL));


  menu_separator (menu);
  menu_separator (menu);
}







/* SEND MENU */


void
create_send_menu (GtkWidget * vbox)
{
  GtkWidget *menu;
  GtkWidget *menu_bar;
  GtkWidget *file_menu;
  GtkWidget *menu_item;
  GtkAccelGroup *accel_ = gtk_accel_group_new ();
  if (GTK_WIDGET_TOPLEVEL (gtk_widget_get_toplevel (vbox)))
     gtk_window_add_accel_group (GTK_WINDOW (gtk_widget_get_toplevel (vbox)), accel_);


  menu_bar = gtk_menu_bar_new ();
  gtk_box_pack_start (GTK_BOX (vbox), menu_bar, FALSE, TRUE, 0);
  gtk_widget_show (menu_bar);



  /* MESSAGE MENU */
  file_menu = gtk_menu_item_new_with_label (_("Message"));
  gtk_menu_shell_append (GTK_MENU_SHELL (menu_bar), file_menu);
  gtk_widget_show (file_menu);

  menu = gtk_menu_new ();

  gtk_menu_item_set_submenu (GTK_MENU_ITEM (file_menu), menu);

  menu_tearoff (menu);

  menu_item = create_menu_item (menu, _("Load..."), TRUE, GTK_SIGNAL_FUNC (message_load), NULL);
  ACCEL (menu_item, 'L', GDK_CONTROL_MASK);

  menu_separator (menu);

  menu_item = create_menu_item (menu, _("Send"), TRUE, GTK_SIGNAL_FUNC (send_send), NULL);
  ACCEL (menu_item, 'S', GDK_CONTROL_MASK);

  menu_separator (menu);
  menu_separator (menu);



  /* EDIT MENU  */
  file_menu = gtk_menu_item_new_with_label (_("Edit"));
  gtk_menu_shell_append (GTK_MENU_SHELL (menu_bar), file_menu);
  gtk_widget_show (file_menu);

  menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (file_menu), menu);

  menu_tearoff (menu);

  menu_item = create_menu_item (menu, _("Cut"), TRUE, GTK_SIGNAL_FUNC (text_cut_to_clipboard), send_text);
  ACCEL (menu_item, 'X', GDK_CONTROL_MASK);

  menu_item = create_menu_item (menu, _("Copy"), TRUE, GTK_SIGNAL_FUNC (text_copy_to_clipboard), send_text);
  ACCEL (menu_item, 'C', GDK_CONTROL_MASK);

  menu_item = create_menu_item (menu, _("Paste"), TRUE, GTK_SIGNAL_FUNC (text_paste_from_clipboard), send_text);
  ACCEL (menu_item, 'V', GDK_CONTROL_MASK);

  menu_separator (menu);

  menu_item = create_menu_item (menu, _("Select All"), TRUE, GTK_SIGNAL_FUNC (text_select_all), send_text);
  ACCEL (menu_item, 'A', GDK_CONTROL_MASK);

  menu_separator (menu);
  menu_separator (menu);


}

/* EOF */
