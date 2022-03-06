#include "pitchcontrol.h"
#include "audio_read.h"
#include "audio_callback.h"

typedef struct appdata{
	Evas_Object* win;
	Evas_Coord width, height;
} appdata_s;

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
win_resize_cb(void *data, Evas *e , Evas_Object *obj , void *event_info)
{
	appdata_s *ad = data;
	evas_object_geometry_get(ad->win, NULL, NULL, &ad->width, &ad->height);
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

static Eina_Bool cb_keepAlive(void *data)
{
	return ECORE_CALLBACK_RENEW;
}

static Ecore_Timer *timer;


static void
create_base_gui(appdata_s *ad)
{
	char edj_path[PATH_MAX] = {0, };

	/* Window */
	ad->win = elm_win_util_standard_add(PACKAGE, PACKAGE);
	evas_object_resize(ad->win, 360, 360);
	elm_win_autodel_set(ad->win, EINA_TRUE);

	if (elm_win_wm_rotation_supported_get(ad->win)) {
		int rots[4] = { 0, 90, 180, 270 };
		elm_win_wm_rotation_available_rotations_set(ad->win, (const int *)(&rots), 4);
	}

	evas_object_smart_callback_add(ad->win, "delete,request", win_delete_request_cb, NULL);
	eext_object_event_callback_add(ad->win, EEXT_CALLBACK_BACK, win_back_cb, ad);
	evas_object_event_callback_add(ad->win, EVAS_CALLBACK_RESIZE, win_resize_cb, ad);
	evas_object_show(ad->win);

	Evas *canvas = evas_object_evas_get(ad->win);
	/* Image */
	Evas_Object *img = evas_object_image_filled_add(canvas);
	app_get_resource("images/centdial.png", edj_path, (int)PATH_MAX);
	evas_object_image_file_set(img, edj_path, NULL);
	evas_object_resize(img, 360, 360);
	evas_object_show(img);
	Evas_Object *hand = evas_object_image_filled_add(canvas);
	app_get_resource("images/hand_cent.png", edj_path, (int)PATH_MAX);
	evas_object_image_file_set(hand, edj_path, NULL);
	evas_object_move(hand, 165, 10);
	evas_object_resize(hand, 30, 340);
	Evas_Map *rot = evas_map_new(4);
	evas_map_util_points_populate_from_object(rot, hand);
	evas_map_point_image_uv_set(rot, 0, 0., 0.);
	evas_map_point_image_uv_set(rot, 1, 60., 0.);
	evas_map_point_image_uv_set(rot, 2, 60., 720.);
	evas_map_point_image_uv_set(rot, 3, 0., 720.);
	evas_map_util_rotate(rot, 55, 180, 180);
	evas_object_map_set(hand, rot);
	evas_object_map_enable_set(hand, EINA_TRUE);
	evas_object_show(hand);

	evas_object_geometry_get(ad->win, NULL, NULL, &ad->width, &ad->height);
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

	return true;
}

static void
app_control(app_control_h app_control, void *data)
{
//	activateAudioModule();
//    /* register the timer */
//    ecore_timer_add(0.2, cb_keepAlive, NULL);
}

static void
app_pause(void *data)
{
	deactivateAudioModule();
	ecore_timer_del(timer);
}

static void
app_resume(void *data)
{
//	activateAudioModule();
//    /* register the timer */
//    ecore_timer_add(0.2, cb_keepAlive, NULL);
}

static void
app_terminate(void *data)
{
	deactivateAudioModule();
	ecore_timer_del(timer);
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
