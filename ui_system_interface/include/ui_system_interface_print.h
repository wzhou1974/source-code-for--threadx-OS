/*
 * ============================================================================
 * Copyright (c) 2011-2013   Marvell International, Ltd. All Rights Reserved
 *
 *                         Marvell Confidential
 * ============================================================================
 */

/** 
 * \file ui_system_interface_print.h
 * 
 * \brief 
 * 
 * 
 **/

#ifndef __UI_SYSTEM_INTERFACE_PRINT_H__
#define __UI_SYSTEM_INTERFACE_PRINT_H__

#include <stdint.h>
#include <stdbool.h>

#include "printvars_api.h"
#include "logger.h"
#include "ui_system_interface_api.h"

#define PRINT_MODULE_NAME               "print"

#define PRINT_STATUS_IDLE               GENERAL_STATUS_IDLE
#define PRINT_STATUS_INITIALIZING       GENERAL_STATUS_INITIALIZING
#define PRINT_STATUS_PRINTING           "printing"
#define PRINT_STATUS_DOOR_OPEN          "door_open"
#define PRINT_STATUS_REAR_DOOR_OPEN     "rear_door_open"
#define PRINT_STATUS_PAPER_OUT          "paper_out"
#define PRINT_STATUS_PAPER_OUT_ALL      "paper_out_all"
#define PRINT_STATUS_PAPER_JAM          "paper_jam"
#define PRINT_STATUS_PAPER_JAM_PATH     "paper_jam_print_path"
#define PRINT_STATUS_PAPER_JAM_INPUT    "paper_jam_input"
#define PRINT_STATUS_PAPER_JAM_OUTPUT   "paper_jam_output"
#define PRINT_STATUS_MANUAL_DUPLEX      "manual_duplex"
#define PRINT_STATUS_CART_MISSING       "cartridge_missing"
#define PRINT_STATUS_COOLDOWN           "engine_cooldown"
#define PRINT_STATUS_CALIBRATING        "engine_calibrating"
#define PRINT_STATUS_CLEANING           "engine_cleaning"
#define PRINT_STATUS_CANCELING          "canceling_print"
#define PRINT_STATUS_INTERNAL_CLEANING  "engine_cleaning_roller"
#define PRINT_STATUS_MEASURING_TONER    "engine_measuring_toner"

void ui_sys_print_default_papersize_set(const mediasize_t size);
void ui_sys_print_default_papersize_get(mediasize_t *size);
void ui_sys_print_default_papertype_set(const mediatype_t type);
void ui_sys_print_default_papertype_get(mediatype_t *type);
void ui_sys_print_density_cal_power_on_time_set(const uint32_t time_in_minutes);
void ui_sys_print_density_cal_power_on_time_get(uint32_t *time_in_minutes);

#endif // ifndef __UI_SYSTEM_INTERFACE_PRINT_H__

