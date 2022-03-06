/*
 * audio_read.cc
 *
 *  Created on: Mar 22, 2016
 *      Author: a552095
 */

#include <audio_io.h>
#include <stdio.h>
#include <system_settings.h>
#include <device/power.h>
#include <dlog.h>

#include "app_activate.h"
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

const static char *octave[MAXOCTAVE] = {
		"ctr,,", "ctr,", "gr.", "sm.", "sup'", "sup''", "sup'''", "sup''''"
};

const static char *octave_de[MAXOCTAVE] = {
		"ktr,,", "kontra", "gr.", "kl.", "sup'", "sup''", "sup'''", "sup''''"
};

const static char *octave_lat[MAXOCTAVE] = {
		"o-1", "o1", "o2", "o3", "o4", "o5", "o6", "o7"
};

void activateApp(appdata_s *ad) {
	if (ad->isActive) {
		dlog_print(DLOG_INFO, LOG_TAG, "App is already active");
		return;
	}
	dlog_print(DLOG_INFO, LOG_TAG, "App activation Requested");

	ad->isActive = 1;
	device_power_request_lock(POWER_LOCK_DISPLAY, 180000);
	char *locale;
	system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &locale);
	strncpy(language, locale, 2);
	language[2] = '\0';
	// Retrieve the current system language
	int notesize = 140, x = 40, y = 100;
	if (strcmp(language, "de") == 0) {
		noteName = note_de;
		octName = octave_de;
	} else if (strstr("fr|it|es|pt", language) != NULL) {
		noteName = note_lat;
		octName = octave_lat;
		notesize = 90;
		y = 70;
	} else {
		noteName = note;
		octName = octave;
	}
	evas_object_move(ad->note, ad->centerX - x, ad->centerY - y);
	evas_object_text_font_set(ad->note, "TizenSans:style=bold", notesize);
	dlog_print(DLOG_INFO, LOG_TAG, "App was activated");
	ad->timer = ecore_timer_add(0.08, displayNote, ad);
	reset_data();
	ad->newFreq = 0.;
	displayNote(ad);
}

void deactivateApp(appdata_s *ad) {
	if (!ad->isActive) {
		dlog_print(DLOG_INFO, LOG_TAG, "App is already inactive");
		return;
	}
	device_power_release_lock(POWER_LOCK_DISPLAY);
	ad->isActive = 0;
	time(&ad->pauseTime);
	dlog_print(DLOG_INFO, LOG_TAG, "App deactivated");
	ecore_timer_add(30., deactivateAudio, ad);
}
