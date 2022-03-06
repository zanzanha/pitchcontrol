/*
 * audio_callback.c
 *
 *  Created on: Mar 23, 2016
 *      Author: a552095
 */
#include "audio_callback.h"

#include <stdio.h>
#include <math.h>

#include "fourier.h"
#define BUFFSIZE 32768

/*
 * Berechnet die Anzahl Halbtöne, die der gesuchte Ton vom Kammerton entfernt ist und zählt vier Oktaven dazu.
 */
static double calculateHalfTones(float freq) {
	return 12. / M_LN2 * (log(freq) - log(440.)) + 48.5;
}

const static char *note[] = {
		"A", "A", "H", "C", "C", "D", "D", "E", "F", "F", "G", "G"
};

#define MAXOCTAVE 8

const static char *octave[MAXOCTAVE] = {
		"sub2", "sub1", "gr.", "kl.", "sup'", "sup''", "sup'''", "sup''''"
};

const static char *accidental[] = {
		"", "#", "", "", "#", "", "#", "", "", "#", "", "#"
};

/*
 * @brief Rotate hands of the watch
 * @param[in] hand The hand you want to rotate
 * @param[in] degree The degree you want to rotate
 * @param[in] cx The rotation's center horizontal position
 * @param[in] cy The rotation's center vertical position
 */
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

float oldfreq = 0.;

void displayNote(float freq, appdata_s *ad) {
	if (freq == oldfreq)
		return;
	elm_win_norender_push(ad->win);
	char hertzstr[32];
	double deg = 0.;
	if (freq > 10.) {
		double halftones = calculateHalfTones(freq);
		int fht = (int)halftones;
		int octaveidx = (fht - 3) / 12;
		int noteidx = fht % 12;
		sprintf(hertzstr, "%.1f Hz", freq);
//		Evas_Object *hand = elm_object_part_content_get(layout, "hand_cent");
//		view_rotate_hand(hand, 2., 180, 180);
//		elm_object_part_content_set(layout, "hand_cent", hand);
		evas_object_text_text_set(ad->freq, hertzstr);
		evas_object_text_text_set(ad->note, note[noteidx]);
		evas_object_text_text_set(ad->accidental, accidental[noteidx]);
		if (octaveidx < MAXOCTAVE)
			evas_object_text_text_set(ad->octave, octave[octaveidx]);
		else
			evas_object_text_text_set(ad->octave, "");
		deg = (halftones - 0.5 - fht) * 40.;
	} else {
		evas_object_text_text_set(ad->freq, "");
		evas_object_text_text_set(ad->note, "-");
		evas_object_text_text_set(ad->accidental, "");
		evas_object_text_text_set(ad->octave, "");
	}
	Evas_Map *rot = evas_map_new(4);
	evas_map_util_points_populate_from_object(rot, ad->hand);
	evas_map_point_image_uv_set(rot, 0, 0., 0.);
	evas_map_point_image_uv_set(rot, 1, 60., 0.);
	evas_map_point_image_uv_set(rot, 2, 60., 720.);
	evas_map_point_image_uv_set(rot, 3, 0., 720.);
	evas_map_util_rotate(rot, deg, ad->centerX, ad->centerY);
	evas_object_map_set(ad->hand, rot);
	evas_object_map_enable_set(ad->hand, EINA_TRUE);
	elm_win_render(ad->win);
	elm_win_norender_pop(ad->win);
}

float data[BUFFSIZE];
float *dataptr, *dataend;
char working = 0;
int nproc = 0;

void reset_data() {
	dataptr = data;
	dataend = data + BUFFSIZE;
}


float absquad(float *dptr) {
	return dptr[0] * dptr[0] + dptr[1] * dptr[1];
}

void evaluate_audio(appdata_s *ad) {
	realft(data - 1, BUFFSIZE, 1);
	// GetMax
	float maxval = 0;
	int maxidx = 0;
	for (int i = 40; i < BUFFSIZE; i += 2) {
		float val = absquad(data + i) / i;
		if (val > maxval) {
			maxval = val;
			maxidx = i / 2;
		}
	}
	if (maxval * maxidx > 5.e10) {
		// Aufgrund der Werte der Nachbarpunkte und einer quadratischen Interpolation
		// versuchen wir die Frequenz noch genauer abzuschätzen.
		float ym = sqrt(absquad(data + maxidx - 1));
		float y0 = sqrt(maxval);
		float yp = sqrt(absquad(data + maxidx + 1));
		float corr = (ym - yp) / (2. * ym - 4. * y0 + 2. * yp);
		float freq = ((float) maxidx + corr) * SAMPLE_RATE / BUFFSIZE;
		displayNote(freq, ad);
	} else {
		displayNote(0.f, ad);
	}
}

void io_stream_callback(audio_in_h handle, size_t nbytes, void *userdata) {
	const short *buffer;
	if (nbytes > 0) {
		audio_in_peek(handle, &buffer, &nbytes);
		if (!working) {
			short *buffend = ((char *)buffer) + nbytes;
			while (dataptr < dataend && buffer < buffend) {
				*dataptr++ = *buffer++;
			}
			audio_in_drop(handle);
			if (dataptr >= dataend)  {
				working = 1;
				evaluate_audio((appdata_s *)userdata);
				reset_data();
				working = 0;
			}
		} else
			audio_in_drop(handle);
	}
}

