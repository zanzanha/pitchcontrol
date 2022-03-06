/*
 * audio_read.cc
 *
 *  Created on: Mar 22, 2016
 *      Author: a552095
 */

#include <audio_io.h>
#include <stdio.h>
#include <system_settings.h>

#include "audio_read.h"
#include "audio_callback.h"

const static char *note_de[] = {
		"A", "A", "H", "C", "C", "D", "D", "E", "F", "F", "G", "G"
};

const static char *note_lat[] = {
		"la", "la", "si", "do", "do", "re", "re", "mi", "fa", "fa", "sol", "sol"
};

const static char *note[] = {
		"A", "A", "B", "C", "C", "D", "D", "E", "F", "F", "G", "G"
};

audio_in_h input;
char isActive = 0;

void printError(appdata_s *ad, char *msg, int code) {
	char txt[80];
	sprintf(txt, "%s: %d", msg, code);
	evas_object_text_text_set(ad->freq, txt);
}

void activateAudioModule(appdata_s *ad) {
	if (isActive)
		return;
	audio_io_error_e error_code;

	// Initialize the audio input device

	error_code = audio_in_create(SAMPLE_RATE, AUDIO_CHANNEL_MONO,
			AUDIO_SAMPLE_TYPE_S16_LE, &input);
	if (error_code) {
		printError(ad, "Fehler audio_in_create", error_code);
		return;
	}
	error_code = audio_in_set_stream_cb(input, io_stream_callback, ad);
	if (error_code) {
		printError(ad, "Fehler audio_in_set_stream", error_code);
		return;
	}
	reset_data();
	error_code = audio_in_prepare(input);
	if (error_code) {
		printError(ad, "Fehler audio_in_prepare", error_code);
		return;
	}
	isActive = 1;
	char *locale;
	system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &locale);
	strncpy(language, locale, 2);
	language[2] = '\0';
	// Retrieve the current system language
	int notesize = 140, x = 40, y = 100;
	if (strcmp(language, "de") == 0)
		noteName = note_de;
	else if (strstr("fr|it|es|pt", language) != NULL) {
		noteName = note_lat;
		notesize = 90;
		y = 70;
	} else
		noteName = note;
	evas_object_move(ad->note, ad->centerX - x, ad->centerY - y);
	evas_object_text_font_set(ad->note, "TizenSans:style=bold", notesize);
}

void deactivateAudioModule() {
	if (!isActive)
		return;
	int error_code;
	error_code = audio_in_unprepare(input);
	error_code = audio_in_destroy(input);
	isActive = 0;
}
