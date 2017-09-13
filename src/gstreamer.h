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

#ifndef DEF_MAINWINDOW
#define DEF_MAINWINDOW

gboolean showEffectsWindow, updateScaleSeek, JACK, act_rec, stop, stop_loop; 
char *sFile;
gchar *uri, file[200], *encodePath, *tmpPath;
GList *playlist;
guint displaySpectroTimeout, playlistPosition;
guint spect_bands;
gint playlistSize, playlistStoreSize;
gint64 pos, len;
int AUDIOFREQ, pose, playing, typeSource, range, fast, nSec, intervalTimeout, redo;
gfloat height_scale;
gdouble volume;

GstElement *spectrum, *pipeline, *playbin, *equalizer, *audiopanorama;
GMainLoop *loop;
GtkWidget *effectsWindow, *pScaleSeek, *pPlayButton, *playRecButton, *progressWindow;

typedef enum PlayType PlayType;
enum PlayType
{
	NONE,		// no play type; 
	PLAY, 		// play audio file;
	REC, 		// record audio file; playType must be ACT_REC to be changed to REC; when recording is finished, playType is changed to ACT_RECT;
	SAVE,		// save the charged audio file with the currently defined characteristics of pitch and tempo
	ACT_REC,	// microphone input without recording; allows vu-meter to show level;
	PLAY_REC	// play the recorded file, that is located in the /tmp directory;
};
PlayType playType;


void getFileName();
void on_seek (GtkRange *range, gchar *data);
void errorMessageWindow(char *message);
gboolean message_handler (GstBus * bus, GstMessage * message, gpointer data);
GstTaskPool *
test_rt_pool_new ();
void getBand();
void setPlayButtonIcon();
void showPosition(gchar *positionLabel);
void draw_level(gdouble *rms, gint channels);
void playFromSource(GtkWidget *pWidget, gpointer signal);
void draw_spectrum (gfloat * data);
void errorMessageWindowJack(char *message);
void reset_view();

#endif



