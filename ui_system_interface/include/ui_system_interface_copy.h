/*
 * ============================================================================
 * Copyright (c) 2011-2013   Marvell International, Ltd. All Rights Reserved
 *
 *                         Marvell Confidential
 * ============================================================================
 */

/** 
 * \file ui_system_interface_copy.h
 * 
 * \brief 
 * 
 * 
 **/

#ifndef __UI_SYSTEM_INETRFACE_COPY_H__
#define __UI_SYSTEM_INETRFACE_COPY_H__

#include <stdint.h>
#include <stdbool.h>
#include "print_job_types.h"
#include "response_subject.h"
#include "copyapp_api.h"


/**
 * module name related to copy module
 */
#define COPY_MODULE_NAME            "copy"
#define IDCOPY_MODULE_NAME          "idcopy"
#define CONTINUOUSCOPY_MODULE_NAME  "contcopy"
#define CONTINUOUSSCAN_MODULE_NAME  "contscan"

/**
 * status message related to copy module
 */

#define COPY_STATUS_IDLE            GENERAL_STATUS_IDLE
#define IDCOPY_STATUS_IDLE          GENERAL_STATUS_IDLE
#define CONTINUOUSSCAN_STATUS_IDLE  GENERAL_STATUS_IDLE
#define CONTINUOUSCOPY_STATUS_IDLE  GENERAL_STATUS_IDLE
#define COPY_STATUS_BUSY            "device-busy-copy-later"
#define COPY_STATUS_PENDING         "copy-pending"
#define COPY_SCAN_STATUS_RUNNING    "scan-running"
#define COPY_STATUS_PAGECOUNT       "copy-page-count"
#define COPY_STATUS_USER_INPUT      "copy-user-input"
#define COPY_STATUS_CANCELING       "canceling_copy"

/*
  restore the copy config nvram settings to factory default values
*/
void ui_sys_copy_settings_setdefaults(void);

/*
  save the copy config settings passed in copyConfig to nvram 
*/
void ui_sys_copy_settings_set(CopyConf *copyConfig);

/*
  execute a copy job using the copy config settings passed in copyConfig
*/
void ui_sys_copy_startjob(CopyConf *copyConfig);


#ifdef  HAVE_ID_COPY
/*
  execute a copyID job using the copy config settings passed in copyConfig
*/
void ui_sys_copy_copyid_jobstart(CopyConf *copyConfig);

/*
  Issue "Next" to copy ID job using
*/
void ui_sys_copy_copyid_jobnext();

/*
  Issue "Finish" to copy ID job using
*/
void ui_sys_copy_copyid_jobfinish();

/*
  Issue "Cancel" to copy ID job using
*/
void ui_sys_copy_copyid_jobcancel();

#endif

/*
  Retrieve the current copy config values stored in nvram
    returns:  CopyConfig object populated with the contents of that entry.
*/
void ui_sys_copy_settings_get(CopyConf *copyConfig);

/*
  set copy parameters in non-volatile memory
*/
void ui_sys_copy_settings_copy_quality_set(e_COPYQUALITY quality);

void ui_sys_copy_settings_colormode_set(copy_cmode_t ccmode);
void ui_sys_copy_settings_resize_set(e_COPYRESIZE resize_var, uint32_t custom_resize);
void ui_sys_copy_settings_numcopies_set(int num_copies);
void ui_sys_copy_settings_brightness_set(int brightness);
void ui_sys_copy_settings_print_quality_set(PRINT_QUALITY print_quality);
void ui_sys_copy_settings_inputtray_set(trayenum_t input_tray);
void ui_sys_copy_settings_collation_set(BOOL enable);
void ui_sys_copy_settings_print_duplex_set(BOOL enable);
void ui_sys_copy_settings_scan_duplex_set(BOOL enable);
void ui_sys_copy_settings_color_tint_set(int r, int g, int b);
void ui_sys_copy_settings_background_set(int background);
void ui_sys_copy_settings_sharpness_set(int sharpness);
void ui_sys_copy_settings_contrast_set(int contrast);
void ui_sys_copy_settings_papersize_set(mediasize_t paper_size);

void ui_sys_copy_start_continuous_scan();
void ui_sys_copy_stop_continuous_scan();

void ui_sys_copy_start_continuous_copy();
void ui_sys_copy_stop_continuous_copy();

#endif // ifndef __UI_SYSTEM_INETRFACE_COPY_H__

