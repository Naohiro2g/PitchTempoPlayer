/* This file is part of PitchTempoPlayer (PTPlayer).

    PitchTempoPlayer is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    PitchTempoPlayer is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with PitchTempoPlayer.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gst/gst.h>
#include <stdbool.h>
#include <gtk/gtk.h>

#include "config.h"

#include "onclick.h"

static GtkWidget *mainWindow;

void reset_view()
{
	g_signal_handlers_block_by_func(G_OBJECT(GTK_RANGE(pScaleSeek)), G_CALLBACK(on_seek), NULL);
	gtk_range_set_value(GTK_RANGE(pScaleSeek), 0);
	g_signal_handlers_unblock_by_func(G_OBJECT(GTK_RANGE(pScaleSeek)), G_CALLBACK(on_seek), NULL);
	showPosition("      0.00 / 0.00      ");
	
	setPlayButtonIcon();
}

/* stop playing audio */
void onStop() 
{ 
	playing = 0; pose = 0;
	if (loop != NULL){
		stop = 1;
		g_main_loop_quit(loop);
		}
	reset_view();
}

void seek (GtkButton *button, GdkEvent *event, gpointer *data) {
	fast =  atoi((gchar *)data);
}

void previousTrack(){
	if (playing == 1){
		if (pos < (gint64)1000000000){
			if (playlistPosition > 0){
				playlistPosition -= 2;
				}
			else {
				playlistPosition -= 1;
				}
			}
		else {
			playlistPosition -= 1;
			}
		}
	else if (playing == 0){
		if (playlistPosition > 0){
			playlistPosition -= 1;
			}
		getFileName();
		}
	if (loop != NULL){
		g_main_loop_quit(loop);
		}
}

void nextTrack(){
	if (playing == 0){
		if (playlistPosition < (playlistStoreSize - 1)){
			playlistPosition += 1;
			}
		getFileName();
		}
	if (loop != NULL){
		g_main_loop_quit(loop);
		}
}

void on_seek (GtkRange *range, gchar *data)
{
	gdouble value = gtk_range_get_value (range);
	if (playing){
		gint64 seekPos = len * value/100;
		if (!gst_element_seek (pipeline, 1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH,
			GST_SEEK_TYPE_SET, seekPos,
			GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE)) {
			g_print ("Seek failed!\n");
			}
		}
}

void check_mute(GtkWidget *pToggle, gpointer data)
{
	gboolean state;
	state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pToggle));
	if (state){
		g_object_set (G_OBJECT (playbin), "mute", TRUE, NULL);
		}
	else {
		g_object_set (G_OBJECT (playbin), "mute", FALSE, NULL);
		}
}

void volume_changed (GtkWidget *widget, gpointer data)
{
	volume = gtk_scale_button_get_value(GTK_SCALE_BUTTON(widget));
	if (playing){
		g_object_set (G_OBJECT (playbin), "volume", volume, NULL);
		}
}

void change_panorama (GtkRange *range, gchar *data)
{
	gdouble value = gtk_range_get_value (range);
	if (playing){
		g_object_set (G_OBJECT (audiopanorama), "panorama", (gfloat)value, NULL);
		}
}

void use_jack(GtkWidget *pToggle, gpointer data)
{
	gboolean state;
	state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pToggle));
	if (state){
		JACK = TRUE;
		}
	else {
		JACK = FALSE;
		}
}

void errorMessageWindowJack(char *message){
	GtkWidget *pAbout;
	pAbout = gtk_message_dialog_new (GTK_WINDOW(mainWindow),
		GTK_DIALOG_MODAL,
		GTK_MESSAGE_WARNING,
		GTK_BUTTONS_OK,
		"%s", message);
	gtk_window_set_transient_for (GTK_WINDOW(pAbout), GTK_WINDOW(mainWindow));
	gtk_dialog_run(GTK_DIALOG(pAbout));
     	gtk_widget_destroy(pAbout);
	
	playing = 0;
}


void errorMessageWindow(char *message){
	GtkWidget *pAbout;
	pAbout = gtk_message_dialog_new (GTK_WINDOW(mainWindow),
		GTK_DIALOG_MODAL,
		GTK_MESSAGE_WARNING,
		GTK_BUTTONS_OK,
		"%s", message);
	gtk_window_set_transient_for (GTK_WINDOW(pAbout), GTK_WINDOW(mainWindow));
	gtk_dialog_run(GTK_DIALOG(pAbout));
     	gtk_widget_destroy(pAbout);
}





