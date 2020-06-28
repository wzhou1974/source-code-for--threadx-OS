/*
 * ============================================================================
 * Copyright (c) 2013   Marvell International, Ltd. All Rights Reserved
 *
 *                         Marvell Confidential
 * ============================================================================
 */

/** 
 * \file ui_system_interface_wirednetwork.c
 * 
 * \brief Implementations of ethernet part of ui_system_interface
 *        
 * 
 **/



#include "ui_system_interface_network.h"
#include "ui_system_interface_api.h"
#include "ui_system_interface_utils.h"

#include "memAPI.h"
#include "net_eth_vars.h"
#include "eth_mac_api.h"
#include "net_iface_vars.h"
#include "lassert.h"
#include "dprintf.h"
#include "trsocket.h"
#include "net_iface_api.h"
#include "net_link_api.h"
#include "net_dns_api.h"

void ui_sys_wired_network_get_link_status(bool * status)
{
    eth_mac_link_status_t link_status;
    bool link_up = false;

    net_eth_get_var_link_status(&link_status);

    if (link_status != ETH_MAC_LINK_STATUS_DOWN)
    {
        link_up = true;                
    }    
	*status = link_up;
}

void ui_sys_wired_network_get_link_speed(eth_mac_link_config_t * speed)
{
	ASSERT(speed != NULL);	
	error_type_t	status;

    status = net_eth_get_var_link_config(speed);
    ASSERT(status == NET_ETH_OK);	
}

void ui_sys_wired_network_get_mac_address(uint8_t * phys_addr, size_t size)
{
	XASSERT(size >= TM_ETHERNET_PHY_ADDR_LEN, size);

	ttUserInterface iface;

	iface = tfInterfaceNameToDevice(NET_IFACE_INTERFACE_NAME);
	tfGetIfPhysAddr(iface, (char *)phys_addr, TM_ETHERNET_PHY_ADDR_LEN);	
}


bool ui_sys_wired_network_restore_default(void)
{
    error_type_t status;

	//Since we are attempting to restore defaults, set link speed to Auto as well
	net_eth_set_var_link_config((eth_mac_link_config_t)ETH_MAC_LINK_CONFIG_AUTO);

	status = net_iface_var_restore_defaults(if_nametoindex(NET_IFACE_INTERFACE_NAME), NET_IFACE_VAR_ALL);

	if(status == OK)
	{
		return	true;
	}
	else
	{
		return 	false;
	}
}

bool ui_sys_wired_network_set_link_speed(eth_mac_link_config_t speed)
{
    error_type_t	status;	
	status = net_eth_set_var_link_config(speed);
	if(status == OK)
	{
		return	true;
	}
	else
	{
		return	false;
	}	
}



void ui_sys_wired_network_init(void)
{
    // Currently, do nothing
}

