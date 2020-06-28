/*
 * ============================================================================
 * Copyright (c) 2011-2013   Marvell International, Ltd. All Rights Reserved
 *
 *                         Marvell Confidential
 * ============================================================================
 */

/** 
 * \file ui_system_interface_consumables.h
 * 
 * \brief 
 * 
 * 
 **/

#ifndef __UI_SYSTEM_INTERFACE_CONSUMABLES_H__
#define __UI_SYSTEM_INTERFACE_CONSUMABLES_H__

#include <stdint.h>
#include <stdbool.h>
#include "consumables_common_api.h"


#define CONSUMABLES_MODULE_NAME             "consumables"

#define CONSUMABLES_STATUS_IDLE             GENERAL_STATUS_IDLE

#define CONSUMABLES_STATUS_CHANGE           "status_change"
#define CONSUMABLES_LEVEL_CHANGE            "level_change"
#define CONSUMABLES_LEVEL_CHANGE_STEPPED    "level_change_stepped"


/**
 * \brief Get the number of consumables supported by the device.
 *
 * \retval int The number of consumables supported by the device.
 *
 **/
int ui_sys_consumables_get_num_consumables();

/**
 * \brief Get the type of the specfied consumable.
 *  
 * \param[in]  consumable_num  The number of the consumable.
 *
 * \retval consumable_type_t The type of the consumable.
 *
 **/
consumable_type_t ui_sys_consumables_get_type(int consumable_num);

/**
 * \brief Get the color of the specfied consumable.
 *  
 * \param[in]  consumable_num  The number of the consumable.
 *
 * \retval consumable_color_t The color of the consumable.
 *
 **/
consumable_color_t ui_sys_consumables_get_color(int consumable_num);

/**
 * \brief Get the class of the specfied consumable.
 *  
 * \param[in]  consumable_num  The number of the consumable.
 *
 * \retval consumable_class_t The class of the consumable.
 *
 **/
consumable_class_t ui_sys_consumables_get_class(int consumable_num);


/**
 * \brief Get the installed state of the consumable.
 *  
 * \param[in]  consumable_num  The number of the consumable.
 *
 * \retval bool True if installed; else false.
 *  
 * Note that consumable_is_installed is a convenience wrapper 
 * around consumable_get_status. consuamble_is_installed 
 * returns true when consumable_get_status != REMOVED. 
 **/
bool ui_sys_consumables_is_installed(int consumable_num);

/**
 * \brief Get the status of the specfied consumable.
 *  
 * \param[in]  consumable_num  The number of the consumable.
 *
 * \retval consumable_status_t The status of the consumable. 
 *  
 * If the status indicates that the consumable is in error condition 
 * call consumable_get_error for details of the error. 
 *
 **/
consumable_status_t ui_sys_consumables_get_status(int consumable_num);

/**
 * \brief Get the error information of the specfied consumable.
 *  
 * \param[in]  consumable_num  The number of the consumable.
 *
 * \retval consumable_error_t The error condition of the consumable. 
 *  
 * If the cartridge is not in an error state CONSUMABLE_ERROR_NONE is returned. 
 * Otherwise an oem defined error is returned as specified in 
 * consumables_config.h. 
 *
 **/
consumable_error_t ui_sys_consumables_get_error(int consumable_num);

/**
 * \brief Get the name of the specfied consumable as a 
 * null terminated string.
 *  
 * \param[in]  consumable_num  The number of the consumable. 
 *  
 * \param[out] buf Buffer to be filled with the consumable name. 
 *  
 * \param[in][out] size On entry contains the size of the passed in 
 * buffer specfied by buf.  On return contains the size of the name 
 * including the terminating NULL.
 *
 * \retval error_type_t OK on success 
 *                      FAIL if buf is too small.
 *  
 * If the passed in buf is too small FAIL will be returned and the 
 * size parameter will be set to the size required. 
 *  
 **/
error_type_t ui_sys_consumables_get_name(int consumable_num, char* buf, int* size);

/**
 * \brief Get the model of the specfied consumable as a 
 * null terminated string.
 *  
 * \param[in]  consumable_num  The number of the consumable. 
 *  
 * \param[out] buf Buffer to be filled with the consumable model. 
 *  
 * \param[in][out] size On entry contains the size of the passed in 
 * buffer specfied by buf.  On return contains the size of the string 
 * including the terminating NULL.
 *
 * \retval error_type_t OK on success 
 *                      FAIL if buf is too small.
 *  
 * If the passed in buf is too small FAIL will be returned and the 
 * size parameter will be set to the size required. 
 *  
 **/
error_type_t ui_sys_consumables_get_model(int consumable_num, char* buf, int* size);

/**
 * \brief Get the percent of the consumable remaining.  This is the 
 * the accurate percentage in units of 1%. 
 *  
 * \param[in]  consumable_num  The number of the consumable.
 *
 * \retval int The percent of the consumable remaining [0..100]. 
 * -1 is returned if the level cannot be determined. 
 *
 **/
int ui_sys_consumables_get_percent_remaining(int consumable_num);

/**
 * \brief Get the stepped percent of the consumable remaining.  This is the 
 * the percentage is in units of 1%, but is stepped down according 
 * to an algorithm determined by the oem implementer.  For example, 
 * the percent may step down in units of 10%, 20% or some other 
 * value desired by the oem. 
 *  
 * \param[in]  consumable_num  The number of the consumable.
 *
 * \retval int The percent of the consumable remaining [0..100]. 
 * -1 is returned if the level cannot be determined. 
 *
 **/
int ui_sys_consumables_get_percent_remaining_stepped(int consumable_num);


/**
 * \brief Get the actual units of measurement of the consumable.
 *  
 * \param[in]  consumable_num  The number of the consumable.
 *
 * \retval consumable_units_t The units tracked by the consumable.
 *
 **/
consumable_units_t ui_sys_consumables_get_units(int consumable_num);

/**
 * \brief Get the maximum capacity of the consumable in units 
 * defined by the consumable_get_units( ) routine.
 *  
 * \param[in]  consumable_num  The number of the consumable.
 *
 * \retval int The maximum capacity of the consumable. 
 * -1 means other and indicates that there are no restrictions on this parameter. 
 * -2 means unknown. 
 *
 **/
int ui_sys_consumables_get_max_capacity_in_units(int consumable_num);

/**
 * \brief Get the current level of the consumable in units 
 * defined by the consumable_get_units( ) routine.
 *  
 * \param[in]  consumable_num  The number of the consumable.
 *
 * \retval int The current level of the consumable. 
 * -1 means other and indicates that there are no restrictions on this parameter. 
 * -2 means unknown. 
 * -3 means that the consumable is not out, but no other level information can be determined. 
 *
 **/
int ui_sys_consumables_get_level_in_units(int consumable_num);

/**
 * \brief Get the serial number of the specfied consumable as a 
 * null terminated string.
 *  
 * \param[in]  consumable_num  The number of the consumable. 
 *  
 * \param[out] buf Buffer to be filled with the consumable serial number. 
 *  
 * \param[in][out] size On entry contains the size of the passed in 
 * buffer specfied by buf.  On return contains the size of the string
 * including the terminating NULL.
 *
 * \retval error_type_t OK on success 
 *                      FAIL if buf is too small.
 *  
 * If the passed in buf is too small FAIL will be returned and the 
 * size parameter will be set to the size required. 
 *  
 **/
error_type_t ui_sys_consumables_get_serial_number(int consumable_num, char* buf, int* size);

/**
 * \brief Get the number of impressions of the consumable.
 *  
 * \param[in]  consumable_num  The number of the consumable.
 *
 * \retval int The number of impressions of the consumable. 
 * -1 means unknown
 *
 **/
int ui_sys_consumables_get_num_impressions(int consumable_num);

/**
 * \brief Get the date the consumable was first installed as a
 * null terminated string of the form YYYYMMDD.
 *  
 * \param[in]  consumable_num  The number of the consumable. 
 *  
 * \param[out] buf Buffer to be filled with the install date. 
 *  
 * \param[in][out] size On entry contains the size of the passed in 
 * buffer specfied by buf.  On return contains the size of the returned string 
 * including the terminating NULL.
 *
 * \retval error_type_t OK on success 
 *                      FAIL if buf is too small.
 *                      CONSUMABLE_ERROR_UNKNOWN if the date is not known
 *  
 * If the passed in buf is too small FAIL will be returned and the 
 * size parameter will be set to the size required. 
 *  
 **/
error_type_t ui_sys_consumables_get_date_installed(int consumable_num, char* buf, int* size);

/**
 * \brief Get the date the consumable was last used as a
 * null terminated string of the form YYYYMMDD.
 *  
 * \param[in]  consumable_num  The number of the consumable. 
 *  
 * \param[out] buf Buffer to be filled with the last used date. 
 *  
 * \param[in][out] size On entry contains the size of the passed in 
 * buffer specfied by buf.  On return contains the size of the returned string 
 * including the terminating NULL.
 *
 * \retval error_type_t OK on success 
 *                      FAIL if buf is too small.
 *                      CONSUMABLE_ERROR_UNKNOWN if the date is not known
 *  
 * If the passed in buf is too small FAIL will be returned and the 
 * size parameter will be set to the size required. 
 *  
 **/
error_type_t ui_sys_consumables_get_date_last_used(int consumable_num, char* buf, int* size);


#endif // ifndef __UI_SYSTEM_INTERFACE_CONSUMABLES_H__

