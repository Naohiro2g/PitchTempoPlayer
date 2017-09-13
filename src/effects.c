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

#include "effects.h"

#define NBANDS 10

GtkWidget *drawingarea = NULL;
GtkWidget *widget[10];
guint spect_height = 128;
GObject *band;
gdouble freq;
gdouble bw;
gdouble gain;

/* Quit main window and eveything */
static void on_window_destroy (GObject * object, GtkWidget *effectsWindow)
{	
	showEffectsWindow = FALSE;
	gtk_widget_destroy(effectsWindow);
}
	
static gboolean
on_configure_event (GtkWidget * widget, GdkEventConfigure * event,
    gpointer user_data)
{
	GST_INFO ("%d x %d", event->width, event->height); 
	spect_height = event->height;
	height_scale = event->height / 64.0;
	spect_bands = event->width;
	if (playing){
		g_object_set (G_OBJECT (spectrum), "bands", spect_bands, NULL);
		}
  return FALSE;
}

void
draw_spectrum (gfloat * data)
{
	gint i;
	GdkRectangle rect = { 0, 0, spect_bands, spect_height };
	cairo_t *cr;

  if (!drawingarea)
    return;

  gdk_window_begin_paint_rect (gtk_widget_get_window (drawingarea), &rect);

  cr = gdk_cairo_create (gtk_widget_get_window (drawingarea));
	
  cairo_set_source_rgb (cr, 0, 0, 0);
  cairo_rectangle (cr, 0, 0, spect_bands, spect_height);
  cairo_fill (cr);
	
  for (i = 0; i < spect_bands; i++) {
    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_rectangle (cr, i, -data[i], 1, spect_height + data[i]);
    cairo_fill (cr);
  }
	
  cairo_destroy (cr);

  gdk_window_end_paint (gtk_widget_get_window (drawingarea));
}

static void
on_gain_changed (GtkRange * range, gpointer user_data)
{
	//printf("gain changed\n");
  GstObject *band = GST_OBJECT (user_data);
  gdouble value = gtk_range_get_value (range);

  g_object_set (band, "gain", value, NULL);
}

static void
on_bandwidth_changed (GtkRange * range, gpointer user_data)
{
  GstObject *band = GST_OBJECT (user_data);
  gdouble value = gtk_range_get_value (range);

  g_object_set (band, "bandwidth", value, NULL);
}

/* control frequency */
static void
on_freq_changed (GtkRange * range, gpointer user_data)
{
  GstObject *band = GST_OBJECT (user_data);
  gdouble value = gtk_range_get_value (range);

  /* hbox */
  GtkWidget *parent = gtk_widget_get_parent (GTK_WIDGET (range));

  /* frame */
  GtkWidget *parent_parent = gtk_widget_get_parent (parent);
  gchar *label = g_strdup_printf ("%d Hz", (int) (value + 0.5));

  gtk_frame_set_label (GTK_FRAME (parent_parent), label);
  g_free (label);

  g_object_set (band, "freq", value, NULL);
}

void getBand(){
	int i = 0;
	for (i = 0; i < NBANDS; i++) {
		band = gst_child_proxy_get_child_by_index (GST_CHILD_PROXY (equalizer), i);
		//g_assert (band != NULL);
		//g_object_get (G_OBJECT (band), "freq", &freq, NULL);
		//g_object_get (G_OBJECT (band), "bandwidth", &bw, NULL);
		//g_object_get (G_OBJECT (band), "gain", &gain, NULL);
		gain = gtk_range_get_value (GTK_RANGE (widget[i]));
		on_gain_changed(GTK_RANGE (widget[i]), (gpointer) band);
		g_signal_connect (G_OBJECT (widget[i]), "value-changed", G_CALLBACK (on_gain_changed), (gpointer) band);
		}
}

void reset_equalizer(){
	int i = 0;
	for (i = 0; i < NBANDS; i++) {
		gtk_range_set_value (GTK_RANGE (widget[i]), 0);
		}

}


void effects_window()
{
if (showEffectsWindow == FALSE){
	int i = 0;
	GtkWidget *vbox[4], *hbox[11], *button;

	height_scale = 2.0;
			
	effectsWindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gchar *title = g_strdup_printf (" %s : effects", PACKAGE_NAME);
	gtk_window_set_title(GTK_WINDOW(effectsWindow), title);
	g_signal_connect (G_OBJECT (effectsWindow), "destroy", G_CALLBACK (on_window_destroy), effectsWindow);
	vbox[0] = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
	hbox[0] = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	hbox[1] = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	hbox[2] = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);			

	gtk_container_add(GTK_CONTAINER(effectsWindow), vbox[0]);
	gtk_box_pack_start(GTK_BOX(vbox[0]), hbox[0], TRUE, TRUE, 0);

	drawingarea = gtk_drawing_area_new ();
	gtk_widget_set_size_request (drawingarea, spect_bands, spect_height);
	g_signal_connect (G_OBJECT (drawingarea), "configure-event", G_CALLBACK (on_configure_event), (gpointer) spectrum);
	gtk_box_pack_start (GTK_BOX (hbox[0]), drawingarea, TRUE, TRUE, 0);
	
	for (i = 0; i < NBANDS; i++) {
			
		//GstObject *band;
		//gdouble freq;
		//gdouble bw;
		//gdouble gain;
		gchar *label;
		GtkWidget *frame, *scales_hbox;

		band = gst_child_proxy_get_child_by_index (GST_CHILD_PROXY (equalizer), i);
		g_assert (band != NULL);
		g_object_get (G_OBJECT (band), "freq", &freq, NULL);
		//g_object_get (G_OBJECT (band), "bandwidth", &bw, NULL);
		g_object_get (G_OBJECT (band), "gain", &gain, NULL);

		label = g_strdup_printf ("%d Hz", (int) (freq + 0.5));
		frame = gtk_frame_new (label);
		g_free (label);

		scales_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);

		widget[i] = gtk_scale_new_with_range (GTK_ORIENTATION_VERTICAL, -24.0, 12.0, 0.5);
		gtk_scale_set_draw_value (GTK_SCALE (widget[i]), TRUE);
		gtk_scale_set_value_pos (GTK_SCALE (widget[i]), GTK_POS_TOP);
		gtk_range_set_inverted (GTK_RANGE (widget[i]), TRUE);
		gtk_range_set_value (GTK_RANGE (widget[i]), gain);
		gtk_widget_set_size_request (widget[i], 25, 150);
		g_signal_connect (G_OBJECT (widget[i]), "value-changed", G_CALLBACK (on_gain_changed), (gpointer) band);
		gtk_box_pack_start (GTK_BOX (scales_hbox), widget[i], TRUE, TRUE, 0);

		/*widget = gtk_vscale_new_with_range (0.0, 20000.0, 5.0);
		gtk_scale_set_draw_value (GTK_SCALE (widget), TRUE);
		gtk_scale_set_value_pos (GTK_SCALE (widget), GTK_POS_TOP);
		gtk_range_set_value (GTK_RANGE (widget), bw);
		gtk_widget_set_size_request (widget, 25, 150);
		g_signal_connect (G_OBJECT (widget), "value-changed",
		G_CALLBACK (on_bandwidth_changed), (gpointer) band);
		gtk_box_pack_start (GTK_BOX (scales_hbox), widget, TRUE, TRUE, 0);

		widget = gtk_vscale_new_with_range (20.0, 20000.0, 5.0);
		gtk_scale_set_draw_value (GTK_SCALE (widget), TRUE);
		gtk_scale_set_value_pos (GTK_SCALE (widget), GTK_POS_TOP);
		gtk_range_set_value (GTK_RANGE (widget), freq);
		gtk_widget_set_size_request (widget, 25, 150);
		g_signal_connect (G_OBJECT (widget), "value-changed",
		G_CALLBACK (on_freq_changed), (gpointer) band);
		gtk_box_pack_start (GTK_BOX (scales_hbox), widget, TRUE, TRUE, 0);*/
		
		gtk_container_add (GTK_CONTAINER (frame), scales_hbox);

		gtk_box_pack_start (GTK_BOX (hbox[1]), frame, TRUE, TRUE, 0);
  }

  gtk_box_pack_start (GTK_BOX (vbox[0]), hbox[1], TRUE, TRUE, 0);
	
	button = gtk_button_new_with_label("Reset\nequalizer");
	gtk_box_pack_start (GTK_BOX (hbox[2]), button, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox[0]), hbox[2], FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(reset_equalizer), NULL);
	
	showEffectsWindow = TRUE;
  	gtk_widget_show_all (effectsWindow);
}
else {
	gtk_widget_destroy(effectsWindow);
	}
}



