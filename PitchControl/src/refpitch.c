#include <Elementary.h>
#include <efl_extension.h>
#include <app.h>
#include <dlog.h>
#include <time.h>

#include "refpitch.h"
#include "audio_callback.h"

static Evas_Object *refRect, *titleText, *refText;
int refPitch = 440;
static short refIsShown = 0;
static time_t mytime;

/**
 * @brief Registers rotary event callback function.
 * @param[in] obj Object that will receive rotary event
 * @param[in] rotary_cb Function will be operated when rotary event happens
 * @param[in] user_data Data passed to the 'rotary_cb' function
 */
void set_rotary_event_callback(Evas_Object *obj, Eext_Rotary_Event_Cb rotary_cb, void *user_data)
{
	if (obj == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "parent is NULL.");
		return;
	}

	eext_rotary_object_event_activated_set(obj, EINA_TRUE);
	eext_rotary_object_event_callback_add(obj, rotary_cb, user_data);
}

static void showRef() {
	if (!refIsShown) {
		evas_object_show(refRect);
		evas_object_show(titleText);
		evas_object_show(refText);
		refIsShown = 1;
	}
}

short hideRef() {
	if (refIsShown && time(NULL) - mytime > 2) {
		evas_object_hide(refRect);
		evas_object_hide(titleText);
		evas_object_hide(refText);
		refIsShown = 0;
		return 0;
	}
	return refIsShown;
}

static Eina_Bool rotary_cb(void *user_data, Evas_Object *obj, Eext_Rotary_Event_Info *rotary_info)
{
	appdata_s *ad = user_data;
	dlog_print(DLOG_DEBUG, LOG_TAG, "Detent detected, obj[%p], direction[%d]", obj, rotary_info->direction);
	mytime = time(NULL);
	if (refIsShown) {
		if (rotary_info->direction == EEXT_ROTARY_DIRECTION_CLOCKWISE) {
			if (refPitch < 470)
				refPitch++;
		} else {
			if (refPitch > 410)
				refPitch--;
		}
		char disp[8];
		sprintf(disp, "%d Hz", refPitch);
		evas_object_text_text_set(refText, disp);
	} else
		showRef();
	return ECORE_CALLBACK_RENEW;
}


void register_rotary_callback(appdata_s *data)
{
	refRect = evas_object_rectangle_add(data->canvas);
	evas_object_resize(refRect, data->centerX * 1.4, data->centerY * 1.0);
	evas_object_move(refRect, 0.3 * data->centerX, 0.5 * data->centerY);
	evas_object_color_set(refRect, 230, 230, 230, 164);
	titleText = evas_object_text_add(data->canvas);
	evas_object_text_font_set(titleText, "TizenSans:style=bold", 16);
	evas_object_color_set(titleText, 128, 0, 0, 255);
	evas_object_move(titleText, 0.3 * data->centerX + 15, 0.5 * data->centerY + 15);
	evas_object_text_text_set(titleText, "Reference Concert Pitch");
	refText = evas_object_text_add(data->canvas);
	evas_object_text_font_set(refText, "TizenSans:style=bold", 60);
	evas_object_color_set(refText, 80, 20, 0, 255);
	evas_object_text_text_set(refText, "440 Hz");
	int horiz = evas_object_text_horiz_advance_get(refText);
	int vert = evas_object_text_vert_advance_get(refText);
	evas_object_move(refText, data->centerX - horiz / 2, data->centerY - vert / 2);
	set_rotary_event_callback(data->win, rotary_cb, data);
}
