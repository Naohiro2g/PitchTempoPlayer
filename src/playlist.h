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

#ifndef DEFINE_PLAYLIST
#define DEFINE_PLAYLIST

typedef struct PreviousValue PreviousValue;
struct PreviousValue
{
	GList *isPlayingList;	// the column where is the arrow, that shows which file is beeing played or will be played;
	GList *titleList;	// the complete path of the file;
	GList *nameList;	// the 'basename' (Glib), i.e. the last part of the name;
};


gboolean DOUBLE_CLICK;
char *sFile;
int AUDIOFREQ, interval, pose, playing, typeSource, range, nSec, intervalTimeout;
gint playlistSize, playlistStoreSize, playlistPosition;
gdouble speed;
gfloat tempo;

GList *playlist;
GMainLoop *loop;
static GtkWidget *mainWindow;
GtkListStore *pListPlaylistStore; 

void getFileName();
void updatePlaylistView();
void updateGListFromTree();
void removeRows();
gboolean updatePlaylistPosition (GtkTreeModel *model, GtkTreePath  *path, GtkTreeIter  *iter, gpointer user_data);
void update_array_playlist();
void get_playlist_store_size ();
void onStop();
void onPlay();
void previousTrack();
void nextTrack();
void getPlaylistSize();
void playFromSource();
void onSource(GtkWidget *pBtn, gpointer data);

void change_tempo (GtkSpinButton *spinbutton, gpointer data);
void change_pitch (GtkSpinButton *spinbutton, gpointer data);

void onPreferences(GtkWidget* widget, gpointer data);
void saveChange(GtkWidget* widget, gpointer data);
void onAbout(GtkWidget* widget, gpointer data);
void onDisplay(GtkWidget *pBtn, gpointer data);
void onShortcuts (GtkWidget* widget, gpointer data);
void onGesturesShortcuts (GtkWidget* widget, gpointer data);
void errorMessageWindow(char *message);

#endif



