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

#include <gtk/gtk.h>
#include "protos.h"



GtkWidget *
v_box (GtkWidget * parent, char homogene, gint spacing, gint border)
{
  GtkWidget *alert_box = gtk_vbox_new (homogene, spacing);
  gtk_container_add (GTK_CONTAINER (parent), alert_box);
  gtk_container_border_width (GTK_CONTAINER (alert_box), border);
  gtk_widget_show (alert_box);
  return alert_box;
}



GtkWidget *
h_box (GtkWidget * parent, char homogene, gint spacing, gint border)
{
  GtkWidget *alert_box = gtk_hbox_new (homogene, spacing);
  gtk_container_add (GTK_CONTAINER (parent), alert_box);
  gtk_container_border_width (GTK_CONTAINER (alert_box), border);
  gtk_widget_show (alert_box);
  return alert_box;
}


GtkWidget *
hbutton_box (GtkWidget * box)
{
  GtkWidget *buttonbox = gtk_hbutton_box_new ();
  gtk_widget_show (buttonbox);
  gtk_box_pack_end (GTK_BOX (box), buttonbox, FALSE, TRUE, 0);
  gtk_container_border_width (GTK_CONTAINER (buttonbox), 5);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (buttonbox), GTK_BUTTONBOX_END);
  gtk_button_box_set_spacing (GTK_BUTTON_BOX (buttonbox), 5);
  return buttonbox;
}

GtkWidget *
box_button (GtkWidget * box, char *label)
{
  GtkWidget *button = gtk_button_new_with_label (label);
  gtk_container_add (GTK_CONTAINER (box), button);
  gtk_widget_show (button);
  return button;
}

void
hseparator (GtkWidget * box)
{
  GtkWidget *alert_separator = gtk_hseparator_new ();
  gtk_box_pack_start (GTK_BOX (box), alert_separator, FALSE, TRUE, 0);
  gtk_widget_show (alert_separator);
}


GtkWidget *
create_frame (GtkWidget * parent, char *title, char h_expand, char w_expand, GtkShadowType shadow_type)
{
  GtkWidget *frame = gtk_frame_new (title);
  gtk_widget_show (frame);
  gtk_box_pack_start (GTK_BOX (parent), frame, h_expand, w_expand, 0);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), shadow_type);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);
  return frame;
}

GtkWidget *
create_label (GtkWidget * box, char *label_text, GtkJustification jtype, int x_pad, int y_pad)
{
  GtkWidget *label;
  label = gtk_label_new (label_text);
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (box), label, FALSE, TRUE, 0);
  gtk_label_set_justify (GTK_LABEL (label), jtype);
  gtk_misc_set_padding (GTK_MISC (label), x_pad, y_pad);
  return label;
}
