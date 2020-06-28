/*
 * ============================================================================
 * Copyright (c) 2011-2013   Marvell International, Ltd. All Rights Reserved
 *
 *                         Marvell Confidential
 * ============================================================================
 */

/** 
 * \file ui_system_interface_scan.h
 * 
 * \brief 
 * 
 * 
 **/

#ifndef __UI_SYSTEM_INTERFACE_SCAN_H__
#define __UI_SYSTEM_INTERFACE_SCAN_H__

#include <stdint.h>
#include <stdbool.h>

#include "scantypes.h"


#define SCAN_MODULE_NAME            "scan"

#define SCAN_STATUS_IDLE            GENERAL_STATUS_IDLE
#define SCAN_STATUS_INITIALIZING    GENERAL_STATUS_INITIALIZING
#define SCAN_EVENT_PAPER_PRESENT    "paper_present"
#define SCAN_EVENT_CAL_GRAPH_END    "cal_graph_end"
#define SCAN_STATUS_OFFLINE         "scanner_error"
#define SCAN_STATUS_RUNNING         "scanning"
#define SCAN_STATUS_CALBRATING      "calibrating"
#define SCAN_STATUS_CANCELING       "canceling_scan"
#define SCAN_STATUS_PAPER_JAM       "paper_jam"
#define SCAN_STATUS_PAPER_NOPICK    "miss_pick"
#define SCAN_STATUS_PP_FAILURE      "paper_present_failure"
#define SCAN_STATUS_BUSY            "device-busy-scan-later"


bool ui_sys_scan_ADF_paper_present_get(void);
bool ui_sys_scan_ADF_paper_present_disabled_get(void);

#endif // ifndef __UI_SYSTEM_INTERFACE_SCAN_H__

