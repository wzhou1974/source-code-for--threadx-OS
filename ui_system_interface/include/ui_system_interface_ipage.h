/*
 * ============================================================================
 * Copyright (c) 2011-2013   Marvell International, Ltd. All Rights Reserved
 *
 *                         Marvell Confidential
 * ============================================================================
 */

/** 
 * \file ui_system_interface_ipage.h
 * 
 * \brief 
 * 
 * 
 **/

#ifndef __UI_SYSTEM_INTERFACE_IPAGE_H__
#define __UI_SYSTEM_INTERFACE_IPAGE_H__

/**
 * There are unique filenames for the different internal pages. So, if the client want to 
 * print the specific internal page, it only need to select the corresponding file to print.
 *	
 **/
void ui_sys_internal_page_print(char * filename);

#endif // ifndef __UI_SYSTEM_INTERFACE_IPAGE_H__

