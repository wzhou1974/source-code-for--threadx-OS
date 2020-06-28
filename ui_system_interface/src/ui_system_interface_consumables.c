/*
 * ============================================================================
 * Copyright (c) 2013   Marvell International, Ltd. All Rights Reserved
 *
 *                         Marvell Confidential
 * ============================================================================
 */

/** 
 * \file ui_system_interface_consumables.c
 * 
 * \brief Implementations of consumables part of ui_system_interface
 *        
 * 
 **/

#include "ATypes.h"
#include "ui_system_interface_api.h"
#include "ui_system_interface_utils.h"
#include "ui_system_interface_consumables.h"

int ui_sys_consumables_get_num_consumables()
{
    return  consumable_get_num_consumables();
}

consumable_type_t ui_sys_consumables_get_type(int consumable_num)
{
    return  consumable_get_type(consumable_num);
}

consumable_color_t ui_sys_consumables_get_color(int consumable_num)
{
    return  consumable_get_color(consumable_num);
}

consumable_class_t ui_sys_consumables_get_class(int consumable_num)
{
    return  consumable_get_class(consumable_num);
}


bool ui_sys_consumables_is_installed(int consumable_num)
{
    return  consumable_is_installed(consumable_num);
}

consumable_status_t ui_sys_consumables_get_status(int consumable_num)
{
    return  consumable_get_status(consumable_num);
}

consumable_error_t ui_sys_consumables_get_error(int consumable_num)
{
    return  consumable_get_error(consumable_num);
}

error_type_t ui_sys_consumables_get_name(int consumable_num, char* buf, int* size)
{
    return  consumable_get_name(consumable_num, buf, size);
}

error_type_t ui_sys_consumables_get_model(int consumable_num, char* buf, int* size)
{
    return  consumable_get_model(consumable_num, buf, size);
}

int ui_sys_consumables_get_percent_remaining(int consumable_num)
{
    return  consumable_get_percent_remaining(consumable_num);
}

int ui_sys_consumables_get_percent_remaining_stepped(int consumable_num)
{
    return  consumable_get_percent_remaining_stepped(consumable_num);
}


consumable_units_t ui_sys_consumables_get_units(int consumable_num)
{
    return  consumable_get_units(consumable_num);
}

int ui_sys_consumables_get_max_capacity_in_units(int consumable_num)
{
    return  consumable_get_max_capacity_in_units(consumable_num);
}

int ui_sys_consumables_get_level_in_units(int consumable_num)
{
    return  consumable_get_level_in_units(consumable_num);
}

error_type_t ui_sys_consumables_get_serial_number(int consumable_num, char* buf, int* size)
{
    return  consumable_get_serial_number(consumable_num, buf, size);
}

int ui_sys_consumables_get_num_impressions(int consumable_num)
{
    return  consumable_get_num_impressions(consumable_num);
}

error_type_t ui_sys_consumables_get_date_installed(int consumable_num, char* buf, int* size)
{
    return  consumable_get_date_installed(consumable_num, buf, size);
}

error_type_t ui_sys_consumables_get_date_last_used(int consumable_num, char* buf, int* size)
{
    return  consumable_get_date_last_used(consumable_num, buf, size);
}

static void consumables_event_change_cb(char *message, uint32_t consumable_num)
{
    data_block_handle_t d_hndl;
    d_hndl = ui_sys_create_data_block();
    ui_sys_data_block_add_int(d_hndl, consumable_num);

    ui_sys_notify_ui_of_event(CONSUMABLES_MODULE_NAME, message, d_hndl);
}

static void consumables_status_change_cb(uint32_t consumable_num)
{
    consumables_event_change_cb(CONSUMABLES_STATUS_CHANGE, consumable_num);
}

static void consumables_level_change_cb(uint32_t consumable_num)
{
    consumables_event_change_cb(CONSUMABLES_LEVEL_CHANGE, consumable_num);
}

static void consumables_level_change_stepped_cb(uint32_t consumable_num)
{
    consumables_event_change_cb(CONSUMABLES_LEVEL_CHANGE_STEPPED, consumable_num);
}

void ui_sys_consumables_init(void)
{
    ui_sys_register_module_for_status(CONSUMABLES_MODULE_NAME, CONSUMABLES_STATUS_IDLE, NULL);

    consumable_status_change_event_attach(consumables_status_change_cb);
    consumable_level_change_event_attach(consumables_level_change_cb);
    consumable_level_change_stepped_event_attach(consumables_level_change_stepped_cb);
}

