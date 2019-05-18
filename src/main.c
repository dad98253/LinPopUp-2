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
#include <gdk/gdkkeysyms.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>
#include <unistd.h>
#include "pixmaps/icon_256.xpm"
#include "pixmaps/icon_16.xpm"


void
main_quit (GtkWidget * widget, int *data)
{
  gtk_timeout_remove (check_for_new_message_timeout);
  gtk_main_quit ();
}

gboolean
on_window_state (GtkWidget *widget, GdkEventWindowState *event, gpointer not_used)
{
  if(user_rc.minimize_to_tray)
  {
    if(event->new_window_state == GDK_WINDOW_STATE_ICONIFIED)
      {
        gtk_widget_show_all( GTK_WIDGET (tray_icon));
        gtk_window_set_skip_taskbar_hint (GTK_WINDOW (widget), TRUE);
        gtk_window_set_skip_pager_hint (GTK_WINDOW (widget), TRUE);
      }
    else
      {
        gtk_widget_hide_all( GTK_WIDGET (tray_icon));
        gtk_window_set_skip_taskbar_hint (GTK_WINDOW (widget), FALSE);
        gtk_window_set_skip_pager_hint (GTK_WINDOW (widget), FALSE);
      }
  }
  return FALSE;
}


int
main (int argc, char *argv[])
{
  GtkWidget *vbox, *vbox1;
  GtkWidget *table;
  GtkWidget *frame;
  GdkPixmap *icon_pixmap;
  GdkBitmap *icon_mask;


  if (argc > 3)
    trap_message (argc, argv);


  samba_installed = samba_info (samba_version, sizeof(samba_version));


  if (argc > 1)
    {
      if (strstr (argv[1], "info"))
	{
	  char buf[1024];
	  static struct utsname uname_buf;
	  uname (&uname_buf);
	  printf ("\n%s", many_version_infos (buf, sizeof(buf)));
	  printf ("\nMessages are kept in %s", DATA_FILE);
	  printf ("\nGTK+   : %i.%i.%u", GTK_MAJOR_VERSION, GTK_MINOR_VERSION, GTK_MICRO_VERSION);
	  printf ("\nKernel : %s", uname_buf.release);
	  printf ("\nHost   : %s", uname_buf.nodename);
	  printf ("\nCPU    : %s", uname_buf.machine);
	  if (samba_installed)
	    printf ("\nSamba  : %s", samba_version + 1);

	  printf ("\n\n");
	  exit (0);
	}

      if (strstr (argv[1], "help"))
	{
	  char buf[1024];
	  printf ("\n%s", many_version_infos (buf, sizeof(buf)));
	  printf ("\nTo obtain help, type :\n %% man linpopup\n Or launch LinPopUp in a Xsession and choose the help menu.\n\n");
	  exit (0);
	}
    }

#ifdef ENABLE_NLS
  bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);
#endif

  /* GTK INIT */
  gtk_set_locale ();
  gtk_init (&argc, &argv);


  /* PARSE gtkrc Theme File - based on code contributed by  Brent Verner */
  {
    char buf[1024];
    char *ptr = getenv ("HOME");
    g_snprintf (buf, sizeof(buf), "%s%s", SHARE_PATH, "/gtkrc");
    if (access (buf, R_OK) && ptr != NULL)
      g_snprintf (buf, sizeof(buf), "%s%s", ptr, "/.gtkrc");
    gtk_rc_parse (buf);
  }


  /* USER PREFERENCES INIT */
  init_rc ();



  /* -- here comes the main window -- */
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  gtk_window_set_default_size (GTK_WINDOW (window), user_rc.win_w ? user_rc.win_w : MAIN_WIDTH, user_rc.win_h ? user_rc.win_h : MAIN_HEIGHT);

  if (user_rc.win_x && user_rc.win_y)
     gtk_window_move (GTK_WINDOW (window), user_rc.win_x, user_rc.win_y);

  gtk_window_set_title (GTK_WINDOW (window), "LinPopUp");
  gtk_container_set_border_width (GTK_CONTAINER (window), 0);
  gtk_widget_realize (window);

  g_signal_connect (GTK_OBJECT (window), "destroy", (GtkSignalFunc) main_quit, NULL);
  g_signal_connect (GTK_OBJECT (window), "window-state-event", GTK_SIGNAL_FUNC (on_window_state), NULL);

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (window), vbox);
  gtk_widget_show (vbox);

  main_text = gtk_text_view_new ();

  create_main_menu (vbox);

  main_toolbar = create_toolbar (window, vbox);

  frame = create_frame (vbox, NULL, FALSE, FALSE, GTK_SHADOW_OUT);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 0);

  vbox1 = v_box (frame, FALSE, 5, 0);

  main_label_from = gtk_label_new (" ");
  gtk_box_pack_start (GTK_BOX (vbox1), main_label_from, TRUE, TRUE, 10);
  gtk_widget_show (main_label_from);

  create_text_area (main_text, vbox);
  gtk_text_view_set_editable (GTK_TEXT_VIEW(main_text), FALSE);
  gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (main_text), FALSE);
  gtk_text_view_set_left_margin (GTK_TEXT_VIEW (main_text), 3);
  gtk_text_view_set_right_margin (GTK_TEXT_VIEW (main_text), 3);


  /* -- create statusbar (4 cols  in a table) -- */
  {
    short int f;
    for (f = 0; f < 4; f++)
      {
	main_statusbar[f] = gtk_statusbar_new ();
	gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR(main_statusbar[f]), FALSE);
	gtk_widget_show (main_statusbar[f]);
	main_context_id[f] = gtk_statusbar_get_context_id (GTK_STATUSBAR (main_statusbar[f]), " ");
	main_message_id[f] = gtk_statusbar_push (GTK_STATUSBAR (main_statusbar[f]), main_context_id[f], " ");
      }
  }

  table = gtk_table_new (1, 6, TRUE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 1);

  gtk_table_attach (GTK_TABLE (table), main_statusbar[0], 0, 2, 0, 1, GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);
  gtk_table_attach (GTK_TABLE (table), main_statusbar[1], 2, 3, 0, 1, GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);
  gtk_table_attach (GTK_TABLE (table), main_statusbar[2], 3, 5, 0, 1, GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);
  gtk_table_attach (GTK_TABLE (table), main_statusbar[3], 5, 6, 0, 1, GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);

  gtk_box_pack_start (GTK_BOX (vbox), table, FALSE, TRUE, 1);
  gtk_widget_show (table);



  /* -- set icon pixmap -- */
  if (user_rc.hcolors)
    icon_pixmap = gdk_pixmap_create_from_xpm_d (window->window, &icon_mask, &window->style->bg[GTK_STATE_NORMAL], icon_256);
  else
    icon_pixmap = gdk_pixmap_create_from_xpm_d (window->window, &icon_mask, &window->style->bg[GTK_STATE_NORMAL], icon_16);
  gdk_window_set_icon (window->window, NULL, icon_pixmap, icon_mask);


  /* -- init tray icon -- */
  tray_icon_init();


  /* -- show the main window -- */
  gtk_widget_show (window);
  PROCESS_EVENTS;




  /* -- initialisation -- */
  send_window = NULL;
  processing_new_message = FALSE;
  is_it_a_reply = FALSE;

  total_number_of_messages = count_total_number_of_messages ();
  if ((new_messages = are_there_new_messages_in_data_file ()))
    show_message (first_new_message);
  else if (total_number_of_messages)
    show_message (total_number_of_messages);
  else
    show_no_message ();


  if (strstr (samba_version, " 1.") != NULL)
    if (!user_rc.samba_warn)
      {
	alert_ok_text ("LinPopUp", _("\nWARNING:\n\nDue to a security hole found in Samba message command you may install Samba version >= 2.0 Beta 2 when using LinPopUp."));
	PROCESS_EVENTS;
	user_rc.samba_warn = TRUE;
	save_rc ();
      }



  /* -- minimise the window if required -- */
  if ((argc > 1) && (strstr (argv[1], "min")) && (!new_messages))
    {
      gtk_window_iconify (GTK_WINDOW (window));
    }


  /* -- start the check_for_new_message timeout -- */
  if (user_rc.message_check_interval)
    check_for_new_message_timeout = gtk_timeout_add (user_rc.message_check_interval, check_for_new_message, NULL);


  /* -- main loop -- */
  gtk_main ();

  return 0;
}

/* EOF */
