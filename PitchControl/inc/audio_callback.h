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
#include <time.h>

#define SAMPLE_RATE 44100
#define MAXOCTAVE 8

extern char **noteName, **octName;
extern char language[3];

void reset_data();

typedef struct appdata{
	Evas_Object *win, *note, *accidental, *freq, *octave, *hand;
	Evas *canvas;
	int centerX, centerY;
	float dispFreq;
	audio_in_h input;
	Ecore_Thread *thread;
} appdata_s;

void displayNote(appdata_s *ad, float freq);
void printError(appdata_s *ad, char *msg, int code);
Ecore_Thread *startRecordThread(appdata_s *ad);

#endif /* AUDIO_CALLBACK_H_ */
