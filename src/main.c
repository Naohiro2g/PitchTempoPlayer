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
#include <errno.h>
#include <gdk/gdkkeysyms.h>

#include "config.h"

#include "main.h"

char prefPath[100];
gchar *selected;
gchar *filePathTree;
GtkTreeViewColumn *pColumn;
GtkCellRenderer *pCellRenderer;
GtkWidget *pListPlaylistView, *label, *pPlayButton, *forwardButton, *backwardButton; 
GdkRGBA color;

enum {
	IS_PLAYING_COLUMN,
	TITLE_COLUMN,
	NAME_COLUMN,
	N_COLUMN
	};

/* Quit main window and eveything */
static void on_window_destroy (GObject * object, gpointer user_data)
{
	printf("Quit everything\nGood Bye!\n");
	onStop();
	if (sFile != NULL){
		g_free(sFile);
		}
	gtk_main_quit ();
}

/*gboolean change_bg_button (GtkWidget *widget){
	//printf("change bg color\n");
	gchar *colorName = gdk_rgba_to_string(&color);
	if (strcmp(colorName, "#ffff00000000") == 0) {
		gdk_rgba_parse (&color, "gray");
		gtk_widget_override_background_color(GTK_WIDGET(widget), GTK_STATE_NORMAL, &color);
		gtk_widget_override_background_color(GTK_WIDGET(widget), GTK_STATE_PRELIGHT, &color);
		}
	else {
		gdk_rgba_parse (&color, "red");
		gtk_widget_override_background_color(GTK_WIDGET(widget), GTK_STATE_NORMAL, &color);
		gtk_widget_override_background_color(GTK_WIDGET(widget), GTK_STATE_PRELIGHT, &color);
		}
	
	if (pose){
		return TRUE;
		}
	else {
		gdk_rgba_parse (&color, "gray");
		gtk_widget_override_background_color(GTK_WIDGET(widget), GTK_STATE_NORMAL, &color);
		gtk_widget_override_background_color(GTK_WIDGET(widget), GTK_STATE_PRELIGHT, &color);
		return FALSE;
		}
} */

void setPlayButtonIcon (){
	if (playType == PLAY || playType == PLAY_REC || playType == REC){
		//printf("playing = %d\n", playing);
		GtkWidget *widget;
		GtkWidget *playImage;
		gchar *filename;
		if (playType == PLAY || playType == PLAY_REC){
			widget = pPlayButton;
			filename = g_build_filename (G_DIR_SEPARATOR_S, DATADIR, "icons", "ptp-play.png", NULL);
			playImage = gtk_image_new_from_file(filename);
			//playImage = gtk_image_new_from_stock (GTK_STOCK_MEDIA_PLAY, GTK_ICON_SIZE_LARGE_TOOLBAR);
			}
		else if (playType == REC){
			//printf("test\n");
			widget = recordButton;
			filename = g_build_filename (G_DIR_SEPARATOR_S, DATADIR, "icons", "ptp-record.png", NULL);
			playImage = gtk_image_new_from_file(filename);
			//playImage = gtk_image_new_from_stock (GTK_STOCK_MEDIA_RECORD, GTK_ICON_SIZE_BUTTON);
			}
		filename = g_build_filename (G_DIR_SEPARATOR_S, DATADIR, "icons", "ptp-pause.png", NULL);
		GtkWidget *pauseImage = gtk_image_new_from_file(filename);
		//GtkWidget *pauseImage = gtk_image_new_from_stock (GTK_STOCK_MEDIA_PAUSE, GTK_ICON_SIZE_LARGE_TOOLBAR);
		if (playing){
			//printf("playing!\n");
			if (pose){
				//printf("test1\n");
				gtk_button_set_image(GTK_BUTTON(widget),playImage);
				//g_timeout_add (500, (GSourceFunc) change_bg_button, widget);
				}
			else{
				//printf("test2\n");
				gtk_button_set_image(GTK_BUTTON(widget),pauseImage);
				}
			}
		else {
			gtk_button_set_image(GTK_BUTTON(widget),playImage);
			}
		}
}

void showPosition(gchar *positionLabel){
	gchar *sUtf8;
	sUtf8 = g_locale_to_utf8(positionLabel, -1, NULL, NULL, NULL);
	//label=gtk_label_new(sUtf8);
	gtk_label_set_label(GTK_LABEL(label), sUtf8);
	g_free(sUtf8);
	gtk_widget_show_all(label);

}

void select_save_file(GtkWidget *widget, gpointer data){
		if (act_rec){
			if (playType == ACT_REC){
				save_file();
				}
			}
}

void start_rec(){
	//printf("playing = %d\n", playing);
	if (playType == ACT_REC){
		playType = REC;
		redo = 1;
		if (loop != NULL){
			g_main_loop_quit(loop);
			}
		}
	else if (playing && playType == REC){
		playFromSource(NULL, "iii");
		}
}

int main(int argc, char *argv[])
{
	printf("%s \nPlease report any bug to %s\n", PACKAGE_STRING, PACKAGE_BUGREPORT);	
	
	playing = 0;
	fast = 0;
	pose = 0;
	intervalTimeout = 100;
	playlistPosition = 0;
	spect_bands = 512;
	volume = 0.8;
			
	int i = 0;
	gchar *filename;
	GtkWidget *pVBox[4], *pHBox[11], *pMenuBar, *pMenu, *pMenuItem, *pButtonSelect, *pRadio[3], *frame, *pViewPlaylist, *pScrollbar, *image, *widget;
	const GtkTargetEntry *targets;
	GtkTreeSelection  *selection;
	GError **error;

	tmpPath = g_build_filename (G_DIR_SEPARATOR_S, g_get_tmp_dir(), "record.flac", NULL);
	printf("tmpPath = %s\n", tmpPath); 
		
	initGstreamer();
	gtk_init (&argc, &argv); // create main window, vertical and horizontal boxes
	mainWindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_widget_set_size_request (mainWindow, 600, 460);
	gtk_window_set_resizable (GTK_WINDOW(mainWindow), TRUE);
	gtk_window_set_title(GTK_WINDOW(mainWindow), PACKAGE_NAME);
	filename = g_build_filename (G_DIR_SEPARATOR_S, DATADIR, "icons", "ptplayer.png", NULL);
	gtk_window_set_icon(GTK_WINDOW(mainWindow), gdk_pixbuf_new_from_file (filename, error));
	g_signal_connect (G_OBJECT (mainWindow), "destroy", G_CALLBACK (on_window_destroy), NULL);

	for (i = 0; i < 4; i++) {
		pVBox[i] = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
		}
	pHBox[0] = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	pHBox[1] = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	pHBox[4] = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	pHBox[5] = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	for (i = 6; i < 11; i++) {
		pHBox[i] = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
		}
		
	gtk_container_add(GTK_CONTAINER(mainWindow), pVBox[1]); 

	gtk_box_pack_start(GTK_BOX(pVBox[1]), pHBox[0], FALSE, TRUE, 0);
		
/* Menu */
	pMenuBar = gtk_menu_bar_new();

	pMenu = gtk_menu_new(); // 'Quit' submenu
	pMenuItem = gtk_menu_item_new_with_label("Quit");
	gtk_menu_shell_append(GTK_MENU_SHELL(pMenu), pMenuItem);    
	g_signal_connect(G_OBJECT(pMenuItem), "activate", G_CALLBACK(on_window_destroy), (GtkWidget*) mainWindow); 
	pMenuItem = gtk_menu_item_new_with_label("Quit");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(pMenuItem), pMenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(pMenuBar), pMenuItem);

	/*pMenu = gtk_menu_new(); // 'Edit' submenu
        pMenuItem = gtk_menu_item_new_with_label("Preferences");
	//g_signal_connect(G_OBJECT(pMenuItem), "activate", G_CALLBACK(onPreferences), (GtkWidget*) mainWindow);
	gtk_menu_shell_append(GTK_MENU_SHELL(pMenu), pMenuItem);
	pMenuItem = gtk_menu_item_new_with_label("Edit");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(pMenuItem), pMenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(pMenuBar), pMenuItem);*/

	pMenu = gtk_menu_new(); // 'View' submenu
        pMenuItem = gtk_menu_item_new_with_label("Equalizer & Spectrogram");
	g_signal_connect(G_OBJECT(pMenuItem), "activate", G_CALLBACK(effects_window), (GtkWidget*) mainWindow);
	gtk_menu_shell_append(GTK_MENU_SHELL(pMenu), pMenuItem);
	pMenuItem = gtk_menu_item_new_with_label("View");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(pMenuItem), pMenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(pMenuBar), pMenuItem);


	pMenu = gtk_menu_new(); // 'Help...' submenu
	/*pMenuItem = gtk_menu_item_new_with_label("Shortcuts"); 
	//g_signal_connect(G_OBJECT(pMenuItem), "activate", G_CALLBACK(onShortcuts), (GtkWidget*) mainWindow);
	gtk_menu_shell_append(GTK_MENU_SHELL(pMenu), pMenuItem);*/
	pMenuItem = gtk_menu_item_new_with_label("About...");
	g_signal_connect(G_OBJECT(pMenuItem), "activate", G_CALLBACK(onAbout), (GtkWidget*) mainWindow);
	gtk_menu_shell_append(GTK_MENU_SHELL(pMenu), pMenuItem);
	pMenuItem = gtk_menu_item_new_with_label("Help");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(pMenuItem), pMenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(pMenuBar), pMenuItem);

	gtk_box_pack_start(GTK_BOX(pHBox[0]), pMenuBar, FALSE, TRUE, 0);

/* "Previous Track" button */
	gtk_box_pack_start(GTK_BOX(pVBox[1]), pHBox[1], FALSE, FALSE, 5);
	widget = gtk_button_new();
	gtk_widget_set_tooltip_text (widget, "Go to previous track");
	filename = g_build_filename (G_DIR_SEPARATOR_S, DATADIR, "icons", "ptp-previous.png", NULL);
	image = gtk_image_new_from_file(filename);
	//image = gtk_image_new_from_stock (GTK_STOCK_MEDIA_PREVIOUS, GTK_ICON_SIZE_BUTTON);
	gtk_button_set_image(GTK_BUTTON(widget),image);
	gtk_box_pack_start(GTK_BOX(pHBox[1]), widget, FALSE, FALSE, 4);
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(previousTrack), NULL);
	
/* "Seek Backward" button */
	backwardButton = gtk_button_new();
	gtk_widget_set_tooltip_text (backwardButton, "Rewind");
	filename = g_build_filename (G_DIR_SEPARATOR_S, DATADIR, "icons", "ptp-rewind.png", NULL);
	image = gtk_image_new_from_file(filename);
	//image = gtk_image_new_from_stock (GTK_STOCK_MEDIA_REWIND, GTK_ICON_SIZE_BUTTON);
	gtk_button_set_image(GTK_BUTTON(backwardButton),image);
	gtk_box_pack_start(GTK_BOX(pHBox[1]), backwardButton, FALSE, FALSE, 4);
	g_signal_connect(G_OBJECT(backwardButton), "button-press-event", G_CALLBACK(seek), "-1");
	g_signal_connect(G_OBJECT(backwardButton), "button-release-event", G_CALLBACK(seek), "0");
	
/* "Play/Pause" button */
	pPlayButton = gtk_button_new();
	gtk_widget_set_size_request (pPlayButton, 50, 40);
	filename = g_build_filename (G_DIR_SEPARATOR_S, DATADIR, "icons", "ptp-play.png", NULL);
	GtkWidget *playImage = gtk_image_new_from_file(filename);
	//GtkWidget *playImage = gtk_image_new_from_stock (GTK_STOCK_MEDIA_PLAY, GTK_ICON_SIZE_LARGE_TOOLBAR);
	gtk_button_set_image(GTK_BUTTON(pPlayButton),playImage);
	gtk_box_pack_start(GTK_BOX(pHBox[1]), pPlayButton, TRUE, TRUE, 4);
	g_signal_connect(G_OBJECT(pPlayButton), "clicked", G_CALLBACK(playFromSource), "play");
	
/* Record button */
	recordButton = gtk_button_new();
	filename = g_build_filename (G_DIR_SEPARATOR_S, DATADIR, "icons", "ptp-record.png", NULL);
	image = gtk_image_new_from_file(filename);
	gtk_button_set_image(GTK_BUTTON(recordButton),image);
	gtk_widget_set_sensitive (recordButton, FALSE);
	gtk_box_pack_start(GTK_BOX(pHBox[1]), recordButton, FALSE, FALSE, 4);
	//g_signal_connect(G_OBJECT(recordButton), "clicked", G_CALLBACK(playFromSource), "rec");
	g_signal_connect(G_OBJECT(recordButton), "clicked", G_CALLBACK(start_rec), NULL);
			
/* "Stop" button */
	widget = gtk_button_new();
	filename = g_build_filename (G_DIR_SEPARATOR_S, DATADIR, "icons", "ptp-stop.png", NULL);
	image = gtk_image_new_from_file(filename);
	//image = gtk_image_new_from_stock (GTK_STOCK_MEDIA_STOP, GTK_ICON_SIZE_BUTTON);
	gtk_button_set_image(GTK_BUTTON(widget),image);
	gtk_box_pack_start(GTK_BOX(pHBox[1]), widget, FALSE, FALSE, 4);
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(onStop), NULL);

/* "Seek Forward" button */
	forwardButton = gtk_button_new();
	gtk_widget_set_tooltip_text (forwardButton, "Forward");
	filename = g_build_filename (G_DIR_SEPARATOR_S, DATADIR, "icons", "ptp-forward.png", NULL);
	image = gtk_image_new_from_file(filename);
	//image = gtk_image_new_from_stock (GTK_STOCK_MEDIA_FORWARD, GTK_ICON_SIZE_BUTTON);
	gtk_button_set_image(GTK_BUTTON(forwardButton),image);
	gtk_box_pack_start(GTK_BOX(pHBox[1]), forwardButton, FALSE, FALSE, 4);
	g_signal_connect(G_OBJECT(forwardButton), "button-press-event", G_CALLBACK(seek), "1");
	g_signal_connect(G_OBJECT(forwardButton), "button-release-event", G_CALLBACK(seek), "0");

/* "Next Track" button */
	widget = gtk_button_new(); 
	gtk_widget_set_tooltip_text (widget, "Go to next track");
	filename = g_build_filename (G_DIR_SEPARATOR_S, DATADIR, "icons", "ptp-next.png", NULL);
	image = gtk_image_new_from_file(filename);
	//image = gtk_image_new_from_stock (GTK_STOCK_MEDIA_NEXT, GTK_ICON_SIZE_BUTTON);
	gtk_button_set_image(GTK_BUTTON(widget),image);
	gtk_box_pack_start(GTK_BOX(pHBox[1]), widget, FALSE, FALSE, 4);
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(nextTrack), NULL);
 
/* "Select File" button */
	widget = gtk_button_new(); 
	gtk_widget_set_tooltip_text (widget, "Select file");
	filename = g_build_filename (G_DIR_SEPARATOR_S, DATADIR, "icons", "ptp-document-open.png", NULL);
	image = gtk_image_new_from_file(filename);
	//image = gtk_image_new_from_stock (GTK_STOCK_OPEN, GTK_ICON_SIZE_BUTTON);
	gtk_button_set_image(GTK_BUTTON(widget),image);
	gtk_box_pack_start(GTK_BOX(pHBox[1]), widget, FALSE, FALSE, 4);
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(select_file), NULL);

/* "Save as" button */
	widget = gtk_button_new(); 
	gtk_widget_set_tooltip_text (widget, "Save file as...");
	filename = g_build_filename (G_DIR_SEPARATOR_S, DATADIR, "icons", "ptp-document-save-as.png", NULL);
	image = gtk_image_new_from_file(filename);
	//image = gtk_image_new_from_stock (GTK_STOCK_SAVE_AS, GTK_ICON_SIZE_BUTTON);
	gtk_button_set_image(GTK_BUTTON(widget),image);
	gtk_box_pack_start(GTK_BOX(pHBox[1]), widget, FALSE, FALSE, 4);
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(save_file), NULL);


/* 'Activate recording' check button */
	gtk_container_add(GTK_CONTAINER(pHBox[1]), pVBox[2]); 
	filename = g_build_filename (G_DIR_SEPARATOR_S, DATADIR, "icons", "ptp-microphone.png", NULL);
	image = gtk_image_new_from_file(filename);
	widget = gtk_check_button_new();
	gtk_button_set_image (GTK_BUTTON(widget), image);
	gtk_widget_set_tooltip_text (widget, "Activate Recording mode");
	gtk_box_pack_start(GTK_BOX(pVBox[2]), widget, FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT(widget), "toggled", G_CALLBACK(activate_recording), NULL);

/* Drawing area */
	drawingareaLevel = gtk_drawing_area_new ();
	gtk_widget_set_size_request (drawingareaLevel, 120, 25);
	gtk_box_pack_start (GTK_BOX (pVBox[2]), drawingareaLevel, FALSE, FALSE, 0);

/* Volume button */
	gtk_container_add(GTK_CONTAINER(pHBox[1]), pVBox[3]); 
	widget = gtk_volume_button_new ();
	gtk_widget_set_tooltip_text (widget, "Change playback volume");
	gtk_box_pack_start(GTK_BOX(pVBox[3]), widget, FALSE, FALSE, 0);
	gtk_scale_button_set_value(GTK_SCALE_BUTTON(widget), 0.8);
	g_signal_connect(G_OBJECT(widget), "value-changed", G_CALLBACK(volume_changed), NULL);

/* Mute volume check box */
	widget = gtk_check_button_new();
	filename = g_build_filename (G_DIR_SEPARATOR_S, DATADIR, "icons", "ptp-speaker-mute.png", NULL);
	image = gtk_image_new_from_file(filename);
	gtk_button_set_image (GTK_BUTTON(widget), image);
	gtk_box_pack_start(GTK_BOX(pVBox[3]), widget, FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT(widget), "toggled", G_CALLBACK(check_mute), NULL);

/* Panorama scale */
	frame = gtk_frame_new("Balance");
	widget = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, -1, 1, 0.1);
	gtk_widget_set_tooltip_text (widget, "Change balance");
	gtk_scale_set_draw_value (GTK_SCALE(widget), TRUE);
	gtk_container_add(GTK_CONTAINER(frame), widget);
	gtk_box_pack_start(GTK_BOX(pHBox[1]), frame, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT(widget), "value-changed", G_CALLBACK(change_panorama), NULL);
	gtk_range_set_value (GTK_RANGE(widget), 0);

/* Time label */
	label=gtk_label_new("Time : ");
	gtk_box_pack_start(GTK_BOX(pHBox[4]), label, FALSE, FALSE, 0);

/* Time Scale */
	pScaleSeek = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, 0.0, 100.0, 1);
	gtk_scale_set_draw_value (GTK_SCALE (pScaleSeek), FALSE);
	//gtk_scale_set_value_pos (GTK_SCALE (pScaleSeek), GTK_POS_TOP);
	gtk_range_set_value (GTK_RANGE (pScaleSeek), 0);
	gtk_widget_set_size_request (pScaleSeek, 400, 20);
	gtk_box_pack_start(GTK_BOX(pHBox[4]), pScaleSeek, FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT(pScaleSeek), "value-changed", G_CALLBACK(on_seek), NULL);
	gtk_box_pack_start(GTK_BOX(pVBox[1]), pHBox[4], FALSE, FALSE, 0);

	label=gtk_label_new("           0:00 / 0:00           ");
	gtk_box_pack_start(GTK_BOX(pHBox[4]), label, FALSE, FALSE, 0);
	
/* Effects (equaliser and spectrogram) button */
	filename = g_build_filename (G_DIR_SEPARATOR_S, DATADIR, "icons", "ptp-equalizer.png", NULL);
	gtk_box_pack_start(GTK_BOX(pVBox[1]), pHBox[5], FALSE, FALSE, 3);
	widget = gtk_button_new();  
	gtk_widget_set_tooltip_text (widget, "Show spectrogram and equaliser");
	image = gtk_image_new_from_file(filename);
	gtk_button_set_image (GTK_BUTTON(widget), image);
	gtk_box_pack_start(GTK_BOX(pHBox[5]), widget, FALSE, FALSE, 3);
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(effects_window), NULL);

/* JACK check box */
	widget = gtk_check_button_new();
	filename = g_build_filename (G_DIR_SEPARATOR_S, DATADIR, "icons", "ptp-qjackctl.png", NULL);
	image = gtk_image_new_from_file(filename);
	gtk_button_set_image (GTK_BUTTON(widget), image);
	gtk_widget_set_tooltip_text (widget, "Use Jack-Audio-Connection-Kit");
	gtk_box_pack_start(GTK_BOX(pHBox[5]), widget, FALSE, FALSE, 3);
	g_signal_connect(G_OBJECT(widget), "toggled", G_CALLBACK(use_jack), NULL);

/* Change tempo */
	frame = gtk_frame_new("Change speed");
	widget = gtk_spin_button_new_with_range(0.1, 10, 0.05);
	gtk_widget_set_tooltip_text (widget, "Change tempo without changing the pitch");
	gtk_box_pack_start(GTK_BOX(pHBox[5]), frame, FALSE, FALSE, 3);
	gtk_container_add(GTK_CONTAINER(frame), widget);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(widget), 1.0); 
	g_signal_connect(G_OBJECT(widget), "value-changed", G_CALLBACK(change_tempo), NULL);

/* Change pitch */
	frame = gtk_frame_new("Change pitch");
	widget = gtk_spin_button_new_with_range(-24, 24, 1);
	gtk_widget_set_tooltip_text (widget, "Change pitch without changing the tempo");
	gtk_box_pack_start(GTK_BOX(pHBox[5]), frame, FALSE, FALSE, 3);
	gtk_container_add(GTK_CONTAINER(frame), widget);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(widget), 0); 
	g_signal_connect(G_OBJECT(widget), "value-changed", G_CALLBACK(change_pitch), NULL);

/* Change rate (pitch and tempo in a proportionnal way) */
	frame = gtk_frame_new("Change rate");
	widget = gtk_spin_button_new_with_range(0.1, 4, 0.01);
	gtk_widget_set_tooltip_text (widget, "Change tempo and pitch in a proportional way");
	gtk_box_pack_start(GTK_BOX(pHBox[5]), frame, FALSE, FALSE, 3);
	gtk_container_add(GTK_CONTAINER(frame), widget);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(widget), 1); 
	g_signal_connect(G_OBJECT(widget), "value-changed", G_CALLBACK(change_rate), NULL);

/* Adjust fine pitch */
	frame = gtk_frame_new("Adjust fine pitch");
	widget = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, -0.50, 0.50, 0.01);
	gtk_widget_set_tooltip_text (widget, "Adjust pitch; -0.50 is a half tone lower, +0.50 is a half tone higher");
	gtk_scale_set_draw_value (GTK_SCALE (widget), TRUE);
	gtk_scale_set_value_pos (GTK_SCALE (widget), GTK_POS_TOP);
	gtk_range_set_value (GTK_RANGE (widget), 0);
	//gtk_box_pack_start(GTK_BOX(vbox[0]), hbox[1], FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(pHBox[5]), frame, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(frame), widget);
	g_signal_connect(G_OBJECT(widget), "value-changed", G_CALLBACK(change_fine_pitch), NULL);

/* Events on main window */
	g_signal_connect (mainWindow, "key-press-event", G_CALLBACK (on_key_press_mainWindow), NULL);
	g_signal_connect (mainWindow, "key-release-event", G_CALLBACK (on_key_press_mainWindow), NULL);

/* Playlist */
	pListPlaylistStore = gtk_list_store_new(N_COLUMN, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	
	pListPlaylistView = gtk_tree_view_new_with_model(GTK_TREE_MODEL(pListPlaylistStore));
	gtk_tree_view_set_reorderable(GTK_TREE_VIEW(pListPlaylistView), TRUE);
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(pListPlaylistView));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);
			
	/* 1st column */
        pCellRenderer = gtk_cell_renderer_text_new();
    	pColumn = gtk_tree_view_column_new_with_attributes ("  ",
                                                   pCellRenderer,
                                                   "text", IS_PLAYING_COLUMN,
                                                   NULL);

    	gtk_tree_view_append_column(GTK_TREE_VIEW(pListPlaylistView), pColumn);

	/* 2d column */
	pCellRenderer = gtk_cell_renderer_text_new();
   	pColumn = gtk_tree_view_column_new_with_attributes ("TRACK",
                                                   pCellRenderer,
                                                   "text", TITLE_COLUMN,
                                                   NULL);
        //gtk_tree_view_append_column(GTK_TREE_VIEW(pListPlaylistView), pColumn);

	/* 3d column */
	pCellRenderer = gtk_cell_renderer_text_new();
   	pColumn = gtk_tree_view_column_new_with_attributes ("NAME",
                                                   pCellRenderer,
                                                   "text", NAME_COLUMN,
                                                   NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(pListPlaylistView), pColumn);


	pScrollbar = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(pScrollbar),
		GTK_POLICY_AUTOMATIC,
		GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(pScrollbar), pListPlaylistView);
	
	gtk_box_pack_start(GTK_BOX (pVBox[1]), pHBox[10], TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(pHBox[10]), pScrollbar);
	gtk_box_set_child_packing (GTK_BOX (pHBox[10]), pScrollbar, TRUE, TRUE, 0, GTK_PACK_START);


	gtk_widget_set_size_request(pHBox[10], 550, 300);
	//gtk_tree_view_enable_model_drag_dest (GTK_TREE_VIEW(pListPlaylistView), targets, 3, GDK_ACTION_LINK);
	if (1){
		enum
		{
		TARGET_STRING,
		TARGET_URL,
		TARGET_ROOTWIN
		};

		static GtkTargetEntry targetentries[] =
		{
		{ "text/uri-list", 0, TARGET_URL },
		{ "application/x-rootwindow-drop", 0, TARGET_ROOTWIN }
		};

		gtk_tree_view_enable_model_drag_dest (GTK_TREE_VIEW(pListPlaylistView), targetentries, G_N_ELEMENTS(targetentries), GDK_ACTION_COPY | GDK_ACTION_MOVE | GDK_ACTION_LINK);
		gtk_tree_view_enable_model_drag_source (GTK_TREE_VIEW(pListPlaylistView), GDK_BUTTON1_MASK, targetentries, G_N_ELEMENTS(targetentries), GDK_ACTION_MOVE);
		}
		
		g_signal_connect(pListPlaylistView, "drag_data_received",
                     G_CALLBACK(view_onDragDataReceived), pListPlaylistStore);
		g_signal_connect(pListPlaylistView,"drag-data-get",
                	G_CALLBACK(on_drag_data_get),NULL);
		g_signal_connect(gtk_tree_view_get_selection 
			(GTK_TREE_VIEW(pListPlaylistView)),   "changed", 
				G_CALLBACK(on_selection_changed),NULL);
		g_signal_connect (pListPlaylistView, "key-press-event",
			  G_CALLBACK (on_key_press), NULL);
		g_signal_connect(pListPlaylistView, "button_press_event",
                     G_CALLBACK(click_event), NULL);
								
	gtk_widget_show_all (mainWindow);
	gtk_main ();

	return 0;
}


