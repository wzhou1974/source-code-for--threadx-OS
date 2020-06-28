/*
 * ============================================================================
 * Copyright (c) 2013   Marvell International, Ltd. All Rights Reserved
 *
 *                         Marvell Confidential
 * ============================================================================
 */

/** 
 * \file ui_system_interface_platform.c
 * 
 * \brief Implementations of platform part of ui_system_interface
 *        
 * 
 **/
#include "memAPI.h"
#include "nvram_api.h"
#include "sys_init_api.h"
#include "io_device.h"
#include "logger.h"
#include "string.h"
#include "platform_api.h"
#include "dprintf.h"
#include "string_mgr_api.h"
#include "event_observer.h"
#include "response_subject.h"

#include "platform.h"
#include "usbmsc_device_api.h"
#include "ACL_burn_flash_event_api.h"
#include "string_mgr_api.h"     
#include "ui_system_interface_api.h"
#include "ui_system_interface_utils.h"
#include "ui_system_interface_platform.h"
#include "ui_system_interface_oem_platform.h"
#include "lassert.h"

#include "oid_api.h"

#define BURNFLASH_EVENT_ID                  0xFA70306F
#define PLATFORM_EVENT_ID                   0xC74D0D3C
#define STRING_MGR_EVENT_ID                 0xD85E1E4D

static char *ui_sys_platform_cur_state = PLATFORM_STATUS_IDLE;

/*
Reset printer
*/
void ui_sys_platform_reset(void)
{
    // command all threads to die, delay to allow time to cleanup, then reboot
    SysShutdownSystem();
    tx_thread_sleep( 500 );
    PlatformReset();
}


/*
restoreFactoryDefaults (Set the system's parameters to factory defaults)
*/
void ui_sys_platform_NVRAM_restore_factory_defaults(void)
{
    nvram_reset_all_vars( NVRAM_FACTORY_DEFAULTS );
}

/*
restoreDefaults (Set the system's parameters to defaults)
*/
void ui_sys_platform_NVRAM_restore_defaults(void)
{
    nvram_reset_all_vars( NVRAM_LANG_DEFAULTS );
}

#ifdef HAVE_NAND
/*
  cleanFS (Restore the file system to a clean [empty] state)
    returns: Nothing
    WARNING: Erases all files and causes machine to reboot -->
*/
void ui_sys_platform_filesystem_clean_fs(void)
{
    void nand_clean_fs(bool nuclear);
    nand_clean_fs(false);
}
#endif

// <!-- country -->
e_CountryCode ui_sys_platform_country_get(void)
{
    e_CountryCode country;
    plat_get_current_country_code(&country);

    return country;
}

void ui_sys_platform_country_set(e_CountryCode country)
{
    plat_set_current_country_code(country);
}

// <!-- password -->
// Noted: The caller has the responsibility to free the returned string buffer
char * ui_sys_platform_password_get(void)
{        
    char *password = (char *)MEM_MALLOC(PLATFORM_PASSWORD_STRING_MAX_LEN);
    ASSERT(password != NULL);
    
    platvars_get_sys_password( password, PLATFORM_PASSWORD_STRING_MAX_LEN - 1 );
    return  password;
}

bool ui_sys_platform_password_set(char * password)
{
    platvars_set_sys_password( password, strlen (password) );
    return  true;
}

// <!-- default country -->
e_CountryCode ui_sys_platform_default_country_get(void)
{
    e_CountryCode country;
    plat_get_default_country_code(&country);

    return  country;
}

void ui_sys_platform_default_country_set(e_CountryCode country)
{
    plat_set_default_country_code(country);
}

// <!-- language -->
string_mgr_language_t ui_sys_platform_language_get(void)
{
    string_mgr_language_t language;
    string_mgr_get_language(&language);

    return  language;
}

void ui_sys_platform_language_set(string_mgr_language_t language)
{
    string_mgr_set_language(language);
}


// <!-- default language -->
string_mgr_language_t ui_sys_platform_default_language_get(void)
{
    string_mgr_language_t language;
    string_mgr_get_default_language(&language);

    return  language;
}

void ui_sys_platform_default_language_set(string_mgr_language_t language)
{
    string_mgr_set_default_language(language);
}


// <!-- date_format -->
e_DATE_FORMAT ui_sys_platform_date_format_get(void)
{
    e_DATE_FORMAT format;
    platvars_get_current_date_format(&format);

    return  format;
}
void ui_sys_platform_date_format_set(e_DATE_FORMAT format)
{
    platvars_set_current_date_format(format);
}

// <!-- time_format -->
e_TIME_FORMAT ui_sys_platform_time_format_get(void)
{
    e_TIME_FORMAT format;
    platvars_get_current_time_format(&format);

    return  format;
}
void ui_sys_Platform_time_format_set(e_TIME_FORMAT format)
{
    platvars_set_current_time_format(format);
}

char * ui_sys_platform_datecode_get(void)
{
    char *result = NULL;

    platvars_get_fw_version( &result );
    DPRINTF( (DBG_LOUD | DBG_OUTPUT),("%s datecode: %s!\n", __FILE__, result)); 

    return result;
}

USB_FORCE_SPEED ui_sys_platform_USB_speed_get(void)
{
    USB_FORCE_SPEED val;

    USBGetVarForceFullSpeed((uint8_t *)&val);
    XASSERT(val == USB_SPEED_FULL1 || val == USB_SPEED_HIGH1, val);
    return  val;    
}

void ui_sys_platform_USB_speed_set(USB_FORCE_SPEED speed)
{
    XASSERT(speed == USB_SPEED_FULL1 || speed == USB_SPEED_HIGH1, speed);
    USBSetVarForceFullSpeed((uint8_t *)&speed);
}

bool ui_sys_platform_engine_is_color_get(void)
{
#ifdef COLOR_ENG
    return  true;
#else // #ifndef COLOR_ENG
    return  false;
#endif // #ifdef COLOR_ENG
}

#ifdef HAVE_EASY_INSTALL

bool ui_sys_platform_easy_install_disable_get(void)
{
    uint8_t cur_val;
    error_type_t e_res;

    // get current value SmartInstall disable flag
    e_res = usbmsc_get_var_disable_SI(&cur_val);
    ASSERT(OK == e_res);
    
    return  cur_val != 0 ? true : false;
}

void ui_sys_platform_easy_install_disable_set(const bool enabled)
{
    uint8_t cur_val;
    error_type_t e_res;

    // set current value SmartInstall disable flag
    cur_val = (enabled == true ? 1 : 0);
    e_res = usbmsc_set_var_disable_SI(&cur_val);
    ASSERT(OK == e_res);
}

bool ui_sys_platform_easy_install_mfg_disable_get(void)
{
    uint32_t nv_flags;
    error_type_t e_res;

    // get current value SmartInstall manufacturing disable flag
    e_res = platvars_get_nv_flags( &nv_flags );
    ASSERT(OK == e_res);

    return  nv_flags & NV_FLAGS_DISABLE_SMARTINSTALL;
}

#endif

// rebroadcast current "worst" ui_system_interface status
void ui_sys_platform_rebroadcast_current_status(void)
{
    rebroadcast_current_ui_sys_status();
}

static void string_mgr_event_cb(Observer_t *o, Subject_t *s)
{
     // Sanity check of the observer
    ASSERT( VALIDATE_OBSERVER( o, STRING_MGR_EVENT_ID ) );

    if (s != NULL)
    {       
        uint32_t cur_var_update = StringMgrVar_Get_Var(s);

        if (cur_var_update == STRING_MGR_CURRENT_LANGUAGE)
        {
            ui_sys_notify_ui_of_event(SYSTEM_MODULE_NAME, STRING_MGR_EVENT_CUR_LANG_CHANGED, NULL);              
        }
        else if (cur_var_update == STRING_MGR_DEFAULT_LANGUAGE)
        {
            ui_sys_notify_ui_of_event(SYSTEM_MODULE_NAME, STRING_MGR_EVENT_DEFAULT_LANG_CHANGED, NULL);
        }
    }
}

// observer to process burnflash events
static void burnflash_event_cb(Observer_t *observer, Subject_t *s)
{
    // sanity check of the observer
    ASSERT( VALIDATE_OBSERVER(observer, BURNFLASH_EVENT_ID));
    
    acl_burn_flash_event_t burn_flash_event = ACL_Get_burn_flash_event(s);
    switch (burn_flash_event)
    {
    case BURN_FLASH_EVENT_PROGRAMMING_FW:
        // programming the actual firmware so we're committed to a real upgrade 
        ui_sys_platform_cur_state = PLATFORM_STATUS_BURNFLASH_PROGRAMMING;
        ui_sys_notify_ui_of_status(PLATFORM_MODULE_NAME, PLATFORM_STATUS_BURNFLASH_PROGRAMMING, NULL);
        break;

    case BURN_FLASH_EVENT_UPGRADE_REBOOTING:
        // finished programming everything we're going to program before rebooting 
        ui_sys_platform_cur_state = PLATFORM_STATUS_BURNFLASH_COMPLETE;
        ui_sys_notify_ui_of_status(PLATFORM_MODULE_NAME, PLATFORM_STATUS_BURNFLASH_COMPLETE, NULL);
        break;

    default:
        break;
    }
}

void ui_sys_platform_init(void)
{       
    // register system with ui_system_interface status support
    ui_sys_register_module_for_status(SYSTEM_MODULE_NAME, SYSTEM_STATUS_IDLE, NULL);
    ui_sys_register_module_for_status(PLATFORM_MODULE_NAME, PLATFORM_STATUS_IDLE, NULL);

    // register as platform subject observer
    PlatformVar_AttachObserver_Change(Observer_Constructor(PLATFORM_EVENT_ID, ui_sys_platform_oem_event_cb));

    // register as string_mgr subject observer
    StringMgrVar_AttachObserver_Change(Observer_Constructor(STRING_MGR_EVENT_ID, string_mgr_event_cb));

    // register as burnflash subject observer
    ACL_AttachObserver_burn_flash_event(Observer_Constructor(BURNFLASH_EVENT_ID, burnflash_event_cb));
}


