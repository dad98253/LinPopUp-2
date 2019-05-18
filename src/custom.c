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
#include "version.h"
#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>


static char rc_path[2048];
static GtkWidget *rc_window;
static GtkWidget *rc_spin;
static GtkWidget *rc_quote;
static GtkWidget *rc_header;
static GtkWidget *rc_dhost;
static GtkWidget *rc_samba_username;
static GtkWidget *rc_samba_nbiosname;
static GtkWidget *rc_samba_misc;

static GtkWidget *rc_check_auto_iconify;
static GtkWidget *rc_check_self_send;
static GtkWidget *rc_check_confirm;
static GtkWidget *rc_check_wrap_to_first;
static GtkWidget *rc_check_beep_new_message;
static GtkWidget *rc_check_minimize_to_tray;

static char check_flag = 0;



void
save_geom (void)
{
  gdk_drawable_get_size (window->window, &user_rc.win_w, &user_rc.win_h);
  gdk_window_get_origin (window->window, &user_rc.win_x, &user_rc.win_y);
  save_rc ();
}


void
reset_geom (void)
{
  user_rc.win_w = 0;
  user_rc.win_h = 0;
  user_rc.win_x = 0;
  user_rc.win_y = 0;

  save_rc ();

  gtk_window_set_default_size (GTK_WINDOW (window), MAIN_WIDTH, MAIN_HEIGHT);
  gtk_window_resize (GTK_WINDOW (window), MAIN_WIDTH, MAIN_HEIGHT);

}


void
save_rc (void)
{
  int file_handle;
  char errbuf[256];
  errno = 0;

  if ((file_handle = open (rc_path, O_CREAT | O_TRUNC | O_WRONLY, RC_PERM)) == -1)
    {
      g_snprintf (errbuf, sizeof(errbuf), _("%s open error: %s"), rc_path, strerror (errno));
      alert_ok (_("RC Save Error"), errbuf, _("Ok"));
      return;
    }

  if (write (file_handle, &user_rc, sizeof (user_rc)) == -1)
    {
      g_snprintf (errbuf, sizeof(errbuf), _("%s write error: %s"), rc_path, strerror (errno));
      alert_ok (_("RC Save Error"), errbuf, _("Ok"));
    }

  close (file_handle);
  return;
}


static void
create_rc (void)
{
  char *ptr;
  memset (&user_rc, 0, sizeof (user_rc));

  g_strlcpy (user_rc.version, PROG_VERSION, sizeof(user_rc.version));
  user_rc.toolbar_style = GTK_TOOLBAR_BOTH;
  user_rc.message_check_interval = 2000;
  user_rc.hcolors = TRUE;
  user_rc.quote = '|';
  g_strlcpy (user_rc.quote_string, _("USER on HOST wrote :"), sizeof(user_rc.quote_string));
  user_rc.quoted_text_col.red = 0x8000;
  user_rc.quoted_text_col.green = 0;
  user_rc.quoted_text_col.blue = 0;
  user_rc.deliver = GUESS;


  user_rc.guess_adr[0] = '?';
  user_rc.guess_adr[1] = 0;

  if ((ptr = getenv ("HOSTNAME")) == NULL)
    {
      g_strlcpy (user_rc.deliver_adr, COMPILE_HOST, sizeof(user_rc.deliver_adr));
    }
  else
    {
      g_strlcpy (user_rc.deliver_adr, ptr, sizeof(user_rc.deliver_adr));
      g_strlcpy (user_rc.samba_nbiosname, ptr, sizeof(user_rc.samba_nbiosname));
    }

  if ((ptr = strchr (user_rc.deliver_adr, '.')) != NULL)
    *ptr = 0;
  if ((ptr = strchr (user_rc.samba_nbiosname, '.')) != NULL)
    *ptr = 0;


  if ((ptr = getenv ("USER")) != NULL)
    {
      ptr = g_utf8_strup (ptr, -1);
      g_strlcpy (user_rc.samba_username, ptr, sizeof(user_rc.samba_username));
      g_free (ptr);
    }

  user_rc.samba_warn = FALSE;
  user_rc.auto_iconify = TRUE;
  user_rc.self_send = FALSE;
  user_rc.confirm = TRUE;
  user_rc.wrap_to_first = TRUE;
  user_rc.beep_new_message = TRUE;
  user_rc.minimize_to_tray = FALSE;
}



static void
read_rc (void)
{
  int file_handle;
  file_handle = open (rc_path, O_RDONLY, RC_PERM);
  read (file_handle, &user_rc, sizeof (user_rc));
  close (file_handle);
}



void
init_rc (void)
{
  char *p;
  if (!(p = getenv ("HOME")))
    fatal_error (2, "HOME");
  g_snprintf (rc_path, sizeof(rc_path), "%s/.linpopuprc", p);

  create_rc ();

  if (access (rc_path, F_OK))
    save_rc ();
  else
    read_rc ();
}




static void
toolbar_set_style (GtkWidget * widget, GtkToolbarStyle style)
{
  gtk_toolbar_set_style (GTK_TOOLBAR (main_toolbar), style);
  user_rc.toolbar_style = style;
}

static void
icons_set_colors (GtkWidget * widget, char IS_HEIGHT)
{
  user_rc.hcolors = IS_HEIGHT;
}

static void
check_set_interval (GtkWidget * widget, char MUST_CHECK)
{
  (check_flag = MUST_CHECK) ? gtk_widget_set_sensitive (rc_spin, TRUE) : gtk_widget_set_sensitive (rc_spin, FALSE);
}

static void
reply_set_adr (GtkWidget * widget, char deliver)
{
  user_rc.deliver = deliver;
}



static void
custom_ok (GtkWidget * widget)
{
  if (check_flag)
    user_rc.message_check_interval = (gint) gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (rc_spin)) * 1000;
  else
    user_rc.message_check_interval = 0;

  user_rc.quote = *gtk_entry_get_text (GTK_ENTRY (rc_quote));
  g_strlcpy (user_rc.quote_string, gtk_entry_get_text (GTK_ENTRY (rc_header)), sizeof(user_rc.quote_string));
  g_strlcpy (user_rc.deliver_adr, gtk_entry_get_text (GTK_ENTRY (rc_dhost)), sizeof(user_rc.deliver_adr));
  g_strlcpy (user_rc.samba_username, gtk_entry_get_text (GTK_ENTRY (rc_samba_username)), sizeof(user_rc.samba_username));
  g_strlcpy (user_rc.samba_nbiosname, gtk_entry_get_text (GTK_ENTRY (rc_samba_nbiosname)), sizeof(user_rc.samba_nbiosname));
  g_strlcpy (user_rc.samba_misc, gtk_entry_get_text (GTK_ENTRY (rc_samba_misc)), sizeof(user_rc.samba_misc));

  user_rc.confirm = GTK_TOGGLE_BUTTON (rc_check_confirm)->active;
  user_rc.self_send = GTK_TOGGLE_BUTTON (rc_check_self_send)->active;
  user_rc.auto_iconify = GTK_TOGGLE_BUTTON (rc_check_auto_iconify)->active;
  user_rc.wrap_to_first = GTK_TOGGLE_BUTTON (rc_check_wrap_to_first)->active;
  user_rc.beep_new_message = GTK_TOGGLE_BUTTON (rc_check_beep_new_message)->active;
  user_rc.minimize_to_tray = GTK_TOGGLE_BUTTON (rc_check_minimize_to_tray)->active;


  save_rc ();
  gtk_widget_destroy (rc_window);
}



static void
custom_cancel (GtkWidget * widget)
{
  read_rc ();
  toolbar_set_style (widget, user_rc.toolbar_style);
  gtk_widget_destroy (rc_window);
}


static void
custom_reset (GtkWidget * widget)
{
  create_rc ();
  toolbar_set_style (widget, user_rc.toolbar_style);
  gtk_widget_destroy (rc_window);
  custom_notebook ();
}



void
custom_notebook (void)
{
  GtkWidget *rc_notebook;
  GtkWidget *rc_vbox;
  GtkWidget *rc_hbbox;
  GtkWidget *rc_frame;
  GtkWidget *rc_frame_table;
  GtkWidget *rc_radio_text;
  GtkWidget *rc_radio_icons;
  GtkWidget *rc_radio_both;
  GtkWidget *rc_radio_check;
  GtkWidget *rc_radio_never;
  GtkWidget *rc_radio_hcolors;
  GtkWidget *rc_radio_mcolors;
  GtkWidget *rc_radio_default;
  GtkWidget *rc_radio_guess;
  GtkWidget *rc_radio_blank;
  GtkWidget *rc_label;
  GtkWidget *rc_button_ok;
  GtkWidget *rc_button_cancel;
  GtkWidget *rc_button_reset;
  GtkObject *rc_adj;

  gint check_interval;
  char quote[2];
  gint x, y;


  /* Preferences window */
  rc_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_widget_realize (rc_window);
  gtk_window_set_title (GTK_WINDOW (rc_window), _("Preferences"));
  gdk_window_get_origin (window->window, &x, &y);
  gtk_window_move (GTK_WINDOW (rc_window), x, y);
  gtk_window_set_transient_for (GTK_WINDOW (rc_window), GTK_WINDOW (window));
  g_signal_connect (GTK_OBJECT (rc_window), "delete_event", GTK_SIGNAL_FUNC (custom_cancel), NULL);



  /* vbox for notebook and buttons */
  rc_vbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (rc_vbox);
  gtk_container_add (GTK_CONTAINER (rc_window), rc_vbox);

  rc_notebook = gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (rc_notebook), GTK_POS_TOP);
  gtk_widget_show (rc_notebook);
  gtk_box_pack_start (GTK_BOX (rc_vbox), rc_notebook, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (rc_notebook), 5);

  rc_hbbox = gtk_hbutton_box_new ();
  gtk_widget_show (rc_hbbox);
  gtk_box_pack_start (GTK_BOX (rc_vbox), rc_hbbox, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (rc_hbbox), 5);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (rc_hbbox), GTK_BUTTONBOX_END);
  gtk_box_set_spacing (GTK_BOX (rc_hbbox), 5);

  rc_button_ok = box_button (rc_hbbox, _("Done"));
  rc_button_cancel = box_button (rc_hbbox, _("Cancel"));
  rc_button_reset = box_button (rc_hbbox, _("Reset"));

  g_signal_connect (GTK_OBJECT (rc_button_ok), "clicked", GTK_SIGNAL_FUNC (custom_ok), NULL);
  g_signal_connect (GTK_OBJECT (rc_button_cancel), "clicked", GTK_SIGNAL_FUNC (custom_cancel), NULL);
  g_signal_connect (GTK_OBJECT (rc_button_reset), "clicked", GTK_SIGNAL_FUNC (custom_reset), NULL);



  /* vbox for Messages tab */
  rc_vbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (rc_vbox);
  rc_label = gtk_label_new (_("Messages"));
  gtk_notebook_append_page (GTK_NOTEBOOK (rc_notebook), rc_vbox, rc_label);



  rc_frame = create_frame (rc_vbox, _("Check for incoming messages"), FALSE, FALSE, GTK_SHADOW_ETCHED_IN);

  rc_frame_table = gtk_table_new (2, 2, TRUE);
  gtk_widget_show (rc_frame_table);
  gtk_container_add (GTK_CONTAINER (rc_frame), rc_frame_table);
  gtk_container_set_border_width (GTK_CONTAINER (rc_frame_table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (rc_frame_table), 5);
  gtk_table_set_col_spacings (GTK_TABLE (rc_frame_table), 5);

  rc_radio_check = gtk_radio_button_new_with_mnemonic (NULL, _("Every ... second(s):"));
  gtk_widget_show (rc_radio_check);
  gtk_table_attach (GTK_TABLE (rc_frame_table), rc_radio_check, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 5, 0);

  rc_radio_never = gtk_radio_button_new_with_mnemonic (gtk_radio_button_get_group (GTK_RADIO_BUTTON (rc_radio_check)), _("Never"));
  gtk_widget_show (rc_radio_never);
  gtk_table_attach (GTK_TABLE (rc_frame_table), rc_radio_never, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 5, 0);

  if (user_rc.message_check_interval)
    check_interval = (guint32) user_rc.message_check_interval / 1000;
  else
    check_interval = 2;

  rc_adj = gtk_adjustment_new (check_interval, 1, 10, 1, 0, 0);
  rc_spin = gtk_spin_button_new (GTK_ADJUSTMENT (rc_adj), 0, 0);
  gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (rc_spin), GTK_UPDATE_ALWAYS);
  gtk_widget_show (rc_spin);
  gtk_table_attach (GTK_TABLE (rc_frame_table), rc_spin, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 5, 0);

  g_signal_connect (GTK_OBJECT (rc_radio_check), "clicked", GTK_SIGNAL_FUNC (check_set_interval), GINT_TO_POINTER (TRUE));
  g_signal_connect (GTK_OBJECT (rc_radio_never), "clicked", GTK_SIGNAL_FUNC (check_set_interval), GINT_TO_POINTER (FALSE));


  if (!user_rc.message_check_interval)
    {
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rc_radio_never), TRUE);
      check_flag = FALSE;
    }
  else
    {
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rc_radio_check), TRUE);
      check_flag = TRUE;
    }



  rc_frame = create_frame (rc_vbox, _("Reply "), FALSE, FALSE, GTK_SHADOW_ETCHED_IN);

  rc_frame_table = gtk_table_new (2, 2, TRUE);
  gtk_container_set_border_width (GTK_CONTAINER (rc_frame_table), 5);
  gtk_container_add (GTK_CONTAINER (rc_frame), rc_frame_table);
  gtk_widget_show (rc_frame_table);
  gtk_table_set_row_spacings (GTK_TABLE (rc_frame_table), 5);
  gtk_table_set_col_spacings (GTK_TABLE (rc_frame_table), 5);

  rc_label = gtk_label_new (_("Quote character:"));
  gtk_widget_show (rc_label);
  gtk_table_attach (GTK_TABLE (rc_frame_table), rc_label, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 2, 0);
  gtk_label_set_justify (GTK_LABEL (rc_label), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (rc_label), 0, 0.5);

  quote[0] = user_rc.quote;
  quote[1] = '\0';

  rc_quote = gtk_entry_new ();
  gtk_entry_set_max_length (GTK_ENTRY (rc_quote), 1);
  gtk_entry_set_text (GTK_ENTRY (rc_quote), quote);
  gtk_entry_set_editable (GTK_ENTRY (rc_quote), TRUE);
  gtk_widget_show (rc_quote);
  gtk_table_attach (GTK_TABLE (rc_frame_table), rc_quote, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 5, 0);

  rc_label = gtk_label_new (_("Reply Header:"));
  gtk_widget_show (rc_label);
  gtk_table_attach (GTK_TABLE (rc_frame_table), rc_label, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 5, 0);
  gtk_label_set_justify (GTK_LABEL (rc_label), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (rc_label), 0, 0.5);

  rc_header = gtk_entry_new ();
  gtk_entry_set_max_length (GTK_ENTRY (rc_header), 50);
  gtk_entry_set_text (GTK_ENTRY (rc_header), user_rc.quote_string);
  gtk_entry_set_editable (GTK_ENTRY (rc_header), TRUE);
  gtk_widget_show (rc_header);
  gtk_table_attach (GTK_TABLE (rc_frame_table), rc_header, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 5, 0);



  rc_frame = create_frame (rc_vbox, _("Delivery address"), FALSE, FALSE, GTK_SHADOW_ETCHED_IN);

  rc_frame_table = gtk_table_new (3, 2, TRUE);
  gtk_container_set_border_width (GTK_CONTAINER (rc_frame_table), 5);
  gtk_container_add (GTK_CONTAINER (rc_frame), rc_frame_table);
  gtk_widget_show (rc_frame_table);
  gtk_table_set_row_spacings (GTK_TABLE (rc_frame_table), 5);
  gtk_table_set_col_spacings (GTK_TABLE (rc_frame_table), 5);

  rc_radio_default = gtk_radio_button_new_with_mnemonic (NULL, _("Use default host:"));
  gtk_widget_show (rc_radio_default);
  gtk_table_attach (GTK_TABLE (rc_frame_table), rc_radio_default, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 5, 0);

  rc_radio_guess = gtk_radio_button_new_with_mnemonic (gtk_radio_button_get_group (GTK_RADIO_BUTTON (rc_radio_default)), _("Guess"));
  gtk_widget_show (rc_radio_guess);
  gtk_table_attach (GTK_TABLE (rc_frame_table), rc_radio_guess, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 5, 0);

  rc_radio_blank = gtk_radio_button_new_with_mnemonic (gtk_radio_button_get_group (GTK_RADIO_BUTTON (rc_radio_default)), _("Leave blank"));
  gtk_widget_show (rc_radio_blank);
  gtk_table_attach (GTK_TABLE (rc_frame_table), rc_radio_blank, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 5, 0);

  rc_dhost = gtk_entry_new ();
  gtk_entry_set_max_length (GTK_ENTRY (rc_dhost), 100);
  gtk_entry_set_text (GTK_ENTRY (rc_dhost), user_rc.deliver_adr);
  gtk_entry_set_editable (GTK_ENTRY (rc_dhost), TRUE);
  gtk_table_attach (GTK_TABLE (rc_frame_table), rc_dhost, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 5, 0);
  gtk_widget_show (rc_dhost);

  g_signal_connect (GTK_OBJECT (rc_radio_default), "clicked", GTK_SIGNAL_FUNC (reply_set_adr), GINT_TO_POINTER (DHOST));
  g_signal_connect (GTK_OBJECT (rc_radio_blank), "clicked", GTK_SIGNAL_FUNC (reply_set_adr), GINT_TO_POINTER (BLANK));
  g_signal_connect (GTK_OBJECT (rc_radio_guess), "clicked", GTK_SIGNAL_FUNC (reply_set_adr), GINT_TO_POINTER (GUESS));


  switch (user_rc.deliver)
    {
    case DHOST:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rc_radio_default), TRUE);
      break;
    case BLANK:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rc_radio_blank), TRUE);
      break;
    default:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rc_radio_guess), TRUE);
    }



  /* vbox for Samba Settings tab */
  rc_vbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (rc_vbox);
  rc_label = gtk_label_new (_("Samba Settings"));
  gtk_notebook_append_page (GTK_NOTEBOOK (rc_notebook), rc_vbox, rc_label);

  rc_frame = create_frame (rc_vbox, _("Identity"), FALSE, FALSE, GTK_SHADOW_ETCHED_IN);

  rc_frame_table = gtk_table_new (2, 2, TRUE);
  gtk_container_set_border_width (GTK_CONTAINER (rc_frame_table), 5);
  gtk_container_add (GTK_CONTAINER (rc_frame), rc_frame_table);
  gtk_widget_show (rc_frame_table);
  gtk_table_set_row_spacings (GTK_TABLE (rc_frame_table), 5);
  gtk_table_set_col_spacings (GTK_TABLE (rc_frame_table), 5);

  rc_label = gtk_label_new (_("User Name:"));
  gtk_widget_show (rc_label);
  gtk_table_attach (GTK_TABLE (rc_frame_table), rc_label, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 5, 0);
  gtk_label_set_justify (GTK_LABEL (rc_label), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (rc_label), 0, 0.5);

  rc_samba_username = gtk_entry_new ();
  gtk_entry_set_max_length (GTK_ENTRY (rc_samba_username), sizeof (user_rc.samba_username) - 1);
  gtk_entry_set_text (GTK_ENTRY (rc_samba_username), user_rc.samba_username);
  gtk_entry_set_editable (GTK_ENTRY (rc_samba_username), TRUE);
  gtk_widget_show (rc_samba_username);
  gtk_table_attach (GTK_TABLE (rc_frame_table), rc_samba_username, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 5, 0);

  rc_label = gtk_label_new (_("Host Name:"));
  gtk_widget_show (rc_label);
  gtk_table_attach (GTK_TABLE (rc_frame_table), rc_label, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 5, 0);
  gtk_label_set_justify (GTK_LABEL (rc_label), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (rc_label), 0, 0.5);

  rc_samba_nbiosname = gtk_entry_new ();
  gtk_entry_set_max_length (GTK_ENTRY (rc_samba_nbiosname), sizeof (user_rc.samba_nbiosname) - 1);
  gtk_entry_set_text (GTK_ENTRY (rc_samba_nbiosname), user_rc.samba_nbiosname);
  gtk_entry_set_editable (GTK_ENTRY (rc_samba_nbiosname), TRUE);
  gtk_widget_show (rc_samba_nbiosname);
  gtk_table_attach (GTK_TABLE (rc_frame_table), rc_samba_nbiosname, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 5, 0);



  rc_frame = create_frame (rc_vbox, _("Arguments"), FALSE, FALSE, GTK_SHADOW_ETCHED_IN);
  rc_frame_table = gtk_table_new (1, 2, TRUE);
  gtk_container_set_border_width (GTK_CONTAINER (rc_frame_table), 5);
  gtk_container_add (GTK_CONTAINER (rc_frame), rc_frame_table);
  gtk_widget_show (rc_frame_table);
  gtk_table_set_row_spacings (GTK_TABLE (rc_frame_table), 5);
  gtk_table_set_col_spacings (GTK_TABLE (rc_frame_table), 5);

  rc_label = gtk_label_new (_("Optional:"));
  gtk_widget_show (rc_label);
  gtk_table_attach (GTK_TABLE (rc_frame_table), rc_label, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 5, 0);
  gtk_label_set_justify (GTK_LABEL (rc_label), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (rc_label), 0, 0.5);

  rc_samba_misc = gtk_entry_new ();
  gtk_entry_set_max_length (GTK_ENTRY (rc_samba_misc), sizeof (user_rc.samba_misc) - 1);
  gtk_entry_set_text (GTK_ENTRY (rc_samba_misc), user_rc.samba_misc);
  gtk_entry_set_editable (GTK_ENTRY (rc_samba_misc), TRUE);
  gtk_widget_show (rc_samba_misc);
  gtk_table_attach (GTK_TABLE (rc_frame_table), rc_samba_misc, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 5, 0);



  /* vbox for General Options tab */
  rc_vbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (rc_vbox);
  rc_label = gtk_label_new (_("General Options"));
  gtk_notebook_append_page (GTK_NOTEBOOK (rc_notebook), rc_vbox, rc_label);

  rc_frame = create_frame (rc_vbox, _("Options"), FALSE, FALSE, GTK_SHADOW_ETCHED_IN);

  rc_frame_table = gtk_table_new (6, 1, TRUE);
  gtk_widget_show (rc_frame_table);
  gtk_container_add (GTK_CONTAINER (rc_frame), rc_frame_table);
  gtk_container_set_border_width (GTK_CONTAINER (rc_frame_table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (rc_frame_table), 5);
  gtk_table_set_col_spacings (GTK_TABLE (rc_frame_table), 5);

  rc_check_confirm = gtk_check_button_new_with_mnemonic (_("Delivery confirmation"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rc_check_confirm), user_rc.confirm);
  gtk_widget_show (rc_check_confirm);
  gtk_table_attach (GTK_TABLE (rc_frame_table), rc_check_confirm, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 5, 0);

  rc_check_wrap_to_first = gtk_check_button_new_with_mnemonic (_("Read wrap-around, last message to first"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rc_check_wrap_to_first), user_rc.wrap_to_first);
  gtk_widget_show (rc_check_wrap_to_first);
  gtk_table_attach (GTK_TABLE (rc_frame_table), rc_check_wrap_to_first, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 5, 0);

  rc_check_self_send = gtk_check_button_new_with_mnemonic (_("Copy all messages to self"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rc_check_self_send), user_rc.self_send);
  gtk_widget_show (rc_check_self_send);
  gtk_table_attach (GTK_TABLE (rc_frame_table), rc_check_self_send, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 5, 0);

  rc_check_auto_iconify = gtk_check_button_new_with_mnemonic (_("Auto-iconify on last deleted message"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rc_check_auto_iconify), user_rc.auto_iconify);
  gtk_widget_show (rc_check_auto_iconify);
  gtk_table_attach (GTK_TABLE (rc_frame_table), rc_check_auto_iconify, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 5, 0);

  rc_check_beep_new_message = gtk_check_button_new_with_mnemonic (_("Beep on new message arrival"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rc_check_beep_new_message), user_rc.beep_new_message);
  gtk_widget_show (rc_check_beep_new_message);
  gtk_table_attach (GTK_TABLE (rc_frame_table), rc_check_beep_new_message, 0, 1, 4, 5,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 5, 0);

  rc_check_minimize_to_tray = gtk_check_button_new_with_mnemonic (_("Minimize to tray"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rc_check_minimize_to_tray), user_rc.minimize_to_tray);
  gtk_widget_show (rc_check_minimize_to_tray);
  gtk_table_attach (GTK_TABLE (rc_frame_table), rc_check_minimize_to_tray, 0, 1, 5, 6,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 5, 0);


  rc_frame = create_frame (rc_vbox, _("Toolbar"), FALSE, FALSE, GTK_SHADOW_ETCHED_IN);

  rc_frame_table = gtk_table_new (3, 2, TRUE);
  gtk_widget_show (rc_frame_table);
  gtk_container_add (GTK_CONTAINER (rc_frame), rc_frame_table);
  gtk_container_set_border_width (GTK_CONTAINER (rc_frame_table), 5);

  rc_radio_both = gtk_radio_button_new_with_mnemonic (NULL, _("Icons and text"));
  gtk_widget_show (rc_radio_both);
  gtk_table_attach (GTK_TABLE (rc_frame_table), rc_radio_both, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 5, 0);

  rc_radio_icons = gtk_radio_button_new_with_mnemonic (gtk_radio_button_get_group (GTK_RADIO_BUTTON (rc_radio_both)), _("Icons only"));
  gtk_widget_show (rc_radio_icons);
  gtk_table_attach (GTK_TABLE (rc_frame_table), rc_radio_icons, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 5, 0);

  rc_radio_text = gtk_radio_button_new_with_mnemonic (gtk_radio_button_get_group (GTK_RADIO_BUTTON (rc_radio_both)), _("Text only"));
  gtk_widget_show (rc_radio_text);
  gtk_table_attach (GTK_TABLE (rc_frame_table), rc_radio_text, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 5, 0);

  g_signal_connect (GTK_OBJECT (rc_radio_both), "clicked", GTK_SIGNAL_FUNC (toolbar_set_style), GINT_TO_POINTER (GTK_TOOLBAR_BOTH));
  g_signal_connect (GTK_OBJECT (rc_radio_icons), "clicked", GTK_SIGNAL_FUNC (toolbar_set_style), GINT_TO_POINTER (GTK_TOOLBAR_ICONS));
  g_signal_connect (GTK_OBJECT (rc_radio_text), "clicked", GTK_SIGNAL_FUNC (toolbar_set_style), GINT_TO_POINTER (GTK_TOOLBAR_TEXT));

  switch (user_rc.toolbar_style)
    {
    case GTK_TOOLBAR_TEXT:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rc_radio_text), TRUE);
      break;
    case GTK_TOOLBAR_ICONS:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rc_radio_icons), TRUE);
      break;
    default:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rc_radio_both), TRUE);
    }

  rc_radio_hcolors = gtk_radio_button_new_with_mnemonic (NULL, _("High colors icons set"));
  gtk_widget_show (rc_radio_hcolors);
  gtk_table_attach (GTK_TABLE (rc_frame_table), rc_radio_hcolors, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 5, 0);

  rc_radio_mcolors = gtk_radio_button_new_with_mnemonic (gtk_radio_button_get_group (GTK_RADIO_BUTTON (rc_radio_hcolors)), _("16 colors icons set"));
  gtk_widget_show (rc_radio_mcolors);
  gtk_table_attach (GTK_TABLE (rc_frame_table), rc_radio_mcolors, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 5, 0);

  g_signal_connect (GTK_OBJECT (rc_radio_hcolors), "clicked", GTK_SIGNAL_FUNC (icons_set_colors), GINT_TO_POINTER (TRUE));
  g_signal_connect (GTK_OBJECT (rc_radio_mcolors), "clicked", GTK_SIGNAL_FUNC (icons_set_colors), GINT_TO_POINTER (FALSE));

  if (user_rc.hcolors)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rc_radio_hcolors), TRUE);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rc_radio_mcolors), TRUE);


  /* show this window */
  gtk_widget_show (rc_window);
}

/* EOF */
