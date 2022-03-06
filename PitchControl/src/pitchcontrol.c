#include "pitchcontrol.h"
#include "app_activate.h"
#include "audio_callback.h"

static void
win_delete_request_cb(void *data, Evas_Object *obj, void *event_info)
{
	ui_app_exit();
}

static void
win_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = data;
	/* Let window go to hide state. */
	elm_win_lower(ad->win);
}

static void
app_get_resource(const char *edj_file_in, char *edj_path_out, int edj_path_max)
{
	char *res_path = app_get_resource_path();
	if (res_path) {
		snprintf(edj_path_out, edj_path_max, "%s%s", res_path, edj_file_in);
		free(res_path);
	}
}

static Evas_Object *addImage(Evas* canvas, char *imagepath, int xpos, int ypos, int width, int height) {
	/* Image */
	char edj_path[PATH_MAX] = { 0, };
	Evas_Object *img = evas_object_image_filled_add(canvas);
	app_get_resource(imagepath, edj_path, (int) PATH_MAX);
	evas_object_image_file_set(img, edj_path, NULL);
	evas_object_move(img, xpos, ypos);
	evas_object_resize(img, width, height);
	evas_object_show(img);
	return img;
}

static void
create_base_gui(appdata_s *ad)
{
	int winwidth, winheight, centerX, centerY;
	/* Window */
	ad->win = elm_win_util_standard_add(PACKAGE, PACKAGE);
	elm_win_screen_size_get(ad->win, NULL, NULL, &winwidth, &winheight);
	ad->centerX = centerX = winwidth / 2;
	ad->centerY = centerY = winheight / 2;
	elm_win_autodel_set(ad->win, EINA_TRUE);

	if (elm_win_wm_rotation_supported_get(ad->win)) {
		int rots[4] = { 0, 90, 180, 270 };
		elm_win_wm_rotation_available_rotations_set(ad->win, (const int *)(&rots), 4);
	}

	evas_object_smart_callback_add(ad->win, "delete,request", win_delete_request_cb, NULL);
	eext_object_event_callback_add(ad->win, EEXT_CALLBACK_BACK, win_back_cb, ad);
	evas_object_show(ad->win);

	Evas *canvas = ad->canvas = evas_object_evas_get(ad->win);
	/* Image */
	addImage(canvas, "images/zifferblatt.png", 0, 0, winwidth, winheight);
	ad->hand = addImage(canvas, "images/zeiger.png", centerX - 11, 40, 21, 43);
	ad->note = evas_object_text_add(canvas);
	evas_object_text_text_set(ad->note, "");
	evas_object_text_style_set(ad->note, EVAS_TEXT_STYLE_TIZEN_GLOW_SHADOW);
	evas_object_color_set(ad->note, 0, 127, 255, 255);
	evas_object_show(ad->note);
	ad->accidental = evas_object_text_add(canvas);
	evas_object_text_font_set(ad->accidental, "TizenSans:style=bold", 40);
	evas_object_color_set(ad->accidental, 0, 220, 255, 255);
	evas_object_move(ad->accidental, centerX + 40, centerY - 75);
	evas_object_show(ad->accidental);
	ad->octave = evas_object_text_add(canvas);
	evas_object_text_font_set(ad->octave, "TizenSans:style=bold", 32);
	evas_object_color_set(ad->octave, 0, 220, 128, 255);
	evas_object_move(ad->octave, centerX - 120, centerY - 16);
	evas_object_show(ad->octave);
	ad->freq = evas_object_text_add(canvas);
	evas_object_text_font_set(ad->freq, "TizenSans:style=bold", 32);
	evas_object_color_set(ad->freq, 0, 128, 128, 255);
	evas_object_move(ad->freq, centerX - 45, centerY + 60);
	evas_object_show(ad->freq);
}

static bool
app_create(void *data)
{
	/* Hook to take necessary actions before main event loop starts
	   Initialize UI resources and application's data
	   If this function returns true, the main loop of application starts
	   If this function returns false, the application is terminated */
	appdata_s *ad = data;

	create_base_gui(ad);
	ad->dispFreq = -1.f;
	ad->newFreq = 0.f;
	ad->timer = ecore_timer_add(0.1, displayNote, data);
	ad->isActive = 0;
	ad->audioActive = 0;
	// Initialize the audio input device
	// Initialize the audio input device

	audio_io_error_e error_code;
	error_code = audio_in_create(SAMPLE_RATE, AUDIO_CHANNEL_MONO,
			AUDIO_SAMPLE_TYPE_S16_LE, &ad->input);
	if (error_code) {
		printError(ad, "Fehler audio_in_create", error_code);
		return false;
	}
	error_code = audio_in_set_stream_cb(ad->input, io_stream_callback, ad);
	if (error_code) {
		printError(ad, "Fehler audio_in_set_stream", error_code);
		error_code = audio_in_destroy(ad->input);
		return false;
	}

	return true;
}

static void
app_control(app_control_h app_control, void *data)
{
	activateApp((appdata_s *)data);
}

static void
app_pause(void *data)
{
	deactivateApp((appdata_s *)data);
}

static void
app_resume(void *data)
{
	activateApp((appdata_s *)data);
}

static void
app_terminate(void *data)
{
	deactivateApp((appdata_s *)data);
}

static void
ui_app_lang_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LANGUAGE_CHANGED*/
	char *locale = NULL;
	system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &locale);
	elm_language_set(locale);
	free(locale);
	return;
}

static void
ui_app_orient_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_DEVICE_ORIENTATION_CHANGED*/
	return;
}

static void
ui_app_region_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_REGION_FORMAT_CHANGED*/
}

static void
ui_app_low_battery(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_BATTERY*/
}

static void
ui_app_low_memory(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_MEMORY*/
}

int
main(int argc, char *argv[])
{
	appdata_s ad = {0,};
	int ret = 0;

	ui_app_lifecycle_callback_s event_callback = {0,};
	app_event_handler_h handlers[5] = {NULL, };

	event_callback.create = app_create;
	event_callback.terminate = app_terminate;
	event_callback.pause = app_pause;
	event_callback.resume = app_resume;
	event_callback.app_control = app_control;

	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, ui_app_low_battery, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY, ui_app_low_memory, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_DEVICE_ORIENTATION_CHANGED], APP_EVENT_DEVICE_ORIENTATION_CHANGED, ui_app_orient_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, ui_app_lang_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, ui_app_region_changed, &ad);
	ui_app_remove_event_handler(handlers[APP_EVENT_LOW_MEMORY]);

	ret = ui_app_main(argc, argv, &event_callback, &ad);
	if (ret != APP_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "ui_app_main() is failed. err = %d", ret);
	}

	return ret;
}
