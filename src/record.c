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
#include <gtk/gtk.h>
#include <gst/gst.h>

#include "config.h"
#include "record.h"

gfloat currentBytes = 0, totalBytes = 0;
GtkWidget *drawingareaLevel = NULL, *dialog, *progress;
typedef enum CallbackMessage CallbackMessage;
enum CallbackMessage
{
	COPY, ENCODE 
};

void draw_level(gdouble *rms, gint channels)
{
	gint i;
	GdkRectangle rect = { 0, 0, 120, 25 };
	cairo_t *cr;

  if (drawingareaLevel != NULL){
    //return;

	  gdk_window_begin_paint_rect (gtk_widget_get_window (drawingareaLevel), &rect);

	  cr = gdk_cairo_create (gtk_widget_get_window (drawingareaLevel));
	
	  cairo_set_source_rgb (cr, 1, 0, 0);
	  cairo_rectangle (cr, 5, 0, (rms[0] * 150), 8);
	  if (channels > 1) {
	  	cairo_rectangle (cr, 5, 17, (rms[1] * 150), 8);
		}
	  cairo_fill (cr);
	
	  cairo_destroy (cr);

	  gdk_window_end_paint (gtk_widget_get_window (drawingareaLevel));
	}
}

void activate_recording(GtkWidget *pToggle, gpointer data)
{
	gboolean state;
	state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pToggle));
	if (state){
		act_rec = TRUE;
		/*if (loop != NULL){
			g_main_loop_quit(loop);
			}*/
		gtk_widget_set_sensitive (recordButton, TRUE);
		playType = ACT_REC;
		playFromSource(NULL, "act_rec");
		}
	else {
		act_rec = FALSE;
		playing = 0; pose = 0;
		if (loop != NULL){
			g_main_loop_quit(loop);
			}
		gtk_widget_set_sensitive (recordButton, FALSE);
		gdouble *rms = g_new (gdouble, 2);
		rms[0] = 0; rms[1] = 0;
		draw_level(rms, 2);
		}
}

void show_progression() {
	GtkWidget *VBox;
	GtkWidget *button;

	progressWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_modal (GTK_WINDOW(progressWindow), TRUE);
	gtk_window_set_title(GTK_WINDOW(progressWindow), "Encode file");
	gtk_window_set_default_size(GTK_WINDOW(progressWindow), 320, 200);
	gtk_container_set_border_width(GTK_CONTAINER(progressWindow), 4);

	VBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(progressWindow), VBox);

	progress = gtk_progress_bar_new();
#ifdef GTK2
	gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(progress), GTK_PROGRESS_LEFT_TO_RIGHT);
#endif
	gtk_box_pack_start(GTK_BOX(VBox), progress, TRUE, FALSE, 0);
	button = gtk_button_new_with_label("Cancel");
	gtk_box_pack_start(GTK_BOX(VBox), button, TRUE, FALSE, 0);
	gtk_widget_show_all(progressWindow);

	g_signal_connect_swapped(G_OBJECT(button), "clicked", G_CALLBACK(onStop), progress);
	g_timeout_add (200, (GSourceFunc) update_progress_bar, progress);
}

void save_file (){
	if ((act_rec && g_file_test(tmpPath, G_FILE_TEST_IS_REGULAR)) || (act_rec == FALSE && playlistStoreSize > 0)) {
		dialog = gtk_file_chooser_dialog_new ("Save File As ...", GTK_WINDOW(mainWindow),
		GTK_FILE_CHOOSER_ACTION_SAVE,
		"Cancel", GTK_RESPONSE_CANCEL,
		"Save", GTK_RESPONSE_ACCEPT,
		NULL);
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), g_get_home_dir());
		gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), "record.flac");
		gint result = gtk_dialog_run (GTK_DIALOG (dialog));
		if (result == GTK_RESPONSE_ACCEPT){
			gchar *name = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(dialog));
			encodePath = g_filename_from_uri(name, NULL, NULL);
			show_progression();
			gtk_widget_destroy (dialog);
			//playType = SAVE;
			/*if (act_rec && loop != NULL){
				printf("record : act_rec && loop != NULL\n");
				redo = 1;
				g_main_loop_quit(loop);
				}
			if (act_rec == FALSE && playing == 0){*/
				playFromSource(NULL, "save");
				//}
			}
		else if (result == GTK_RESPONSE_CANCEL){
			gtk_widget_destroy (dialog);
			}
	}
	else {
		printf("Nothing to save\n");
		}
}




