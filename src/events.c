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
#include <gdk/gdkkeysyms.h>

#include "config.h"

#include "events.h"

/* Manage key press events with GDK */  
gboolean on_key_press_mainWindow (GtkWidget *window, GdkEventKey *event, gpointer data){
	
	if (event->type == GDK_KEY_PRESS){
		  switch (event->keyval){
			case GDK_KEY_Escape :
				onStop();
				break;
			case GDK_KEY_Left :
				fast = -1;
				break;
			case GDK_KEY_Right :
				fast = 1;
				break;
			case GDK_KEY_space :
				gtk_button_clicked (GTK_BUTTON(pPlayButton));
				break;	
		   }
		}
	if (event->type == GDK_KEY_RELEASE){
		  switch (event->keyval){
			case GDK_KEY_Left :
				fast = 0;
				break;
			case GDK_KEY_Right :
				fast = 0;
				break;	
		   }
		}

	return TRUE;  // return TRUE instead of FALSE allows default key bindings not to be aplied to the GtkScale widgets used in the GUI
}



