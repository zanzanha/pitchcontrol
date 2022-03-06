/*
 * audio_callback.h
 *
 *  Created on: Mar 23, 2016
 *      Author: a552095
 */

#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG "PITCHCONTROL"

#ifndef AUDIO_CALLBACK_H_
#define AUDIO_CALLBACK_H_
#include <Elementary.h>
#include <audio_io.h>

#define SAMPLE_RATE 44100
#define MAXOCTAVE 8

extern char **noteName, **octName;
extern char language[3];

void reset_data();

void io_stream_callback(audio_in_h handle, size_t nbytes, void *userdata);

typedef struct appdata{
	Evas_Object *win, *note, *accidental, *freq, *octave, *hand;
	Evas *canvas;
	char isActive;
	int centerX, centerY;
	Ecore_Timer *timer;
	float dispFreq, newFreq;
	audio_in_h input;
} appdata_s;

Eina_Bool displayNote(void *ad);

#endif /* AUDIO_CALLBACK_H_ */
