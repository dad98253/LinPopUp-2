/*
 * Tray processing events.
 *
 * @author Roman Rybalko <roman_rybalko@users.sf.net>
 *
*/


#include "protos.h"
#include "../lib/eggtrayicon.h"
#include <gtk/gtk.h>

GtkWidget *menu;

gboolean
on_button_press(GtkWidget *widget, GdkEventButton *event, gboolean not_used)
{
    switch(event->button){
	case 1:
	    gtk_window_deiconify (GTK_WINDOW (window));
	    break;
	case 2:
	case 3:
	    gtk_menu_popup( GTK_MENU(menu),NULL,NULL,NULL,NULL,event->button,event->time);
	    return TRUE;
    }
    return FALSE;
}

void tray_icon_init(){
    GtkWidget *image;
    GtkWidget *event_box;
    GtkWidget *item;
    

    tray_icon = egg_tray_icon_new("LinPopUp");
    event_box=gtk_event_box_new();
    image=gtk_image_new_from_file(PIXMAPS_PATH "/linpopup_tray.xpm");

    gtk_container_add(GTK_CONTAINER(event_box),GTK_WIDGET(image));
    gtk_container_add(GTK_CONTAINER(tray_icon),GTK_WIDGET(event_box));    
    
    g_signal_connect (GTK_OBJECT (tray_icon), "button-press-event", GTK_SIGNAL_FUNC (on_button_press), NULL);
    
//    gtk_widget_show_all(GTK_WIDGET(tray_icon));


    menu=gtk_menu_new();
    
    item=gtk_menu_item_new_with_label(_("Deiconify"));
    g_signal_connect_swapped (GTK_OBJECT (item), "button-press-event", GTK_SIGNAL_FUNC (gtk_window_deiconify), window);
    gtk_menu_attach(GTK_MENU(menu),GTK_WIDGET(item),0,1,0,1);
    
    gtk_menu_attach(GTK_MENU(menu),GTK_WIDGET(gtk_separator_menu_item_new()),0,1,1,2);

    item=gtk_menu_item_new_with_label(_("Quit"));
    g_signal_connect (GTK_OBJECT (item), "button-press-event", GTK_SIGNAL_FUNC (gtk_main_quit), NULL);
    gtk_menu_attach(GTK_MENU(menu),GTK_WIDGET(item),0,1,2,3);
    
    gtk_widget_show_all(GTK_WIDGET(menu));
}
