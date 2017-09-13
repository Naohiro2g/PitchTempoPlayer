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

#ifndef DEFINE_ONCLICK
#define DEFINE_ONCLICK

gboolean updateScaleSeek, JACK, act_rec, stop;
int playing, pose, fast, nSec, scale, intervalTimeout;
guint displaySpectroTimeout, playlistPosition;
gint64 pos, len;
gint playlistStoreSize;
gdouble volume;

typedef enum PlayType PlayType;
enum PlayType
{
	NONE, PLAY, REC, SAVE, ACT_REC, PLAY_REC
};
PlayType playType;

GMainLoop *loop;
GtkWidget *pScaleSeek;
GstElement *pipeline, *playbin, *audiopanorama;
void on_seek (GtkRange *range, gchar *data);
void showPosition(gchar *positionLabel);
void setPlayButtonIcon ();
void getFileName();

#endif
