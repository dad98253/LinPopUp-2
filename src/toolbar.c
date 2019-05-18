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

#include "pixmaps/new_16.xpm"
#include "pixmaps/reply_16.xpm"
#include "pixmaps/del_16.xpm"
#include "pixmaps/left_16.xpm"
#include "pixmaps/right_16.xpm"
#include "pixmaps/min_16.xpm"
#include "pixmaps/new_256.xpm"
#include "pixmaps/reply_256.xpm"
#include "pixmaps/del_256.xpm"
#include "pixmaps/left_256.xpm"
#include "pixmaps/right_256.xpm"
#include "pixmaps/min_256.xpm"



GtkWidget *
new_pixmap (GtkWidget * widget, char **data)
{
  GdkPixmap *gdkpixmap;
  GdkBitmap *mask;
  GtkWidget *pixmap;

  gdkpixmap = gdk_pixmap_create_from_xpm_d (widget->window, &mask, &widget->style->bg[GTK_STATE_NORMAL], data);
  pixmap = gtk_image_new_from_pixmap (gdkpixmap, mask);

  gdk_pixmap_unref (gdkpixmap);
  gdk_pixmap_unref (mask);

  return pixmap;
}


void
iconify_window ()
{
  gtk_window_iconify (GTK_WINDOW (window));
}


GtkWidget *
create_toolbar (GtkWidget * window, GtkWidget * vbox)
{
  GtkWidget *toolbar, *toolwidget;
  static GtkWidget *icon_new, *icon_reply, *icon_del, *icon_left, *icon_right, *icon_min;

  GtkAccelGroup *accel_ = gtk_accel_group_new ();
  if (GTK_WIDGET_TOPLEVEL (gtk_widget_get_toplevel (vbox)))
    gtk_window_add_accel_group (GTK_WINDOW (gtk_widget_get_toplevel (vbox)), accel_);

  toolbar = gtk_toolbar_new ();
  gtk_toolbar_set_orientation (GTK_TOOLBAR (toolbar), GTK_ORIENTATION_HORIZONTAL);
  gtk_toolbar_set_style (GTK_TOOLBAR (toolbar), user_rc.toolbar_style);
  gtk_box_pack_start (GTK_BOX (vbox), toolbar, FALSE, TRUE, 0);
  gtk_widget_show (toolbar);


  if (user_rc.hcolors)
    {
      icon_new = new_pixmap (window, new_256);
      icon_reply = new_pixmap (window, reply_256);
      icon_del = new_pixmap (window, del_256);
      icon_left = new_pixmap (window, left_256);
      icon_right = new_pixmap (window, right_256);
      icon_min = new_pixmap (window, min_256);
    }
  else
    {
      icon_new = new_pixmap (window, new_16);
      icon_reply = new_pixmap (window, reply_16);
      icon_del = new_pixmap (window, del_16);
      icon_left = new_pixmap (window, left_16);
      icon_right = new_pixmap (window, right_16);
      icon_min = new_pixmap (window, min_16);
    }

  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), _("New"), _("Send a New Message"), "", icon_new, GTK_SIGNAL_FUNC (send_message), NULL);
  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), _("Reply"), _("Reply to this Message"), "", icon_reply, GTK_SIGNAL_FUNC (reply_message), NULL);

  gtk_toolbar_append_space (GTK_TOOLBAR (toolbar));

  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), _("Delete"), _("Delete this Message"), "", icon_del, GTK_SIGNAL_FUNC (delete_showed_message), NULL);

  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), _("Prev."), _("Show Previous Message"), "", icon_left, GTK_SIGNAL_FUNC (show_previous_message), NULL);
  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), _("Next"), _("Show Next Message"), "", icon_right, GTK_SIGNAL_FUNC (show_next_message), NULL);

  gtk_toolbar_append_space (GTK_TOOLBAR (toolbar));

  toolwidget = gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), _("Iconize"), _("Iconize LinPopUp"), "", icon_min, GTK_SIGNAL_FUNC (iconify_window), NULL);
  gtk_widget_add_accelerator (toolwidget, "clicked", accel_, 'I', GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

  return toolbar;
}

/* EOF */
