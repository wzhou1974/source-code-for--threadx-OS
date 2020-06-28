/*
 * ============================================================================
 * Copyright (c) 2011-2013   Marvell International, Ltd. All Rights Reserved
 *
 *                         Marvell Confidential
 * ============================================================================
 */

/** 
 * \file ui_system_interface_powermgr.h
 * 
 * \brief 
 * 
 * 
 **/

#ifndef __UI_SYSTEM_INTERFACE_POWERMGR_H__
#define __UI_SYSTEM_INTERFACE_POWERMGR_H__

#include "ATypes.h"
#include <stdint.h>
#include <stdbool.h>


bool ui_sys_power_manager_usbjob_wake_get(void);
void ui_sys_power_manager_usbjob_wake_set(const bool enabled);
bool ui_sys_power_manager_ethernetjob_wake_get(void);
void ui_sys_power_manager_ethernetjob_wake_set(const bool enabled);
bool ui_sys_power_manager_wirelessjob_wake_get(void);
void ui_sys_power_manager_wirelessjob_wake_set(const bool enabled);
bool ui_sys_power_manager_cppress_wake_get(void);
void ui_sys_power_manager_cppress_wake_set(const bool enabled);
bool ui_sys_power_manager_faxactivity_wake_get(void);
void ui_sys_power_manager_faxactivity_wake_set(const bool enabled);
bool ui_sys_power_manager_paper_in_adf_wake_get(void);
void ui_sys_power_manager_paper_in_adf_wake_set(const bool enabled);

bool 	 ui_sys_power_manager_wakeup(void);
uint16_t ui_sys_power_manager_idle_delay_get(void);
void 	 ui_sys_power_manager_idle_delay_set(const uint16_t delay);

// return the delayed minutes
uint16_t ui_sys_power_manager_poweroff_delay_get(void);
// set the delayed parameters(minutes)
void 	 ui_sys_power_manager_poweroff_delay_set(const uint16_t delay);

#endif // ifndef __UI_SYSTEM_INTERFACE_POWERMGR_H__

