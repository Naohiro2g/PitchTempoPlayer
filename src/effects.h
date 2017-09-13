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

#ifndef DEFINE_EFFECTS
#define DEFINE_EFFECTS

gboolean showEffectsWindow;
int playing;
guint spect_bands;
gfloat height_scale;
GtkWidget *effectsWindow;
GstElement *spectrum, *equalizer;

#endif



