/*
 * ============================================================================
 * Copyright (c) 2013   Marvell International, Ltd. All Rights Reserved
 *
 *                         Marvell Confidential
 * ============================================================================
 */

/** 
 * \file ui_system_interface_ipage.c
 * 
 * \brief Implementations of internal page part of ui_system_interface
 *        
 * 
 **/

#include "ui_system_interface_ipage.h"
#include "internal_page.h"
#include "lassert.h"

void ui_sys_internal_page_print(char * filename)
{
	internal_page_print(filename);
}
