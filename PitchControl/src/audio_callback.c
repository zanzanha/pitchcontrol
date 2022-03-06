/*
 * audio_callback.c
 *
 *  Created on: Mar 23, 2016
 *      Author: a552095
 */
#include "audio_callback.h"
#include <dlog.h>

#include <stdio.h>
#include <math.h>

#include "fourier.h"
#define BUFFSIZE 32768

char **noteName, **octName;

/*
 * Berechnet die Anzahl Halbtöne, die der gesuchte Ton vom Kammerton entfernt ist und zählt vier Oktaven dazu.
 */
static double calculateHalfTones(float freq) {
	return 12. / M_LN2 * (log(freq) - log(440.)) + 48.5;
}

const static char *accidental[] = {
		"", "#", "", "", "#", "", "#", "", "", "#", "", "#"
};

char language[3];
float data[2][BUFFSIZE];
int nproc = 0, activebuf = 0, idx = 0;

void reset_data() {
	activebuf = idx = 0;
}


float absquad(float *dptr) {
	return dptr[0] * dptr[0] + dptr[1] * dptr[1];
}


void printError(appdata_s *ad, char *msg, int code) {
	char txt[80];
	sprintf(txt, "%s: %d", msg, code);
	evas_object_text_text_set(ad->freq, txt);
}

float evaluate_audio(float *data, appdata_s *ad) {
	realft(data - 1, BUFFSIZE, 1);
	// GetMax
	float maxval = 0;
	int maxidx = 0;
	for (int i = 40; i < BUFFSIZE; i += 2) {
		float val = absquad(data + i) / (i + 100);
		if (val > maxval) {
			maxval = val;
			maxidx = i / 2;
		}
	}
	if (maxval * maxidx > 1.e10) {
		// Aufgrund der Werte der Nachbarpunkte und einer quadratischen Interpolation
		// versuchen wir die Frequenz noch genauer abzuschätzen.
		float ym = sqrt(absquad(data + maxidx - 1));
		float y0 = sqrt(maxval);
		float yp = sqrt(absquad(data + maxidx + 1));
		float corr = (ym - yp) / (2. * ym - 4. * y0 + 2. * yp);
		float freq = ((float) maxidx + corr) * SAMPLE_RATE / BUFFSIZE;
		return freq;
	} else {
		return 0.f;
	}
}

void copyAudioData(const short *buffer, size_t nbytes, void *userdata) {
	float *evalbuf = NULL;
	appdata_s *ad = (appdata_s *)userdata;
	if (nbytes > 0) {
		short *buffend = ((char *)buffer) + nbytes;
		while (buffer < buffend) {
			data[activebuf][idx++] = *buffer++;
			if (idx >= BUFFSIZE)  {
				memcpy(data[1 - activebuf], data[activebuf] + BUFFSIZE / 2, BUFFSIZE / 2 * sizeof(float));
				evalbuf = data[activebuf];
				activebuf = 1 - activebuf;
				idx = BUFFSIZE / 2;
			}
		}
		if (evalbuf != NULL) {
			float freq = evaluate_audio(evalbuf, ad);
			if (freq != ad->dispFreq) {
				ecore_thread_feedback(ad->thread, &freq);
			}
		}
	}
}


void activateAudio(void *data, Ecore_Thread *thread) {
	appdata_s *ad = data;
	dlog_print(DLOG_INFO, LOG_TAG, "Audio Record Start Requested");
	audio_io_error_e error_code;
	reset_data();
	error_code = audio_in_prepare(ad->input);
	if (error_code) {
		printError(ad, "Fehler audio_in_prepare", error_code);
		return;
	}
	dlog_print(DLOG_INFO, LOG_TAG, "Audio was activated");
	unsigned long buffbytes = BUFFSIZE * sizeof(short);
	void *buff = malloc(buffbytes);

	while (!ecore_thread_check(thread)) {
		int read = audio_in_read(ad->input, buff, buffbytes);
		dlog_print(DLOG_DEBUG, LOG_TAG, "Read %lu audio bytes", read);
		if (read > 0) {
			copyAudioData(buff, read, ad);
		} else if (read < 0) {
			dlog_print(DLOG_ERROR, LOG_TAG, "Error reading Audio", read);
		}
	}

	audio_in_unprepare(ad->input);
	free(buff);
}

void view_rotate_hand(Evas_Object *hand, double degree, Evas_Coord cx, Evas_Coord cy)
{
	Evas_Map *m = NULL;

	m = evas_map_new(4);
	evas_map_util_points_populate_from_object(m, hand);
	evas_map_util_rotate(m, degree, cx, cy);
	evas_object_map_set(hand, m);
	evas_object_map_enable_set(hand, EINA_TRUE);
	evas_map_free(m);
}

void displayNote(appdata_s *ad, float freq) {
//	dlog_print(DLOG_DEBUG, LOG_TAG, "Timer was triggered: NewFreq: %f, oldFreq: %f", ad->newFreq, ad->dispFreq);
	char hertzstr[32];
	double deg = 0.;
	if (freq > 27.) {
		double halftones = calculateHalfTones(freq);
		int fht = (int)halftones;
		int octaveidx = (fht - 3) / 12;
		int noteidx = fht % 12;
		sprintf(hertzstr, "%.1f Hz", freq);
		evas_object_text_text_set(ad->freq, hertzstr);
		evas_object_text_text_set(ad->note, noteName[noteidx]);
		evas_object_text_text_set(ad->accidental, accidental[noteidx]);
		if (octaveidx < MAXOCTAVE)
			evas_object_text_text_set(ad->octave, octName[octaveidx]);
		else
			evas_object_text_text_set(ad->octave, "");
		deg = (halftones - 0.5 - fht) * 80.;
	} else {
		evas_object_text_text_set(ad->freq, "-------");
		evas_object_text_text_set(ad->note, "");
		evas_object_text_text_set(ad->accidental, "");
		evas_object_text_text_set(ad->octave, "");
	}
	Evas_Map *rot = evas_map_new(4);
	evas_map_util_points_populate_from_object(rot, ad->hand);
	evas_map_point_image_uv_set(rot, 0, 0., 0.);
	evas_map_point_image_uv_set(rot, 1, 39., 0.);
	evas_map_point_image_uv_set(rot, 2, 39., 88.);
	evas_map_point_image_uv_set(rot, 3, 0., 88.);
	evas_map_util_rotate(rot, deg, ad->centerX, ad->centerY);
	evas_object_map_set(ad->hand, rot);
	evas_object_map_enable_set(ad->hand, EINA_TRUE);
	ad->dispFreq = freq;
}

void displayNoteCallback(void *data, Ecore_Thread *thread, void *msg_data) {
	displayNote((appdata_s *)data, *(float *)msg_data);
}

static void thread_end_cb(void *data, Ecore_Thread *thread)
{
}

static void thread_cancel_cb(void *data, Ecore_Thread *thread)
{
}

Ecore_Thread *startRecordThread(appdata_s *ad) {
	Ecore_Thread *thread = ecore_thread_feedback_run(activateAudio, displayNoteCallback,
	   thread_end_cb, thread_cancel_cb, ad,
	   EINA_TRUE);
	return thread;
}

