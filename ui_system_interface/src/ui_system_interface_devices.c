/*
 * ============================================================================
 * Copyright (c) 2013   Marvell International, Ltd. All Rights Reserved
 *
 *                         Marvell Confidential
 * ============================================================================
 */

/** 
 * \file ui_system_interface_devices.c
 * 
 * \brief Implementations of device part of ui_system_interface
 *        
 * 
 **/




#include "ui_system_interface_devices.h"
#include "lassert.h"

/*
setDateTime (Set the system's current date and time)
  cur_date_time: A Date() object representing the new date and time.
  returns:       True on success, false on failure
*/
bool ui_sys_devices_RTC_setdatetime(const date_time_t * const new_date)
{
	if (rtc_set_time(new_date) == OK)
	{
		return	true;
	}
	else
	{
		return	false;
	}
}

