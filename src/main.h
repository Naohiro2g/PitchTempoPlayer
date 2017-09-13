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

#ifndef DEFINE_MAIN
#define DEFINE_MAIN

gboolean act_rec;
char *sFile;
gchar *tmpPath;
int AUDIOFREQ, pose, playing, fast, intervalTimeout, redo;
gint playlistSize;
guint playlistPosition, spect_bands;
gdouble volume;
gfloat height_scale;

GList *playlist;
GtkWidget *pScaleSeek, *recordButton, *drawingareaLevel;
static GtkWidget *mainWindow;
GtkListStore *pListPlaylistStore;
GMainLoop *loop;

typedef enum PlayType PlayType;
enum PlayType
{
	NONE, PLAY, REC, SAVE, ACT_REC, PLAY_REC
};
PlayType playType;

void initGstreamer();
void getFileName();
void select_file(GtkWidget *pWidget, gpointer data);
void updatePlaylistView();
void on_selection_changed (GtkTreeSelection *treeselection, gpointer user_data);
void on_drag_data_get (GtkWidget *widget, GdkDragContext *drag_context, GtkSelectionData *sdata, guint info, guint time, gpointer user_data);
void view_onDragDataReceived(GtkWidget *wgt, GdkDragContext *context, int x, int y, GtkSelectionData *seldata, guint info, guint time, gpointer userdata);
gboolean on_key_press (GtkWidget * window, GdkEventKey*	pKey, gpointer data);
gboolean on_key_press_mainWindow (GtkWidget *window, GdkEventKey *event, gpointer data);
void  on_row_changed (GtkTreeModel *tree_model, GtkTreePath  *path, GtkTreeIter  *iter, gpointer user_data);
gint click_event(GtkWidget *widget, GdkEventButton *event, gpointer func_data);
void removeRows();
void onStop();
void previousTrack();
void nextTrack();
void on_seek (GtkRange *range, gchar *data);
void save_file ();
void activate_recording(GtkWidget *pToggle, gpointer data);
void getPlaylistSize();
void playFromSource(GtkWidget *pWidget, gpointer signal);
void onSource(GtkWidget *pBtn, gpointer data);
void seek (GtkButton *button, GdkEvent *event, gpointer *data);
gboolean seek_to_time ();

void change_tempo (GtkSpinButton *spinbutton, gpointer data);
void change_pitch (GtkSpinButton *spinbutton, gpointer data);
void change_fine_pitch (GtkRange *range, gpointer data);
void change_rate (GtkSpinButton *spinbutton, gpointer data);
void check_mute(GtkWidget *pWidget, gpointer pToggle);
void volume_changed (GtkRange *range, gpointer user_data);
void change_panorama (GtkScaleButton *button, gdouble value, gpointer data);
void use_jack(GtkWidget *pToggle, gpointer data);

void saveChange(GtkWidget* widget, gpointer data);
void onAbout(GtkWidget* widget, gpointer data);
void onDisplay(GtkWidget *pBtn, gpointer data);
void onShortcuts (GtkWidget* widget, gpointer data);
void onGesturesShortcuts (GtkWidget* widget, gpointer data);
void errorMessageWindow(char *message);

void effects_window();

#endif



