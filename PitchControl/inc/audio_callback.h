/*
 * audio_callback.h
 *
 *  Created on: Mar 23, 2016
 *      Author: a552095
 */

#ifndef AUDIO_CALLBACK_H_
#define AUDIO_CALLBACK_H_
#include <Elementary.h>
#include <audio_io.h>

#define SAMPLE_RATE 44100

extern Evas_Object *layout, *window;

void updateLabel(char *txt);
void reset_data();

void io_stream_callback(audio_in_h handle, size_t nbytes, void *userdata);

#endif /* AUDIO_CALLBACK_H_ */
