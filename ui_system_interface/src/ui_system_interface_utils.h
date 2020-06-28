/*
 * ============================================================================
 * Copyright (c) 2013   Marvell International, Ltd. All Rights Reserved
 *
 *                         Marvell Confidential
 * ============================================================================
 */

/** 
 * \file ui_system_interface_utils.h
 * 
 * \brief Utilities and helper routines for UI System Interface.
 * 
 **/

#ifndef __UI_SYSTEM_INTERFACE_UTILS_H__
#define __UI_SYSTEM_INTERFACE_UTILS_H__

/** 
 *  @brief Request a rebroadcast of the current "worst" extension status to the
 *         UI app.
 * 
 */
void rebroadcast_current_ui_sys_status();

#ifdef HAVE_PRINT_SUPPORT
void ui_sys_print_init(void);
#endif

#ifdef HAVE_COPY_SUPPORT
void ui_sys_copy_init(void);
#endif

#ifdef HAVE_PLATFORM
void ui_sys_platform_init(void);
#endif

#ifdef HAVE_FAX
void ui_sys_fax_init(void);
#endif

#ifdef HAVE_COMMON_CONSUMABLES_API
void ui_sys_consumables_init(void);
#endif

#ifdef HAVE_SCAN_SUPPORT
void ui_sys_scan_init(void);
#endif

#ifdef HAVE_NETWORK
void ui_sys_network_init(void);
#endif	

#ifdef HAVE_ETHERNET
void ui_sys_wired_network_init(void);
#endif

#ifdef HAVE_WIRELESS
void ui_sys_wireless_init(void);
#endif

#ifdef HAVE_UI_SYSTEM_INTERFACE_TEST
void ui_sys_test_init();
#endif // #ifdef HAVE_UI_SYSTEM_INTERFACE_TEST


#endif // ifndef __UI_SYSTEM_INTERFACE_UTILS_H__
