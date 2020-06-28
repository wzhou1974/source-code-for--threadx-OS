/*
 * ============================================================================
 * Copyright (c) 2013   Marvell International, Ltd. All Rights Reserved
 *
 *                         Marvell Confidential
 * ============================================================================
 */

/** 
 * \file ui_system_interface_network.c
 * 
 * \brief Implementations of network part of ui_system_interface
 *        
 * 
 **/



#include "ui_system_interface_network.h"
#include "ui_system_interface_api.h"
#include "ui_system_interface_utils.h"
#include "memAPI.h"
#include "logger.h"
#include "string.h"
#include "dprintf.h"
#include "memAPI.h"
#include "tx_api.h"
#include "lassert.h"
#include "net_dns_api.h"
#include "net_link_api.h"

#ifdef HAVE_MDNS
#include "net_mdns_vars.h"
#endif

#ifdef HAVE_SLP
#include "net_slp_vars.h"
#endif

#ifdef HAVE_WSD
#include "net_wsd_vars.h"
#endif


static unsigned long new_IP_address = 0;
static unsigned long new_subnet_mask = 0;
static unsigned long new_default_gateway = 0;

static error_type_t handle_setting_auto_TCP_IP_config(network_type_t type);
static error_type_t handle_setting_manual_TCP_IP_config(network_type_t type);

typedef struct
{
    uint8_t             local_idx; // index into local ifaces array    
    bool                ipv4_configured;
#ifdef HAVE_IPV6
    bool                ipv6_configured;
#endif // HAVE_IPV6      
} ui_sys_network_context_t;

ui_sys_network_context_t *g_ui_sys_net_ctxt[NET_MAX_INTERFACES];


static unsigned int net_interface_index_get(network_type_t type)
{
	if(type == e_NET_ETHERNET && type == e_NET_STATION)
	{
		return	if_nametoindex(NET_IFACE_INTERFACE_NAME);
	}
	else
	{
		return	if_nametoindex("uAP_drvr");
	}
}

/*RestoreDefaults
  */
bool ui_sys_network_restore_defaults(void)
{
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In Restore Defaults\n") );

#ifdef HAVE_ETHERNET
	// restore ethernet network
	ui_sys_wired_network_restore_default();
#endif

#ifdef HAVE_WIELESS
	// restore wireless network
	ui_sys_wireless_restore_default();
#endif

#ifdef HAVE_SLP
	net_slp_var_restore_defaults( NET_SLP_VAR_ALL );
#endif

#ifdef HAVE_WSD
	net_wsd_var_restore_defaults( WSD_VAR_ALL );
#endif

#ifdef HAVE_MDNS

#ifdef HAVE_ETHERNET
	net_mdns_var_restore_defaults(net_interface_index_get(e_NET_ETHERNET), NET_MDNS_VAR_ALL);	
#endif
#ifdef HAVE_WIELESS
	bool isStationMode = false;
	ui_sys_wireless_station_enabled_get(&isStationMode);
	if(isStationMode)
	{
		net_mdns_var_restore_defaults(net_interface_index_get(e_NET_STATION), NET_MDNS_VAR_ALL);
	}
	
	bool isuAPMode = false;		
	ui_sys_wireless_uAP_enabled_get(&isuAPMode);
	if(isuAPMode)
	{
		net_mdns_var_restore_defaults(net_interface_index_get(e_NET_MICROAP), NET_MDNS_VAR_ALL);	
	}
#endif

#endif		// HAVE_MDNS

	return 	true;
}

/*GetNetworkStatus
  */
bool ui_sys_network_network_status_get(network_type_t type, net_iface_status_t * status)
{
	ASSERT(status != status);
	net_iface_get_var_network_status(net_interface_index_get(type), status);
	return true;
}

/*GetActiveLink
  */
bool ui_sys_network_active_link_get(network_type_t type, unsigned int * link_index)
{
	ASSERT(link_index != NULL);
    net_iface_get_var_active_link(net_interface_index_get(type), link_index);

	return	true;	
}

/*GetHostName
  */
bool ui_sys_network_hostname_get(network_type_t type, char * hostname, const size_t size)
{
	ASSERT(hostname != NULL);
	if(size < NET_IFACE_HOSTNAME_SIZE)
	{
		return	false;
	}

    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In Get Hostname\n") );

	net_iface_get_var_hostname(net_interface_index_get(type), hostname, NET_IFACE_HOSTNAME_SIZE);
	    
	return	true;
}

/*GetHostNameOrigin
  */    
bool ui_sys_network_hostname_origin_get(network_type_t type, net_iface_config_origin_t * origin)
{
	ASSERT(origin != NULL);

    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In Get device hostname origin\n") );
    
    net_iface_get_var_hostname_origin(net_interface_index_get(type), origin);

	return	true;
}

/*GetDomainName
  */    
bool ui_sys_network_domain_name_get(network_type_t type, char * domain, const size_t size)
{
	ASSERT(domain != NULL);
	if(size < NET_IFACE_DOMAINNAME_SIZE)
	{
		return	false;
	}

    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In Get Domain Name\n") );

    net_iface_get_var_domain(net_interface_index_get(type), domain, size);

	return	true;
}

/*GetDomainNameOrigin
  */    
bool ui_sys_network_domain_name_origin_get(network_type_t type, net_iface_config_origin_t * origin)
{
	ASSERT(origin != NULL);

    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In Get Domain Name Origin\n") );

    net_iface_get_var_domain_origin(net_interface_index_get(type), origin);

	return	true;
}

/*Get PreferredHostNameOrigin 
  */ 
bool ui_sys_network_preferred_hostname_origin_get(network_type_t type, net_iface_config_origin_t * origin)
{
	ASSERT(origin != NULL);	

    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In Get PreferredHostNameOrigin\n") );

    net_iface_get_var_hostname_preferred_origin(net_interface_index_get(type), origin);

	return	true;
}

/*Set PreferredHostNameOrigin 
  */ 
bool ui_sys_network_preferred_hostname_origin_set(network_type_t type, net_iface_config_origin_t origin)
{
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In Set PreferredHostNameOrigin\n") );
    
    net_iface_set_var_hostname_preferred_origin(net_interface_index_get(type), &origin);

	return	true;
}

/*Get PreferredDomainNameOrigin 
  */ 
bool ui_sys_network_preferred_domainname_origin_get(network_type_t type, net_iface_config_origin_t * origin)
{
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In Get PreferredDomainNameOrigin\n") );

    net_iface_get_var_domain_preferred_origin(net_interface_index_get(type), origin);
	
	return	true;
}

/*Set PreferredDomainNameOrigin 
  */ 
bool ui_sys_network_preferred_domainname_origin_set(network_type_t type, net_iface_config_origin_t origin)
{
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In Set PreferredDomainNameOrigin\n") );
    
    net_iface_set_var_domain_preferred_origin(net_interface_index_get(type), &origin);
	
	return	true;
}

/*Get ManualDomainName
  */    
bool ui_sys_network_manual_domainname_get(network_type_t type, char * domainname, const size_t size)
{
	ASSERT(domainname != NULL);
	if(size < NET_IFACE_DOMAINNAME_SIZE)
	{
		return	false;
	}

    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In Get Manual Domain Name\n") );
    
    net_iface_get_var_manual_domain(net_interface_index_get(type), domainname, size);

    return	true;
}

/*Set ManualDomainName
  */    
bool ui_sys_network_manual_domainname_set(network_type_t type, char * domainname)
{    
	ASSERT(domainname != NULL);

    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In Set Manual Domain Name\n") );
    
    net_iface_set_var_manual_domain(net_interface_index_get(type), domainname);

    return	true;
}

/*Get ManualHostName
  */    
bool ui_sys_network_manual_hostname_get(network_type_t type, char * hostname, const size_t size)
{
	ASSERT(hostname != NULL);
	if(size < NET_IFACE_DOMAINNAME_SIZE)
	{
		return	false;
	}

    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In Get Manual Host Name\n") );

    net_iface_get_var_manual_hostname(net_interface_index_get(type), hostname, size);

    return	true;
}

/*Set ManualHostName
  */    
bool ui_sys_network_manual_hostname_set(network_type_t type, const char * hostname)
{
	ASSERT(hostname != NULL);

    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In Set Manual Host Name\n") );
    
    net_iface_set_var_manual_hostname(net_interface_index_get(type), hostname);

    return	true;
}

/*Get isIPv4enabled
  */    
bool ui_sys_network_ipv4_enabled_get(network_type_t type,bool * enabled)
{
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In Get IPv4 enabled\n") );
    
    net_iface_get_var_ipv4_enabled(net_interface_index_get(type), enabled);

    return true;
}

/*Set isIPv4enabled
  */    
bool ui_sys_network_ipv4_enabled_set(network_type_t type,const bool enabled)
{
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In Set IPv4 enabled\n") );

    net_iface_set_var_ipv4_enabled(net_interface_index_get(type), enabled);

    return	true;
}

/*Get IPv4ConfigOptions
  */    
bool ui_sys_network_ipv4_config_options_get(network_type_t type, net_iface_ipv4_config_options_t * options)
{
	ASSERT(options != NULL);

    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In Get IPv4 config options\n") );

    net_iface_get_var_ipv4_config_options(net_interface_index_get(type), options);

    return	true;
}

/*Set IPv4ConfigOptions
  */    
bool ui_sys_network_ipv4_config_options_set(network_type_t type, const net_iface_ipv4_config_options_t options)
{
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In Set IPv4 config options\n") );

    net_iface_set_var_ipv4_config_options(net_interface_index_get(type), &options);

    return	true;
}

/*GetIPv4ConfigStatus
  */    
bool ui_sys_network_ipv4_config_status_get(network_type_t type, bool * configured)
{
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In Get IPv4 Configuration status\n") );

    net_iface_get_var_ipv4_configured(net_interface_index_get(type), configured);

    return	true;
}

/*GetIPv4LastConfig
  */    
bool ui_sys_network_ipv4_last_config_get(network_type_t type, net_iface_ipv4_config_method_t * method)
{
	ASSERT(method != NULL);

    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In Get IPv4 Last successful configuration\n") );

    net_iface_get_var_ipv4_last_config_method(net_interface_index_get(type), method);

	return	true;
}

/*GetIPv4BootServer
  */    
bool ui_sys_network_ipv4_boot_server_get(network_type_t type, char * server_address, const size_t size)
{
	ASSERT(server_address != NULL);
	if(size < 32)
	{
		return	false;
	}

    uint32_t bin_ipv4_addr;

    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In Get IPv4 address of Boot Server\n") );

    net_iface_get_var_ipv4_boot_server(net_interface_index_get(type), &bin_ipv4_addr);
    tfInetToAscii(bin_ipv4_addr, server_address);
	
	return	true;    
}

/*GetDhcpDomainName
  */    
bool ui_sys_network_dhcp_domainname_get(network_type_t type, char * domainname, const size_t size)
{
	ASSERT(domainname != NULL);
	if(size < NET_IFACE_DOMAINNAME_SIZE)
	{
		return	false;
	}	

    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In Get DHCP Domain Name\n") );
    
    net_iface_get_var_dhcpv4_domain(net_interface_index_get(type), domainname, size);
    
    return	true;
}

/*GetDhcpHostName
  */    
bool ui_sys_network_dhcp_hostname_get(network_type_t type, char * hostname, const size_t size)
{
	ASSERT(hostname != NULL);
	if(size < NET_IFACE_HOSTNAME_SIZE)
	{
		return	false;
	}
	
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In Get DHCP Host Name\n") );

    net_iface_get_var_dhcpv4_hostname(net_interface_index_get(type), hostname, size);
    
	return	true;
}

/*GetIPv4DNSServerOrigin
  */ 
bool ui_sys_network_ipv4_dns_server_origin_get(network_type_t type, net_iface_config_origin_t * origin)
{
	ASSERT(origin != NULL);

    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In GetIPv4DNSServerOrigin\n") );

    net_iface_get_var_dns_server_origin(net_interface_index_get(type), origin);

	return	true;
}

/*GetIPv4DNSServerAddress
  */ 
bool ui_sys_network_ipv4_dns_server_address_get(network_type_t type, uint8_t dns_index, char * server_address, const size_t size)
{
	ASSERT(server_address != NULL);
	if(size < 32)
	{
		return	false;
	}
	
    uint32_t bin_ipv4_addr;

    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In GetIPv4DNSServerAddress\n") );

    net_iface_get_var_dns_server_addr(net_interface_index_get(type), dns_index, &bin_ipv4_addr);
    tfInetToAscii(bin_ipv4_addr, server_address);
    
    return	true;
}

/*GetIPv4DHCPDNSAddress
  */ 
bool ui_sys_network_ipv4_dhcp_dns_address_get(network_type_t type, uint8_t dns_index, char * server_address, const size_t size)
{
	ASSERT(server_address != NULL);
	if(size < 32)
	{
		return	false;
	}

    uint32_t bin_ipv4_addr;

    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In GetIPv4DHCPDNSAddress\n") );

    net_iface_get_var_dhcpv4_dns_addr(net_interface_index_get(type), dns_index, &bin_ipv4_addr);
    tfInetToAscii(bin_ipv4_addr, server_address);
    
    return	true;
}

/*Get ManualDNSAddress
  */    
bool ui_sys_network_manual_dns_address_get(network_type_t type, uint8_t dns_index, char * server_address, const size_t size)
{
	ASSERT(server_address != NULL);
	if(size < 32)
	{
		return	false;
	}

    uint32_t bin_ipv4_addr;

    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In Get manual IPv4 DNS address\n") );
    
    net_iface_get_var_manual_dns_addr(net_interface_index_get(type), dns_index, &bin_ipv4_addr);
    tfInetToAscii(bin_ipv4_addr, server_address);

    return	true;
}

/*Set ManualDNSAddress
  */    
bool ui_sys_network_manual_dns_address_set(network_type_t type, uint8_t dns_index, char * server_address)
{
	ASSERT(server_address != NULL);

    uint32_t bin_ipv4_addr;
    
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In Set manual IPv4 DNS address\n") );

    bin_ipv4_addr = inet_aton(server_address);
    net_iface_set_var_manual_dns_addr(net_interface_index_get(type), dns_index, &bin_ipv4_addr);

    return	true;
}

/*GetDHCPLeaseTime
  */    
bool ui_sys_network_dhcp_lease_time_get(network_type_t type, uint32_t * lease_remaining)
{
	ASSERT(lease_remaining != NULL);    
    
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In Get DHCP Lease Time\n") );
  
    net_iface_get_var_dhcpv4_lease_remaining(net_interface_index_get(type), lease_remaining);

    return	true;
}

/*GetIPv4Address
  */    
bool ui_sys_network_ipv4_address_get(network_type_t type, char * ipv4_address, const size_t size) 
{
	ASSERT(ipv4_address != NULL);
	if(size < 32)
	{
		return	false;
	}

    uint32_t bin_ipv4_addr;
    
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In Get IPv4 address\n") );

    net_iface_get_var_ipv4_addr(net_interface_index_get(type), &bin_ipv4_addr);
    tfInetToAscii(bin_ipv4_addr, ipv4_address);

    return	true;
}

/*GetIPv4SubnetMask
  */    
bool ui_sys_network_ipv4_subnet_mask_get(network_type_t type, char * mask, const size_t size)
{
    uint32_t bin_ipv4_addr;
    
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In Get IPv4 subnet mask\n") );

    net_iface_get_var_ipv4_subnet_mask(net_interface_index_get(type), &bin_ipv4_addr);
    tfInetToAscii(bin_ipv4_addr, mask);

    return	true;
}

/*GetIPv4Gateway
  */    
bool ui_sys_network_ipv4_gateway_get(network_type_t type, char *gateway_addr, const size_t size)
{
	ASSERT(gateway_addr != NULL);
	
	if(size < 32)
	{
		return	false;
	}

    uint32_t bin_ipv4_addr;
    
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In Get IPv4 gateway\n") );

    net_iface_get_var_ipv4_gateway(net_interface_index_get(type), &bin_ipv4_addr);
    tfInetToAscii(bin_ipv4_addr, gateway_addr);

    return	true;
}

/*GetIPv4WinsAddress
  */    
bool ui_sys_network_ipv4_wins_address_get(network_type_t type, char * wins_addr, const size_t size)
{
	ASSERT(wins_addr != NULL);
	
	if(size < 32)
	{
		return	false;
	}
	
    uint32_t bin_ipv4_addr;
    
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In Get IPv4 NBNS address\n") );

    net_iface_get_var_ipv4_wins_addr(net_interface_index_get(type), &bin_ipv4_addr);
    tfInetToAscii(bin_ipv4_addr, wins_addr);

    return true;
}

/*Get IPv4ManualAddress
  */    
bool ui_sys_network_ipv4_manual_address_get(network_type_t type, char * address, const size_t size)
{
	ASSERT(address != NULL);
	if(size < 32)
	{
		return	false;
	}

    uint32_t bin_ipv4_addr;
    
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In Get manual IPv4 address\n") );

    net_iface_get_var_ipv4_manual_addr(net_interface_index_get(type), &bin_ipv4_addr);
    tfInetToAscii(bin_ipv4_addr, address);

    return	true;
}

/*Set IPv4ManualAddress
  */    
bool ui_sys_network_ipv4_manual_address_set(network_type_t type, char * address)
{
	ASSERT(address != NULL);

    uint32_t bin_ipv4_addr;
        
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In Set manual IPv4 address\n") );

    bin_ipv4_addr = inet_aton(address);
    net_iface_set_var_ipv4_manual_addr(net_interface_index_get(type), &bin_ipv4_addr);
    
    return	true;
}

/*Get IPv4ManualSubnetMask
  */    
bool ui_sys_network_ipv4_manual_subnet_mask_get(network_type_t type, char * mask, const size_t size)
{
	ASSERT(mask != NULL);
	if(size < 32)
	{
		return	false;	
	}	

    uint32_t bin_ipv4_addr;
    
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In Get manual IPv4 subnet mask\n") );

    net_iface_get_var_ipv4_manual_subnet_mask(net_interface_index_get(type), &bin_ipv4_addr);
    tfInetToAscii(bin_ipv4_addr, mask);

    return	true;
}

/*Set IPv4ManualSubnetMask
  */    
bool ui_sys_network_ipv4_manual_subnet_mask_set(network_type_t type, char * mask)
{
	ASSERT(mask != NULL);

    uint32_t bin_ipv4_addr;
        
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In Set manual IPv4 subnet mask\n") );

    bin_ipv4_addr = inet_aton(mask);
    net_iface_set_var_ipv4_manual_subnet_mask(net_interface_index_get(type), &bin_ipv4_addr);
    
    return	true;
}

/*Get IPv4ManualGateway
  */    
bool ui_sys_network_ipv4_manual_gateway_get(network_type_t type, char * gateway, const size_t size)
{
	ASSERT(gateway != NULL);
	if(size < 32)
	{
		return	false;
	}	
    uint32_t bin_ipv4_addr;
    
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In Get manual IPv4 gateway\n") );

    net_iface_get_var_ipv4_manual_gateway(net_interface_index_get(type), &bin_ipv4_addr);
    tfInetToAscii(bin_ipv4_addr, gateway);

    return	true;
}

/*Set IPv4ManualGateway
  */    
bool ui_sys_network_ipv4_manual_gateway_set(network_type_t type, char * gateway)
{
	ASSERT(gateway != NULL);

    uint32_t bin_ipv4_addr;
        
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In Set manual IPv4 gateway\n") );

    bin_ipv4_addr = inet_aton(gateway);
    net_iface_set_var_ipv4_manual_gateway(net_interface_index_get(type), &bin_ipv4_addr);
    
    return	true;
}

/*Get ManualWinsAddress
  */    
bool ui_sys_network_manual_wins_address_get(network_type_t type, char * wins, const size_t size)
{
	ASSERT(wins != NULL);
	if(size < 32)
	{
		return	false;
	}

    uint32_t bin_ipv4_addr;
    
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In Get manual IPv4 manual NBNS address\n") );

    net_iface_get_var_ipv4_manual_wins_addr(net_interface_index_get(type), &bin_ipv4_addr);
    tfInetToAscii(bin_ipv4_addr, wins);

    return 	true;
}

/*Set ManualWinsAddress
  */    
bool ui_sys_network_manual_wins_address_set(network_type_t type, char * wins)
{
	ASSERT(wins != NULL);

    uint32_t bin_ipv4_addr;
        
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In Set manual IPv4 NBNS address\n") );

    bin_ipv4_addr = inet_aton(wins);
    net_iface_set_var_ipv4_manual_wins_addr(net_interface_index_get(type), &bin_ipv4_addr);
    
    return	true;   
}

/*Get isIPv6enabled
  */    
bool ui_sys_network_ipv6_enabled_get(network_type_t type, bool * enabled)
{
	ASSERT(enabled != NULL);

    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In Get IPv6 enabled\n") );

    net_iface_get_var_ipv6_enabled(net_interface_index_get(type), enabled);

    return	true;
}

/*Set isIPv6enabled
  */    
bool ui_sys_network_ipv6_enabled_set(network_type_t type, const bool enabled)
{
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In Set IPv6 enabled\n") );

    net_iface_set_var_ipv6_enabled(net_interface_index_get(type), enabled);

    return	true;
}

/*Get IPv6ConfigOptions
  */    
bool ui_sys_network_ipv6_config_options_get(network_type_t type, net_iface_ipv6_config_options_t * options)
{
	ASSERT(options != NULL);

    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In Get IPv6 config options\n") );

    net_iface_get_var_ipv6_config_options(net_interface_index_get(type), options);

    return	true;
}

/*Set IPv6ConfigOptions
  */    
bool ui_sys_network_ipv6_config_options_set(network_type_t type, net_iface_ipv6_config_options_t options)
{
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In Set IPv6 config options\n") );

    net_iface_set_var_ipv6_config_options(net_interface_index_get(type), &options);

    return	true;
}

/*GetIPv6LocalConfigured
  */    
bool ui_sys_network_ipv6_local_configured_get(network_type_t type, bool * configured)
{
	ASSERT(configured != NULL);

    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In Get IPv6 Link-local configured\n") );

    net_iface_get_var_ipv6_local_configured(net_interface_index_get(type), configured);
    
    return	true;
}

/*GetIPv6OverallConfigured
  */    
bool ui_sys_network_ipv6_overall_configured_get(network_type_t type, bool * configured)
{
	ASSERT(configured != NULL);

    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In Get IPv6 configured\n") );

    net_iface_get_var_ipv6_configured(net_interface_index_get(type), configured);
    
    return	true;
}

/*GetIPv6DHCPDomainName
  */    
bool ui_sys_network_ipv6_dhcp_domainname_get(network_type_t type, char * domain, const size_t size)
{
	ASSERT(domain != NULL);
	if(size < NET_IFACE_DOMAINNAME_SIZE)
	{
		return	false;
	}
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In Get DHCPv6 Domain Name\n") );

    net_iface_get_var_dhcpv6_domain(net_interface_index_get(type), domain, size);

    return	true;
}

/*GetIPv6DHCPHostName
  */
bool ui_sys_network_ipv6_dhcp_hostname_get(network_type_t type, char * hostname, const size_t size)
{
	ASSERT(hostname != NULL);
	if(size < NET_IFACE_HOSTNAME_SIZE)
	{
		return	false;
	}    
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In Get DHCPv6 Host Name\n") );

    net_iface_get_var_dhcpv6_hostname(net_interface_index_get(type), hostname, size);
    
    return	true;
}

/*GetIPv6LinkLocalAddress
  */
bool ui_sys_network_ipv6_link_local_address_get(network_type_t type, char * ipv6_addr, const size_t size)
{
	ASSERT(ipv6_addr != NULL);
	if(size < INET6_ADDRSTRLEN)
	{
		return	false;
	}
    
    net_iface_ipv6_addr_t bin_ipv6_addr;
    net_iface_ipv6_addr_hndl_t addr_hndl;

    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In Get IPv6 Link Local address\n") );
    
    net_iface_get_var_ipv6_link_local_addr(net_interface_index_get(type), &bin_ipv6_addr, &addr_hndl);
    if(NET_IFACE_IPV6_ADDR_UNSPECIFIED(bin_ipv6_addr))
    {
        DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: IPv6 is down\n") );		        
        return	false; 
    }

    inet_ntop(AF_INET6, (void *)(&bin_ipv6_addr.addr), ipv6_addr, size);    

    return	true;
}

/*GetIPv6PreferredAddress
  */
bool ui_sys_network_ipv6_preferred_address_get(network_type_t type, char * ipv6_addr, const size_t size)
{
	ASSERT(ipv6_addr != NULL);
	if(size < INET6_ADDRSTRLEN)
	{
		return	false;
	}

    net_iface_ipv6_addr_t bin_ipv6_addr;
    net_iface_ipv6_addr_hndl_t addr_hndl;

    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In Get IPv6 Preferred address\n") );
    
    net_iface_get_var_ipv6_preferred_addr(net_interface_index_get(type), &bin_ipv6_addr, &addr_hndl);
    if(NET_IFACE_IPV6_ADDR_UNSPECIFIED(bin_ipv6_addr))
    {
        DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: IPv6 is down\n") );
        return	false;
    }

    inet_ntop(AF_INET6, (void *)(&bin_ipv6_addr.addr), ipv6_addr, size);    

    return	true;
}

/*GetIPv6NextPreferredAddress
  */
bool ui_sys_network_ipv6_next_preferred_address_get(network_type_t type, char * ipv6_addr, const size_t size)
{
	ASSERT(ipv6_addr != NULL);
	if(size < INET6_ADDRSTRLEN)
	{
		return	false;
	}
    
    net_iface_ipv6_addr_t bin_ipv6_addr;
    net_iface_ipv6_addr_hndl_t addr_hndl, prev_addr_hndl;

    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In Get IPv6 Next Preferred address\n") );

    net_iface_get_var_ipv6_preferred_addr(net_interface_index_get(type), &bin_ipv6_addr, &addr_hndl);
    prev_addr_hndl = addr_hndl;
    net_iface_get_var_ipv6_next_addr(net_interface_index_get(type), &bin_ipv6_addr, &addr_hndl, prev_addr_hndl);
    if(NET_IFACE_IPV6_ADDR_UNSPECIFIED(bin_ipv6_addr))
    {
        DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: IPv6 is down\n") );
        return	false;
    }

    inet_ntop(AF_INET6, (void *)(&bin_ipv6_addr.addr), ipv6_addr, size);    

    return	true;
}

/*GetIPv6AddrInfo
  */
bool ui_sys_network_ipv6_addr_info_get(network_type_t type, net_iface_ipv6_addr_info_t * addr_info)
{
	ASSERT(addr_info != NULL);    

    net_iface_ipv6_addr_hndl_t 	addr_hndl;
	net_iface_ipv6_addr_info_t	temp;
	net_iface_ipv6_addr_t bin_ipv6_addr;

    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In Get IPv6 Address Info\n") );

    net_iface_get_var_ipv6_preferred_addr(net_interface_index_get(type), &bin_ipv6_addr, &addr_hndl);   
    net_iface_get_var_ipv6_addr_info(net_interface_index_get(type), &temp, addr_hndl);
	
	*addr_info = temp;

	return	true;        
}

/*Get IPv6ManualAddress
  */    
bool ui_sys_network_ipv6_manual_address_get(network_type_t type, char * addr_str, const size_t size)
{
	ASSERT(addr_str != NULL);
	if(size < INET6_ADDRSTRLEN)
	{
		return	false;
	}
    
    net_iface_ipv6_addr_t bin_ipv6_addr;
    
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In Get manual IPv6 address\n") );

    net_iface_get_var_ipv6_manual_addr(net_interface_index_get(type), &bin_ipv6_addr);
    if(NET_IFACE_IPV6_ADDR_UNSPECIFIED(bin_ipv6_addr))
    {
        DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: IPv6 is down\n") );
        return	false;
    }

    inet_ntop(AF_INET6, (void *)(&bin_ipv6_addr.addr), addr_str, size);

    return	true;
}

/*Set IPv6ManualAddress
  */    
bool ui_sys_network_ipv6_manual_address_set(network_type_t type, char * addr_str)
{
	ASSERT(addr_str != NULL);
    
    net_iface_ipv6_addr_t bin_ipv6_addr;
    
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Printer.Network: In Set manual IPv6 address\n") );

    inet_pton(AF_INET6, (void *)(&bin_ipv6_addr.addr), addr_str);
    net_iface_set_var_ipv6_manual_addr(net_interface_index_get(type), &bin_ipv6_addr);
    
    return	true;
}


bool ui_sys_network_hostname_to_ipv4(const char * hostname, uint32_t *ipv4_addr)
{
	ASSERT(hostname != NULL && ipv4_addr != NULL);
	error_type_t	status;
	
	status = net_dns_hostname_to_ipv4(hostname, ipv4_addr);
	if(status == OK)
	{
		return	true;
	}
	else
	{
		return	false;
	}								
}

error_type_t handle_setting_auto_TCP_IP_config(network_type_t type)
{
    net_iface_ipv4_config_options_t net_flags;
    error_type_t                    status;

    unsigned int if_index = net_interface_index_get(type);

    status = net_iface_get_var_ipv4_config_options(if_index, &net_flags);
    ASSERT(status == OK);

    if (net_flags & NET_IFACE_IPV4_USE_MANUAL)
    {
        net_flags &= ~NET_IFACE_IPV4_USE_MANUAL;
        status = net_iface_set_var_ipv4_config_options(if_index, &net_flags);
        ASSERT(status == OK);
    }

    return(status);
}

error_type_t handle_setting_manual_TCP_IP_config(network_type_t type)
{
    net_iface_ipv4_config_options_t net_flags;
    error_type_t                    status;

    unsigned int if_index = net_interface_index_get(type);

    status = net_iface_get_var_ipv4_config_options(if_index, &net_flags);
    ASSERT(status == OK);

    status = net_iface_set_var_ipv4_manual_addr(if_index, &new_IP_address);
    ASSERT(status == OK);

    status = net_iface_set_var_ipv4_manual_subnet_mask(if_index, &new_subnet_mask);
    ASSERT(status == OK);
    status = net_iface_set_var_ipv4_manual_gateway(if_index, &new_default_gateway);
    ASSERT(status == OK);

    net_flags |= NET_IFACE_IPV4_USE_MANUAL;
    status = net_iface_set_var_ipv4_config_options(if_index, &net_flags);
    ASSERT(status == OK);

    new_IP_address = 0;
    new_subnet_mask = 0;
    new_default_gateway = 0;

    return(status);
}

bool ui_sys_network_set_tcpip_config(network_type_t type, tcpip_config_option_t option)
{
	error_type_t	status;	

	if(option == e_IPv4_CONFIG_MANUAL)
	{
		status = handle_setting_manual_TCP_IP_config(type);	
	}
	else
	{
		status = handle_setting_auto_TCP_IP_config(type);
	}
	if(status == OK)
	{
		return	true;
	}
	else
	{
		return	false;
	}	
}


typedef struct
{
    uint8_t       if_enum;
    unsigned int  if_index;
    char         *name;
    bool          initial_state_sent;

    bool          configured_v4;
    uint32_t      status;
    unsigned int  active_link_index;
    char          active_link_name[NET_LINK_NAME_SIZE + 1];
} iface_info_t;

static iface_info_t *net_ifaces_info = NULL;


static void net_iface_event_callback(net_iface_event_flags_t net_events, void *context)
{
    iface_info_t *iface_info = (iface_info_t *)context;
    const char *ui_event_type = NULL;
    data_block_handle_t data_block = NULL;

    if (net_events & NET_IFACE_EVENT_LINK_STATUS_CHANGE)
    {
        net_iface_status_t link_status;
        unsigned int new_active_link;

        net_iface_get_var_active_link(iface_info->if_index, &new_active_link);

        net_iface_get_var_network_status(iface_info->if_index, &link_status);

        if ((!iface_info->initial_state_sent) || 
            ((link_status & NET_IFACE_STATUS_LINK_UP) != (iface_info->status & NET_IFACE_STATUS_LINK_UP)) ||
            (new_active_link != iface_info->active_link_index))
        {
            if ((!(link_status & NET_IFACE_STATUS_LINK_UP)) ||
                 (
                   (iface_info->active_link_index != 0) && 
                   (new_active_link != iface_info->active_link_index)
                 )
               )
            {
                // We either lost link status or switched to a different link - let them know
                // the old one is now down
                ui_event_type = NETWORK_EVENT_LINK;
                data_block = ui_sys_create_data_block();
                ui_sys_data_block_add_string(data_block, iface_info->name);

                ui_sys_data_block_add_string(data_block, NETWORK_DETAILS_DOWN);
                if (strlen(iface_info->active_link_name) > 0)
                {
                    ui_sys_data_block_add_string(data_block, iface_info->active_link_name);
                }

                ui_sys_notify_ui_of_event(NETWORK_MODULE_NAME, ui_event_type, data_block);

                iface_info->active_link_index = 0;
                iface_info->active_link_name[0] = '\0';
            }

            if (link_status & NET_IFACE_STATUS_LINK_UP)
            {
                ui_event_type = NETWORK_EVENT_LINK;
                data_block = ui_sys_create_data_block();
                ui_sys_data_block_add_string(data_block, iface_info->name);

                ui_sys_data_block_add_string(data_block, NETWORK_DETAILS_UP);
                net_link_get_name(new_active_link, iface_info->active_link_name, sizeof(iface_info->active_link_name) - 1);
                ui_sys_data_block_add_string(data_block, iface_info->active_link_name);

                ui_sys_notify_ui_of_event(NETWORK_MODULE_NAME, ui_event_type, data_block);
            }
        }

        iface_info->status = link_status;
        iface_info->active_link_index = new_active_link;
    }

    if (net_events & NET_IFACE_EVENT_IPV4_CONFIG_CHANGE)
    {
        bool configured;

        net_iface_get_var_ipv4_configured(iface_info->if_index, &configured);

        if ((!iface_info->initial_state_sent) || (configured != iface_info->configured_v4))
        {
            ui_event_type = NETWORK_EVENT_CONFIGV4;
            data_block = ui_sys_create_data_block();
            ui_sys_data_block_add_string(data_block, iface_info->name);

            if (configured)
            {
                ui_sys_data_block_add_string(data_block, NETWORK_DETAILS_CONFIGURED);
            }
            else
            {
                ui_sys_data_block_add_string(data_block, NETWORK_DETAILS_NOT_CONFIGURED);
            }

            ui_sys_notify_ui_of_event(NETWORK_MODULE_NAME, ui_event_type, data_block);
        }

        iface_info->configured_v4 = configured;
    }

    iface_info->initial_state_sent = true;
}

static void configure_network_ui_event_notifier()
{
    int if_enum;
    int num_ifaces = net_iface_enum_get_count();
    net_ifaces_info = (iface_info_t *)MEM_MALLOC( num_ifaces * sizeof(iface_info_t) );	
    XASSERT(net_ifaces_info != NULL, num_ifaces);

    for (if_enum = 0; if_enum < num_ifaces; if_enum++ )
    {
        unsigned int if_index = net_iface_enum_to_if_index(if_enum);

        memset(net_ifaces_info + if_enum, 0, sizeof(iface_info_t));
        net_ifaces_info[if_enum].name = (char *)MEM_MALLOC( NET_LINK_NAME_SIZE );		
        XASSERT(net_ifaces_info[if_enum].name != NULL, NET_LINK_NAME_SIZE);
        net_ifaces_info[if_enum].name[0] = '\0';

        if (if_index > 0)
        {
            net_ifaces_info[if_enum].if_enum  = if_enum;
            net_ifaces_info[if_enum].if_index = if_index;
            if_indextoname(if_index, net_ifaces_info[if_enum].name);

            net_iface_register_events(if_index, 0xffffffff, net_iface_event_callback, (void*)(&net_ifaces_info[if_enum]));

            // Prime the pump - Send UI events indicating the initial state of the system
            net_iface_event_callback(0xffffffff, (void*)(&net_ifaces_info[if_enum]));
        }
    }
}


/*Network init
  */
void ui_sys_network_init(void)
{
	configure_network_ui_event_notifier();
}    
