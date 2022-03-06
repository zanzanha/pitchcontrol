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

Evas_Object *layout, *window;

/*
 * Berechnet die Anzahl Halbtöne, die der gesuchte Ton vom Kammerton entfernt ist und zählt vier Oktaven dazu.
 */
static int calculateHalfTones(float freq) {
	int halftones = (int)(12. / M_LN2 * (log(freq) - log(440.)) + 48.5);
	return halftones;
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

void updateLabel(char *txt) {
	elm_object_part_text_set(layout, "hertz", txt);
}

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

void displayNote(float freq) {
	char hertzstr[32];
	if (freq > 10.) {
		int halftones = calculateHalfTones(freq);
		int octaveidx = (halftones - 3) / 12;
		int noteidx = halftones % 12;
		sprintf(hertzstr, "%.1f Hz", freq);
//		Evas_Object *hand = elm_object_part_content_get(layout, "hand_cent");
//		view_rotate_hand(hand, 2., 180, 180);
//		elm_object_part_content_set(layout, "hand_cent", hand);
		elm_object_part_text_set(layout, "hertz", hertzstr);
		elm_object_part_text_set(layout, "note_name", note[noteidx]);
		elm_object_part_text_set(layout, "accidental", accidental[noteidx]);
		if (octaveidx < MAXOCTAVE)
			elm_object_part_text_set(layout, "octave", octave[octaveidx]);
		else
			elm_object_part_text_set(layout, "octave", "");
	} else {
		elm_object_part_text_set(layout, "hertz", "");
		elm_object_part_text_set(layout, "note_name", "-");
		elm_object_part_text_set(layout, "accidental", "");
		elm_object_part_text_set(layout, "octave", "");
	}
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

void evaluate_audio() {
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
		displayNote(freq);
	} else {
		displayNote(0.f);
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
				evaluate_audio();
				reset_data();
				working = 0;
			}
		} else
			audio_in_drop(handle);
	}
}

