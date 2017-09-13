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
#include <math.h>
#include <gtk/gtk.h>
#include <gst/gst.h>
#include <jack/jack.h>

#include "config.h"
#include "gstreamer.h"

#define NBANDS 10

gboolean firstPlay = TRUE;
gfloat pitchValue = 1, rate = 1, tempo = 1, adjustedPitchValue;
GstElement *audio, *pitch;

static GstTaskPool *pool;
static gdouble period = 0.0;

void initGstreamer(){ 
	stop = 0;
	gst_init (NULL, NULL);	
	equalizer = gst_element_factory_make ("equalizer-nbands", "NULL");
	g_assert (equalizer);
	g_object_set (G_OBJECT (equalizer), "num-bands", NBANDS, NULL);
	pitch = gst_element_factory_make ("pitch", NULL);
	g_assert (pitch);
}

/* Gstreamer message handler */
gboolean message_handler (GstBus * bus, GstMessage * message, gpointer data)
{	
	if (message->type == GST_MESSAGE_ELEMENT) {
		const GstStructure *s = gst_message_get_structure (message);
		const gchar *name = gst_structure_get_name (s);

		if (strcmp (name, "spectrum") == 0) {
			gfloat *spect = g_new (gfloat, spect_bands);
			const GValue *list;
			const GValue *value;
			guint i;

			list = gst_structure_get_value (s, "magnitude");
			for (i = 0; i < spect_bands; ++i) {
				value = gst_value_list_get_value (list, i);
				spect[i] = height_scale * g_value_get_float (value);
				}
			if (showEffectsWindow){
				draw_spectrum (spect);
				g_free (spect);
				}
			}
		}
	  if (message->type == GST_MESSAGE_ELEMENT) {
    const GstStructure *s = gst_message_get_structure (message);
    const gchar *name = gst_structure_get_name (s);

    if (strcmp (name, "level") == 0) {
      guint channels;
      GstClockTime endtime;
      gdouble rms_dB, peak_dB, decay_dB;
      const GValue *list;
      const GValue *value;

      gint i;

      if (!gst_structure_get_clock_time (s, "endtime", &endtime))
        g_warning ("Could not parse endtime");
      /* we can get the number of channels as the length of any of the value
       * lists */
	gdouble *rms = g_new (gdouble, 2);
      list = gst_structure_get_value (s, "rms");
      channels = gst_value_list_get_size (list);

      //g_print ("endtime: %" GST_TIME_FORMAT ", channels: %d\n", GST_TIME_ARGS (endtime), channels);
      for (i = 0; i < channels; ++i) {
        //g_print ("channel %d\n", i);
        list = gst_structure_get_value (s, "rms");
        value = gst_value_list_get_value (list, i);
        rms_dB = g_value_get_double (value);

        /* converting from dB to normal gives us a value between 0.0 and 1.0 */
        rms[i] = pow (10, rms_dB / 20);
        //g_print ("    normalized rms value: %f\n", rms);
	}
	draw_level(rms, channels);
    }
  }
  
	if (message->type == GST_MESSAGE_EOS) {
		printf("EOS\n");
		g_main_loop_quit (loop);
		}
return TRUE;
}

/* Print the total duration and the time position in a terminal when playing an audio file */
static gboolean cb_print_position (GstElement *pipeline)
{
	gchar positionLabel[50];
	gchar shortPos[7], shortLen[7];
	GstFormat fmt = GST_FORMAT_TIME;
	if (gst_element_query_position (pipeline, fmt, &pos)
	&& gst_element_query_duration (pipeline, fmt, &len)) {
		//g_print ("Time: %" GST_TIME_FORMAT " / %" GST_TIME_FORMAT "\r",
		//GST_TIME_ARGS (pos), GST_TIME_ARGS (len));
		gdouble setPos = ((gdouble)pos)/((gdouble)len) * 100;
		//printf("setPos = %f\n", setPos);
		/* pScaleSeek has to be updated with the new time value, but the callback (that updates the time values when pScaleSeek has been changed) doesn't have to be activated; the callback to the 'on_seek' function while moving pScaleSeek has to be blocked */
		g_signal_handlers_block_by_func(G_OBJECT(pScaleSeek), G_CALLBACK(on_seek), NULL);
		gtk_range_set_value(GTK_RANGE(pScaleSeek), setPos);
		g_signal_handlers_unblock_by_func(G_OBJECT(pScaleSeek), G_CALLBACK(on_seek), NULL);
		
		/* Print the time in stringPos and stringLen variables, then make them shorter (cut the '.' and then 10 numbers behind the dot) */
		gchar stringPos[45];
		g_snprintf(stringPos, sizeof(stringPos), "%" GST_TIME_FORMAT "\r", GST_TIME_ARGS (pos));
		stringPos[strlen(stringPos)-11] = 0;
		
		gchar stringLen[45];
		g_snprintf(stringLen, sizeof(stringLen), "%" GST_TIME_FORMAT "\r", GST_TIME_ARGS (len));
		stringLen[strlen(stringLen)-11] = 0;
		
		/* Print the result in the main window */
		g_snprintf(positionLabel, sizeof(positionLabel), "      %s / %s      ", stringPos, stringLen);
		showPosition(positionLabel);
		}
	
	return TRUE;			
}

/* Print the total duration and the time position in a terminal when playing an audio file */
gboolean update_progress_bar(GtkWidget *progress) {
	GstFormat fmt = GST_FORMAT_TIME;
	if (gst_element_query_position (pipeline, fmt, &pos)
	&& gst_element_query_duration (pipeline, fmt, &len)) {
		gdouble setPos = ((gdouble)pos)/((gdouble)len);
		printf("setPos = %f\n", setPos);
		if (setPos >= 0 && setPos <= 1) {
			gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress), setPos);
			}
		}
	if (playing && playType == SAVE){	
		return TRUE;	
		}
	else {
		gtk_widget_destroy(progressWindow);
		return FALSE;	
		} 	
}

/* Seek backward or forward when playing an audio file */
gboolean seek_to_time () {

if (fast == 1 || fast == -1) { 
		nSec = fast * 5;
		if (!gst_element_seek (playbin, 1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH,
		GST_SEEK_TYPE_SET, nSec * GST_SECOND + pos,
		GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE)) {
		g_print ("Seek failed!\n");
		}
	}
return TRUE;
}

/* Change tempo without changing the pitch */
void change_tempo (GtkSpinButton *spinbutton, gpointer data)
{
	tempo = (gfloat)(gtk_spin_button_get_value(spinbutton));
	g_object_set(G_OBJECT (pitch), "tempo", tempo, NULL);
}

/* Change pitch without changing the tempo */
void change_pitch (GtkSpinButton *spinbutton, gpointer data) {
	gdouble value = gtk_spin_button_get_value(spinbutton);
	pitchValue = (gfloat)(pow((double)1.0594630943592953, (double)value));
	g_object_set(G_OBJECT (pitch), "pitch", pitchValue, NULL);
 }

/* Change rate */
void change_rate (GtkSpinButton *spinbutton, gpointer data) {
	rate  = (gfloat)(gtk_spin_button_get_value(spinbutton));
	//rate = (gfloat)(pow((double)1.0594630943592953, (double)value));
	g_object_set(G_OBJECT (pitch), "rate", rate, NULL);
 }

/* Adjust pitch; every step is 1/100th of a tone */
void change_fine_pitch (GtkRange *range, gpointer data) {
	gdouble value = gtk_range_get_value (range);
	adjustedPitchValue = pitchValue * ((gfloat)1 + ((gfloat)0.0594630943592953 * (gfloat)value));
	g_object_set(G_OBJECT (pitch), "pitch", adjustedPitchValue, NULL);
 }

/* Chef if Jack server is working */
int checkJackActive(){
	printf("*** Checking if JACK is running (Jack error messages are normal):\n");
	jack_client_t *src_client;
	jack_status_t status;
	src_client = jack_client_open ("src_client", JackNoStartServer, &status); 
	if (src_client != NULL) {
		printf("JACK seems to be running.\n");
		return 1;
		}
	else if (src_client == NULL) {
		printf("JACK is not running\n");
		return 0;
		}
}

void getFileUri(){
	sprintf(file, "file://%s", sFile); 
	uri = g_strdup (file);
	}

void playFromSource(GtkWidget *pWidget, gpointer signal)
{
	//printf("VERY BEGINING : playType = %d, redo = %d, stop = %d, pose = %d\n", playType, redo, stop, pose);
	int a = 0;
	if (strcmp((gchar*)signal, "play") == 0){
		if (act_rec){
			if (playType == ACT_REC){
				if (loop != NULL){
					printf("play, act_rec, loop != NULL\n"); 
// redo = 1 prevents playType to be automatically reset to ACT_REC if act_rec is TRUE
					//stop_loop = TRUE;
					playType = PLAY_REC;
					redo = 1; 
					g_main_loop_quit(loop);
					}
				}
			else if (playType == PLAY_REC){
				}			
			}
		else if (act_rec == FALSE){
			playType = PLAY;
			}
		}
	else if (strcmp((gchar*)signal, "save") == 0){
		if (act_rec){
			if (playType == ACT_REC){		
				if (loop != NULL){
					playType = SAVE;
					redo = 1;
					g_main_loop_quit(loop);
					}
				}
			}
		else if (act_rec == FALSE && playing == 0){
			playType = SAVE;
			}
		}

	if (playType == PLAY && playlistStoreSize < 1) {
		playing = 2;
		playType = NONE;
		}
	//printf(" main : playing = %d\n", playing);
	if (stop_loop){
		printf("Stop loop\n");
		}
	/* ERROR */
	else if (playing == 2){
		printf("ERROR : Playing is not possible\n");
		errorMessageWindowJack("Playing is not possible");
		playing = 0;
		}
	/* PAUSE */
	else if (playing == 1 && playType != ACT_REC){
			if (pose == 0) {
				pose = 1;
				//printf("pose = oui\n");
				setPlayButtonIcon();
				gst_element_set_state (pipeline, GST_STATE_PAUSED);
				}
			 else {
				pose = 0;
				//printf("pose = non\n");
				setPlayButtonIcon();
				gst_element_set_state (pipeline, GST_STATE_PLAYING);
				}
	    	}

	/* PLAY */
	else if (playing == 0) {
		guint timeoutChangeSpeed, timeoutPrintPosition, timeoutSeekToTime;
		gint i;
		GstElement *src, *capsfilter, *audioconvert1, *audioconvert2, *audioconvert3, *alsasink, *audioresample, *sink, *enc, *level;
		GstBus *bus;
		GstStateChangeReturn ret;
		GstCaps *caps;
		GstPad *audiopad;

		jack_client_t *ptplayer_src, *ptplayer_sink;
		jack_status_t status;	

		playing = 1; pose = 0; stop = FALSE;
		AUDIOFREQ = 44100;
		//sprintf(tmpPath, "%s/record.flac", g_get_tmp_dir());
	gst_init (NULL, NULL);

	do {
		printf("LOOP BEGINS : playType = %d, redo = %d, stop = %d, pose = %d\n", playType, redo, stop, pose);
		int jackActive = 0;
		if (JACK == FALSE){
			jackActive = checkJackActive();
			}
		    if (jackActive){
				errorMessageWindowJack("JACK seems to be running\nStop JACK server and start playing again");
			return;
				}
		    else {
			redo = 0; playing = 1;
			setPlayButtonIcon();
			/* Create main loop */
			loop = g_main_loop_new(NULL, FALSE);
			/* create pipeline */
	  		if (playType == ACT_REC || playType == REC) {
				pipeline = gst_pipeline_new ("pipeline");
				g_assert (pipeline);
				}
			if (playType == PLAY || playType == PLAY_REC || playType == SAVE) {
				pipeline = gst_bin_new ("pipeline");
				g_assert (pipeline);
				}
			/* create jack clients if JACK is set to TRUE*/
			if (JACK){
				ptplayer_src = jack_client_open ("ptplayer", JackNoStartServer, &status);
				  if (ptplayer_src == NULL) {
				    if (status & JackServerFailed){
				        g_print ("JACK server not running\n");
					errorMessageWindowJack("JACK server not running");
					return;
					}
				    else{
					gchar msg[200];
					g_snprintf(msg, sizeof(msg), "jack_client_open() failed, status = 0x%2.0x\n", status);
				        g_print ("%s", msg);
					errorMessageWindowJack(msg);
					return;
					}
				    }
				ptplayer_sink = jack_client_open ("ptplayer", JackNoStartServer, &status);
				  if (ptplayer_sink == NULL) {
				    if (status & JackServerFailed){
				        g_print ("JACK server not running\n");
					errorMessageWindowJack("JACK server not running");
					return;
					}
				    else{
					gchar msg[200];
					g_snprintf(msg, sizeof(msg), "jack_client_open() failed, status = 0x%2.0x\n", status);
				        g_print ("%s", msg);
					errorMessageWindowJack(msg);
					return;
					}
				    }
				}
			/* create source element */
			if (playType == ACT_REC || playType == REC) {
				if (JACK){
					src = gst_element_factory_make ("jackaudiosrc", NULL);
					g_object_set (src, "client", ptplayer_src, NULL);
					g_assert (src);
					}
				else {
					src = gst_element_factory_make ("alsasrc", NULL);
					g_assert (src);
					}
				}
			else if (playType == PLAY || playType == PLAY_REC || playType == SAVE) {
				playbin = gst_element_factory_make ("playbin", NULL);
				g_assert (playbin);
				if (playType == PLAY_REC || (playType == SAVE && act_rec)){
					char uriTmpPath[45];
					sprintf(uriTmpPath, "file://%s", tmpPath);
					g_object_set (G_OBJECT (playbin), "volume", volume, "uri", uriTmpPath, NULL);
					}
				else if (playType == PLAY || (playType == SAVE && act_rec == FALSE)){
					getFileUri();
					g_object_set (G_OBJECT (playbin), "volume", volume, "uri", uri, NULL);
					}
				}
			/* Create pitch, spectrum, equalizer, audioconvert, level, flacenc and audiopanorama elements */
			pitch = gst_element_factory_make ("pitch", NULL);
			g_assert (pitch);
			g_object_set(G_OBJECT (pitch), "pitch", pitchValue, "tempo", tempo, "rate", rate, NULL);
			
			spectrum = gst_element_factory_make ("spectrum", NULL);
			g_assert (spectrum);
			g_object_set (G_OBJECT (spectrum), "bands", spect_bands, "threshold", -80, "post-messages", TRUE, "interval", 500 * GST_MSECOND, NULL);

			equalizer = gst_element_factory_make ("equalizer-nbands", NULL);
			g_assert (equalizer);
			g_object_set (G_OBJECT (equalizer), "num-bands", NBANDS, NULL);
			if (showEffectsWindow){
				getBand();
				}

			audioconvert1 = gst_element_factory_make ("audioconvert", NULL);
			g_assert (audioconvert1);
			audioconvert2 = gst_element_factory_make ("audioconvert", NULL);
			g_assert (audioconvert2);
			audioconvert3 = gst_element_factory_make ("audioconvert", NULL);
			g_assert (audioconvert3);
			audioresample = gst_element_factory_make ("audioresample", NULL);
			g_assert (audioresample);
			g_object_set(G_OBJECT (audioresample), "quality", 10, NULL);

			level = gst_element_factory_make ("level", NULL);
			g_assert (level);
			g_object_set (G_OBJECT (level), "message", TRUE, NULL);

			enc = gst_element_factory_make ("flacenc", NULL);
			g_assert (enc);	

			audiopanorama = gst_element_factory_make ("audiopanorama", NULL);
			g_assert (audiopanorama);
			g_object_set (G_OBJECT (audiopanorama), "panorama", (gfloat)0.5, NULL);

			/* create sink element */
			if (playType == ACT_REC || playType == REC) {
				if (playType == REC) {
					sink = gst_element_factory_make ("filesink", NULL);
					g_assert (sink);
					g_object_set (G_OBJECT (sink), "location", tmpPath, NULL);
					}
				else {
					sink = gst_element_factory_make ("fakesink", NULL);
					g_assert (sink);
					}
				}
			else if (playType == PLAY || playType == PLAY_REC) {
				if (JACK){
					sink = gst_element_factory_make ("jackaudiosink", PACKAGE);
					g_object_set (sink, "client", ptplayer_sink, NULL);
					}
				else {
					sink = gst_element_factory_make ("autoaudiosink", NULL);
					g_assert (sink);
					}
				}
			else if (playType == SAVE) {
				printf("sink = filesink\n"); 
				sink = gst_element_factory_make ("filesink", NULL);
				g_assert (sink);
				g_object_set (G_OBJECT (sink), "location", encodePath, NULL);
				}

			/* add elements to pipeline */
			if (playType == ACT_REC || playType == REC) {
				gst_bin_add_many (GST_BIN (pipeline), src, audioconvert1, spectrum, level, enc, sink, NULL);
				}
			else if (playType == PLAY || playType == PLAY_REC) {
				gst_bin_add_many (GST_BIN (pipeline), audioconvert1, pitch, audioconvert2, equalizer, audioconvert3, spectrum, audioresample, audiopanorama, sink, NULL);
				}
			else if (playType == SAVE) {
				gst_bin_add_many (GST_BIN (pipeline), audioconvert1, pitch, audioconvert2, spectrum, enc, sink, NULL);
				}

			/* link elements */
			//caps = gst_caps_new_simple ("audio/x-raw-int", "rate", G_TYPE_INT, AUDIOFREQ, NULL);
			if (playType == ACT_REC || playType == REC) {			
				if (!gst_element_link_many (src, audioconvert1, spectrum, level, enc, sink, NULL) 
				    /*!gst_element_link_filtered (audioconvert1, spectrum, caps) ||
				    !gst_element_link_many (spectrum, level, enc, sink, NULL)*/) {
				    fprintf (stderr, "can't link elements\n");
					exit (1);
				    }
				}
			else if (playType == PLAY || playType == PLAY_REC) {
				printf ("PLAY || PLAYREC\n");
				if (!gst_element_link_many (audioconvert1, pitch, audioconvert2, equalizer, audioconvert3, spectrum, audioresample, audiopanorama, sink, NULL)){
				    fprintf (stderr, "can't link elements\n");
					exit (1);
			    	    }
				/*if (!gst_element_link_many (audioconvert2, equalizer, NULL)){
				    fprintf (stderr, "can't link elements2\n");
					exit (1);
			    	    }
				if (!gst_element_link_many (equalizer, audioconvert3, spectrum, audioresample, audiopanorama, sink, NULL)){
				    fprintf (stderr, "can't link elements3\n");
					exit (1);
			    	    }*/
				}
			if (playType == SAVE) {			
				if (!gst_element_link_many (audioconvert1, pitch, audioconvert2, spectrum, enc, sink, NULL) 
				    /*!gst_element_link_filtered (audioconvert2, spectrum, caps) ||
				    !gst_element_link_many (spectrum, enc, sink, NULL)*/ ) {
				    fprintf (stderr, "can't link elements\n");
					exit (1);
				    }
				}
			//gst_caps_unref (caps);
			/* if source == AUDIO_FILE, add the playbin element to the pipeline */
			if (playType == PLAY || playType == PLAY_REC || playType == SAVE){
				audiopad = gst_element_get_static_pad (audioconvert1, "sink");
				gst_element_add_pad (pipeline, gst_ghost_pad_new ("sink", audiopad));
				g_object_set(G_OBJECT(playbin), "audio-sink", pipeline, NULL);
				gst_object_unref (audiopad);
				}
			/* create bus */
			if (playType == ACT_REC || playType == REC) {
				bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
				gst_bus_add_watch(bus, message_handler, NULL);
				gst_object_unref(bus);
				}
			else if (playType == PLAY || playType == PLAY_REC || playType == SAVE){
				bus = gst_pipeline_get_bus (GST_PIPELINE (playbin));
				gst_bus_add_watch(bus, message_handler, NULL);
				gst_object_unref(bus);
				}	
		
			/* set state to PLAYING and start main loop */
			if (playType == ACT_REC || playType == REC) {
				gst_element_set_state (pipeline, GST_STATE_PLAYING);
				//printf("play pipeline\n");
				}
			else if (playType == PLAY || playType == PLAY_REC || playType == SAVE){
				gst_element_set_state (playbin, GST_STATE_PLAYING);
				timeoutPrintPosition = g_timeout_add (200, (GSourceFunc) cb_print_position, pipeline);
				timeoutSeekToTime = g_timeout_add (201, (GSourceFunc) seek_to_time, pipeline);
				//printf("play playbin\n");
				}
			printf ("Now playing\n");
			g_main_loop_run (loop);
			
			/* stop playing */
			GstState state, state2;
			printf ("ici");
			gst_element_get_state (pipeline, &state, NULL, GST_CLOCK_TIME_NONE);
			gst_element_get_state (playbin, &state2, NULL, GST_CLOCK_TIME_NONE);
			if (state == 3 || state == 4) {
				gst_element_set_state (pipeline, GST_STATE_NULL);
				}
			else if (state2 == 3 || state2 == 4){
				g_source_remove(timeoutPrintPosition);
				g_source_remove(timeoutSeekToTime);
				gst_element_set_state (playbin, GST_STATE_NULL);
				}
			//printf("TEST : playType = %d, redo = %d, stop = %d, pose = %d\n", playType, redo, stop, pose);
			printf ("Stop playing\n");
			playlistPosition = playlistPosition + 1;
			getFileName();
			}

			playing = 0; pose = 0;
			reset_view();
	/* If  we are in the record mode, we should get back to the ACT_REC playType as soon as recording or playing the recorded file is finished */
			//printf("END 1 : playType = %d, redo = %d, stop = %d\n", playType, redo, stop);
			if (redo == 0){
				if (act_rec){
					//printf("if act_rec && redo == 0 \n");
					redo = 1;
					playType = ACT_REC;
					}
				if (playType == PLAY && playlistPosition < playlistStoreSize && playlistStoreSize > 0) {
					redo = 1;
					}
				}
			if (stop && act_rec == FALSE){
				redo = 0;
				stop = 0;
				}
			//printf("END 2 : playType = %d, redo = %d, stop = %d, pose = %d\n", playType, redo, stop, pose);

	} while (redo); 
	//printf(" OUT OF LOOP\n ");
	playType = NONE;
	playing = 0;
	playlistPosition = 0;
	getFileName();
	}
	
}



