#ifndef __pitchcontrol_H__
#define __pitchcontrol_H__

#include <app.h>
#include <Elementary.h>
#include <system_settings.h>
#include <efl_extension.h>
#include <dlog.h>

#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG "pitchcontrol"

#if !defined(PACKAGE)
#define PACKAGE "priv.sper.pitchcontrol"
#endif

#define EDJ_FILE "edje/pitchcontrol.edj"
#define GRP_MAIN "main"


#endif /* __pitchcontrol_H__ */
