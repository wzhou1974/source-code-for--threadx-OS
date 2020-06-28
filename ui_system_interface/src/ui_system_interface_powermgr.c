/*
 * ============================================================================
 * Copyright (c) 2013   Marvell International, Ltd. All Rights Reserved
 *
 *                         Marvell Confidential
 * ============================================================================
 */

/** 
 * \file ui_system_interface_powermgr.c
 * 
 * \brief Implementations of power manager part of ui_system_interface
 *        
 * 
 **/



#include "ui_system_interface_powermgr.h"
#include "lassert.h"
#include "pwr_mgr_api.h"
#include "pwr_schema_api.h"

bool ui_sys_power_manager_usbjob_wake_get(void)
{
	error_type_t	result;
	uint32_t		event;

	result = pwr_schema_get_wake_on_events(&event);
	ASSERT(result == OK);

	return event & PWR_SCHEMA_WAKE_ON_USB_JOB ? true : false;
}

void ui_sys_power_manager_usbjob_wake_set(const bool enabled)
{
	error_type_t	result;
	uint32_t		event;

	result = pwr_schema_get_wake_on_events(&event);
	ASSERT(result == OK);

	if(enabled)
	{
		pwr_schema_set_wake_on_events(event | PWR_SCHEMA_WAKE_ON_USB_JOB); 	
	}
	else
	{
		pwr_schema_set_wake_on_events(event & ~PWR_SCHEMA_WAKE_ON_USB_JOB); 	
	}

	return;
}

bool ui_sys_power_manager_ethernetjob_wake_get(void)
{
	error_type_t	result;
	uint32_t		event;

	result = pwr_schema_get_wake_on_events(&event);
	ASSERT(result == OK);

	return event & PWR_SCHEMA_WAKE_ON_ETHERNET_JOB ? true : false;
}

void ui_sys_power_manager_ethernetjob_wake_set(const bool enabled)
{
	error_type_t	result;
	uint32_t		event;

	result = pwr_schema_get_wake_on_events(&event);
	ASSERT(result == OK);

	if(enabled)
	{
		pwr_schema_set_wake_on_events(event | PWR_SCHEMA_WAKE_ON_ETHERNET_JOB); 	
	}
	else
	{
		pwr_schema_set_wake_on_events(event & ~PWR_SCHEMA_WAKE_ON_ETHERNET_JOB); 	
	}

	return;	
}

bool ui_sys_power_manager_wirelessjob_wake_get(void)
{
	error_type_t	result;
	uint32_t		event;

	result = pwr_schema_get_wake_on_events(&event);
	ASSERT(result == OK);

	return event & PWR_SCHEMA_WAKE_ON_WIRELESS_JOB ? true : false;

}

void ui_sys_power_manager_wirelessjob_wake_set(const bool enabled)
{
	error_type_t	result;
	uint32_t		event;

	result = pwr_schema_get_wake_on_events(&event);
	ASSERT(result == OK);

	if(enabled)
	{
		pwr_schema_set_wake_on_events(event | PWR_SCHEMA_WAKE_ON_WIRELESS_JOB); 	
	}
	else
	{
		pwr_schema_set_wake_on_events(event & ~PWR_SCHEMA_WAKE_ON_WIRELESS_JOB); 	
	}

	return;	
}

bool ui_sys_power_manager_cppress_wake_get(void)
{
	error_type_t	result;
	uint32_t		event;

	result = pwr_schema_get_wake_on_events(&event);
	ASSERT(result == OK);

	return event & PWR_SCHEMA_WAKE_ON_CP_PRESS ? true : false;

}

void ui_sys_power_manager_cppress_wake_set(const bool enabled)
{
	error_type_t	result;
	uint32_t		event;

	result = pwr_schema_get_wake_on_events(&event);
	ASSERT(result == OK);

	if(enabled)
	{
		pwr_schema_set_wake_on_events(event | PWR_SCHEMA_WAKE_ON_CP_PRESS); 	
	}
	else
	{
		pwr_schema_set_wake_on_events(event & ~PWR_SCHEMA_WAKE_ON_CP_PRESS); 	
	}

	return;	

}

bool ui_sys_power_manager_faxactivity_wake_get(void)
{
	error_type_t	result;
	uint32_t		event;

	result = pwr_schema_get_wake_on_events(&event);
	ASSERT(result == OK);

	return event & PWR_SCHEMA_WAKE_ON_FAX_ACTIVITY ? true : false;
}

void ui_sys_power_manager_faxactivity_wake_set(const bool enabled)
{
	error_type_t	result;
	uint32_t		event;

	result = pwr_schema_get_wake_on_events(&event);
	ASSERT(result == OK);

	if(enabled)
	{
		pwr_schema_set_wake_on_events(event | PWR_SCHEMA_WAKE_ON_FAX_ACTIVITY); 	
	}
	else
	{
		pwr_schema_set_wake_on_events(event & ~PWR_SCHEMA_WAKE_ON_FAX_ACTIVITY); 	
	}

	return;	

}

bool ui_sys_power_manager_paper_in_adf_wake_get(void)
{
	error_type_t	result;
	uint32_t		event;

	result = pwr_schema_get_wake_on_events(&event);
	ASSERT(result == OK);

	return event & PWR_SCHEMA_WAKE_ON_PAPER_IN_ADF ? true : false;
}

void ui_sys_power_manager_paper_in_adf_wake_set(const bool enabled)
{
	error_type_t	result;
	uint32_t		event;

	result = pwr_schema_get_wake_on_events(&event);
	ASSERT(result == OK);

	if(enabled)
	{
		pwr_schema_set_wake_on_events(event | PWR_SCHEMA_WAKE_ON_PAPER_IN_ADF); 	
	}
	else
	{
		pwr_schema_set_wake_on_events(event & ~PWR_SCHEMA_WAKE_ON_PAPER_IN_ADF); 	
	}

	return;	

}

/*
  WakeUp (Wake up the system)
    returns:     true on success
*/
bool ui_sys_power_manager_wakeup(void)
{
    bool result = true;

    result = pwr_mgr_go_active_nowait(PWRMGR_UID_FULL_WAKE);

    return	result;
}


uint16_t ui_sys_power_manager_idle_delay_get(void)
{
	return	pwr_mgr_get_idle_time();
}

void ui_sys_power_manager_idle_delay_set(const uint16_t delay)
{
	pwr_mgr_set_idle_time(delay);
}

uint16_t ui_sys_power_manager_poweroff_delay_get(void)
{
	uint16_t 		delay_minutes;
	error_type_t	result;

	result = pwr_schema_get_power_off_delay_in_minutes(&delay_minutes);
	ASSERT(result == OK);	
	
	return	delay_minutes;	
}

void ui_sys_power_manager_poweroff_delay_set(const uint16_t delay_minutes)
{    
	error_type_t	result;	

	result = pwr_schema_set_power_off_delay_in_minutes(delay_minutes);
	ASSERT(result == OK);
	
	return;			
}





