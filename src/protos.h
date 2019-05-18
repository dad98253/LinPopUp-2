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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <time.h>
#include <fcntl.h>

/* Standard gettext macros */
#ifdef ENABLE_NLS
#  include <libintl.h>
#  undef _
#  define _(String) dgettext (PACKAGE, String)
#  ifdef gettext_noop
#    define N_(String) gettext_noop (String)
#  else
#    define N_(String) (String)
#  endif
#else
#  define textdomain(String) (String)
#  define gettext(String) (String)
#  define dgettext(Domain,Message) (Message)
#  define dcgettext(Domain,Message,Type) (Message)
#  define bindtextdomain(Domain,Directory) (Domain)
#  define _(String) (String)
#  define N_(String) (String)
#endif

/* DEFINES -  GTK  */
#define ACCEL(x,y,z)  gtk_widget_add_accelerator (x, "activate", accel_, y, z, GTK_ACCEL_VISIBLE);
#define PROCESS_EVENTS  while (gtk_events_pending ())  gtk_main_iteration ()


/* DEFINES - WWW */
#define USE_NETSCAPE 1
#define HOME_URL "http://sourceforge.net/projects/linpopup2/"
#define FORUMS_URL "http://sourceforge.net/tracker/?group_id=83081"

/* DEFINES - OTHER */
#ifndef DATA_FILE
#define DATA_FILE "/var/lib/linpopup/messages.dat"
#endif

#ifndef SHARE_DIR
#define SHARE_DIR "/usr/local/share/linpopup"
#endif

#define SHARE_PATH SHARE_DIR
#define PIXMAPS_PATH SHARE_PATH"/pixmaps"


#define DOC_COPYING 0
#define DOC_README 1
#define DOC_CHANGES 2
#define DOC_MANUAL 3
#define HOW_MANY_DOC 4

#define DATA_PERM S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH	//0666
#define RC_PERM S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH	//0644
#define SAVE_PERM S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH	//0644
#define BLANK 0
#define GUESS 1
#define DHOST 2

#define MAIN_WIDTH 390
#define MAIN_HEIGHT 350
#define SEND_WIDTH 390
#define SEND_HEIGHT 350


#define MAX_SEND_HISTORY_LINES 20

/* GLOBALES - GTK */
GtkWidget *window;

GtkWidget *main_statusbar[4];
GtkWidget *main_label_from;
GtkWidget *main_toolbar;
GtkWidget *main_text;
GtkWidget *send_window;
GtkWidget *send_text;


guint main_context_id[4];
guint main_message_id[4];
guint32 check_for_new_message_timeout;


/* GLOBALES - OTHER  */
char samba_version[80];
char samba_installed;
char processing_new_message;
int total_number_of_messages;
int new_messages;
int first_new_message;
int current_showed_message;
char is_it_a_reply;
struct linpopup_message
{
  time_t time;
  char sender[100];
  char host[100];
  char to[100];
  char text[1700];
  char is_new;
  char may_be_used_or_not_in_a_futur_release[1992];
};




/* MENU */
void create_main_menu (GtkWidget * vbox);
void create_send_menu (GtkWidget * vbox);


/* TOOLBAR */
GtkWidget *create_toolbar (GtkWidget * window, GtkWidget * vbox);
GtkWidget *new_pixmap (GtkWidget * widget, char **data);


/* TEXT */
void create_text_area (GtkWidget * this_text, GtkWidget * vbox);
void clear_text (GtkWidget * this_text);
void realize_text (GtkWidget * this_text, gpointer data);
void text_copy_to_clipboard (GtkWidget * widget, GtkWidget * this_text);
void text_cut_to_clipboard (GtkWidget * widget, GtkWidget * this_text);
void text_paste_from_clipboard (GtkWidget * widget, GtkWidget * this_text);
void text_select_all (GtkWidget * widget, GtkWidget * this_text);


/* DIALOG */
GtkWidget *create_dialog_window (GtkWindowType win_TYPE, GtkWindowPosition win_POS, char *win_title, GtkWidget ** this_box);
int alert_dialog (gchar * title, gchar * string, gchar * first_button, gchar * second_button, int selected);
void alert_ok (gchar * title, gchar * alert_message, gchar * alert_button_label);
void alert_ok_text (gchar * tittle, gchar * message);
void about_dialog (void);
void doc_dialog (GtkWidget * widget, int index);
void resizable_text_dialog (gchar * title, char *this_text, int width, int height);


/* SEND */
void send_message (void);
void reply_message (void);
int send_send (GtkWidget * widget, gpointer * data);
void message_load (GtkWidget * widget, gpointer data);


/* RECEPT */
void trap_message (int argc, char *argv[]);
int are_there_new_messages_in_data_file (void);
gint check_for_new_message (gpointer data);
int count_total_number_of_messages (void);
void show_no_message (void);
void show_message (int message_number);
void show_next_message (gpointer * data);
void show_previous_message (gpointer * data);
void message_save (GtkWidget * widget, int data);
void delete_showed_message (void);
void delete_all_messages (void);
void read_message (int message_number, struct linpopup_message *this_message);


/* IO */
void fatal_error (int code, char *data);
void vital_check_for_read_access (char *filename);
int lock_reg (int fd, int cmd, int type, off_t offset, int whence, off_t len);


/* CUSTOM */
void init_rc (void);
void save_rc (void);
void save_geom (void);
void reset_geom (void);
void custom_notebook (void);
struct linpopup_custom
{
  char version[6];
  GtkToolbarStyle toolbar_style;
  guint32 message_check_interval;
  char hcolors;
  char quote;
  char quote_string[50];
  char deliver;
  char deliver_adr[100];
  char guess_adr[100];
  GdkColor quoted_text_col;
  char samba_username[50];
  char samba_nbiosname[50];
  char samba_misc[100];
  char samba_warn;
  char confirm;
  char self_send;
  char auto_iconify;
  char wrap_to_first;
  char beep_new_message;
  int win_x;
  int win_y;
  int win_h;
  int win_w;
  gboolean minimize_to_tray;
}
user_rc;


/* VERSION */
char *many_version_infos (char *buf, size_t bufsize);
char *a_little_version_infos (char *buf, size_t bufsize);
char *what_is_this_stupid_programm_that_have_crashed (char *buf, size_t bufsize);
int how_many_lines_of_code (void);
char samba_info (char *buf, size_t bufsize);


/* STRING */
char *strexpand (char *string, char *search_string, char *replace_string, size_t bufsize);

/* GTKMISC */
GtkWidget *box_button (GtkWidget * box, char *label);
GtkWidget *v_box (GtkWidget * parent, char homogene, gint spacing, gint border);
GtkWidget *h_box (GtkWidget * parent, char homogene, gint spacing, gint border);
GtkWidget *hbutton_box (GtkWidget * box);
void hseparator (GtkWidget * box);
GtkWidget *create_frame (GtkWidget * parent, char *title, char h_expand, char w_expand, GtkShadowType shadow_type);
GtkWidget *create_label (GtkWidget * box, char *label_text, GtkJustification jtype, int x_pad, int y_pad);


/* DOCS */
void doc_show_text_file_from_widget (GtkWidget * widget, gpointer data);
void doc_show_text_file (int this_doc);


/* EXECUTE  */
void execute_program (char *program, char *arg);
void open_URL (char *this_url);
void open_URL_from_widget (GtkWidget * widget, gpointer data);
void open_mail_client (void);
void open_new_message (void);
void open_news_client (void);

/* TRAY */
#include "../lib/eggtrayicon.h"
EggTrayIcon *tray_icon;

/* EOF */
