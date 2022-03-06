/*
 * audio_read.cc
 *
 *  Created on: Mar 22, 2016
 *      Author: a552095
 */

#include <audio_io.h>
#include <stdio.h>

#include "audio_read.h"
#include "audio_callback.h"


audio_in_h input;
char isActive = 0;

void printError(char *msg, int code) {
	char txt[80];
	sprintf(txt, "%s: %d", msg, code);
	updateLabel(txt);
}

void activateAudioModule() {
	if (isActive)
		return;
	audio_io_error_e error_code;

	// Initialize the audio input device

	error_code = audio_in_create(SAMPLE_RATE, AUDIO_CHANNEL_MONO,
			AUDIO_SAMPLE_TYPE_S16_LE, &input);
	if (error_code) {
		printError("Fehler audio_in_create", error_code);
		return;
	}
	error_code = audio_in_set_stream_cb(input, io_stream_callback, NULL);
	if (error_code) {
		printError("Fehler audio_in_set_stream", error_code);
		return;
	}
	reset_data();
	error_code = audio_in_prepare(input);
	if (error_code) {
		printError("Fehler audio_in_prepare", error_code);
		return;
	}
	isActive = 1;
}

void deactivateAudioModule() {
	if (!isActive)
		return;
	int error_code;
	error_code = audio_in_unprepare(input);
	error_code = audio_in_destroy(input);
	isActive = 0;
}
