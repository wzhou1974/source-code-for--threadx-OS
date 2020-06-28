/*
 * ============================================================================
 * Copyright (c) 2011-2013   Marvell International, Ltd. All Rights Reserved
 *
 *                         Marvell Confidential
 * ============================================================================
 */

/** 
 * \file ui_system_interface_platform.h
 * 
 * \brief 
 * 
 * 
 **/

#ifndef __UI_SYSTEM_INTERFACE_PLATFORM_H__
#define __UI_SYSTEM_INTERFACE_PLATFORM_H__

#include <stdint.h>
#include <stdbool.h>

#include "location_api.h"       // for e_CountryCode
#include "string_mgr_api.h"     // for string_mgr_language_t
#include "date_time_api.h"      // for e_DATE_FORMAT and e_TIME_FORMAT
#include "usb_device_vars.h"    // for USB_FORCE_SPEED

#define PLATFORM_MODULE_NAME "platform"

#define PLATFORM_STATUS_IDLE                        GENERAL_STATUS_IDLE
#define PLATFORM_STATUS_READY                       GENERAL_STATUS_READY
#define PLATFORM_STATUS_INITIALIZING                GENERAL_STATUS_INITIALIZING

#define PLATFORM_STATUS_DOWNLOAD_SECURITY_OVERRIDE "download_security_override"
#define PLATFORM_STATUS_DOWNLOAD_SECURITY_DECLINED "download_security_declined"
#define PLATFORM_STATUS_DOWNLOAD_SECURITY_CANCELED "download_security_canceled"
#define PLATFORM_STATUS_BURNFLASH_PROGRAMMING      "burnflash_programming"
#define PLATFORM_STATUS_BURNFLASH_COMPLETE         "burnflash_complete"


#define STRING_MGR_EVENT_CUR_LANG_CHANGED           "cur_lang_changed"
#define STRING_MGR_EVENT_DEFAULT_LANG_CHANGED       "default_lang_changed"

/*
Reset printer
*/
void ui_sys_platform_reset(void);


/*
restoreFactoryDefaults (Set the system's parameters to factory defaults)
*/
void ui_sys_platform_NVRAM_restore_factory_defaults(void);

/*
restoreDefaults (Set the system's parameters to defaults)
*/
void ui_sys_platform_NVRAM_restore_defaults(void);

#ifdef HAVE_NAND
/*
  cleanFS (Restore the file system to a clean [empty] state)
    returns: Nothing
    WARNING: Erases all files and causes machine to reboot -->
*/
void ui_sys_platform_filesystem_clean_fs(void);
#endif


// <!-- country -->
e_CountryCode ui_sys_platform_country_get(void);

void ui_sys_platform_country_set(e_CountryCode country);


// <!-- password -->
// Noted: The caller has the responsibility to free the returned string buffer
char * ui_sys_platform_password_get(void);
bool ui_sys_platform_password_set(char * password);

// <!-- default country -->
e_CountryCode ui_sys_platform_default_country_get(void);
void ui_sys_platform_default_country_set(e_CountryCode country);

// <!-- language -->
string_mgr_language_t ui_sys_platform_language_get(void);
void ui_sys_platform_language_set(string_mgr_language_t language);


// <!-- default language -->
string_mgr_language_t ui_sys_platform_default_language_get(void);
void ui_sys_platform_default_language_set(string_mgr_language_t language);

// <!-- date_format -->
e_DATE_FORMAT ui_sys_platform_date_format_get(void);
void ui_sys_platform_date_format_set(e_DATE_FORMAT format);

// <!-- time_format -->
e_TIME_FORMAT ui_sys_platform_time_format_get(void);
void ui_sys_Platform_time_format_set(e_TIME_FORMAT format);


char * ui_sys_platform_datecode_get(void);

USB_FORCE_SPEED ui_sys_platform_USB_speed_get(void);
void ui_sys_platform_USB_speed_set(USB_FORCE_SPEED speed);


bool ui_sys_platform_engine_is_color_get(void);

#ifdef HAVE_EASY_INSTALL
bool ui_sys_platform_easy_install_disable_get(void);
void ui_sys_platform_easy_install_disable_set(const bool enabled);
bool ui_sys_platform_easy_install_mfg_disable_get(void);
#endif

void ui_sys_platform_rebroadcast_current_status(void);

#endif // ifndef __UI_SYSTEM_INTERFACE_PLATFORM_H__

