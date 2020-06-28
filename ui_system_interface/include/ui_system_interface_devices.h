/*
 * ============================================================================
 * Copyright (c) 2011-2013   Marvell International, Ltd. All Rights Reserved
 *
 *                         Marvell Confidential
 * ============================================================================
 */

/** 
 * \file ui_system_interface_wireless.h
 * 
 * \brief 
 * 
 * 
 **/

#ifndef __UI_SYSTEM_INTERFACE_DEVICES_H__
#define __UI_SYSTEM_INTERFACE_DEVICES_H__

#include <stdint.h>
#include <stdbool.h>
#include "rtc_api.h"

/*
setDateTime (Set the system's current date and time)
  cur_date_time: A Date() object representing the new date and time.
  returns:       True on success, false on failure
*/
bool ui_sys_devices_RTC_setdatetime(const date_time_t * const new_date);


#endif // ifndef __UI_SYSTEM_INTERFACE_DEVICES_H__

