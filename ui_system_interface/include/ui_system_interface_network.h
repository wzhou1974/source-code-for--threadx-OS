/*
 * ============================================================================
 * Copyright (c) 2011-2013   Marvell International, Ltd. All Rights Reserved
 *
 *                         Marvell Confidential
 * ============================================================================
 */

/** 
 * \file ui_system_interface_network.h
 * 
 * \brief 
 * 
 * 
 **/

#ifndef __UI_SYSTEM_INTERFACE_NETWORK_H__
#define __UI_SYSTEM_INTERFACE_NETWORK_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "eth_mac_api.h"
#include "net_iface_vars.h"


#include "wps_events.h"
#include "delay_api.h"
#include "net_eth_vars.h"
#include "eth_mac_api.h"
#include "wifi_config.h"
#include "wifi_intf_api.h"

#include "net_iface_api.h"
#include "trmacro.h"
#include "trtype.h"
#include "trglobal.h"


#define NET_MODULE_NAME         "network"

#define NET_NETDRVR_IP_UP       "net_netdrvr_ip_up"
#define NET_UAPDRVR_IP_UP       "net_uapdrvr_ip_up"
#define NET_NETDRVR_IP_DOWN     "net_netdrvr_ip_down"
#define NET_UAPDRVR_IP_DOWN     "net_uapdrvr_ip_down"


typedef enum _network_type_s
{
    e_NET_ETHERNET,
    e_NET_STATION,
    e_NET_MICROAP   
} network_type_t;

typedef enum _tcpip_config_option_s
{
    e_IPv4_CONFIG_MANUAL,
    e_IPv4_CONFIG_AUTOMATIC
} tcpip_config_option_t;


/*RestoreDefaults
  */
bool ui_sys_network_restore_defaults(void);
/*GetNetworkStatus
  */
bool ui_sys_network_network_status_get(network_type_t type, net_iface_status_t * status);
/*GetActiveLink
  */
bool ui_sys_network_active_link_get(network_type_t type, unsigned int * link_index);
/*GetHostName
  */
bool ui_sys_network_hostname_get(network_type_t type, char * hostname, const size_t size);
/*GetHostNameOrigin
  */    
bool ui_sys_network_hostname_origin_get(network_type_t type, net_iface_config_origin_t * origin);
/*GetDomainName
  */    
bool ui_sys_network_domain_name_get(network_type_t type, char * domain, const size_t size);
/*GetDomainNameOrigin
  */    
bool ui_sys_network_domain_name_origin_get(network_type_t type, net_iface_config_origin_t * origin);
/*Set PreferredHostNameOrigin 
  */ 
bool ui_sys_network_preferred_hostname_origin_set(network_type_t type, net_iface_config_origin_t origin);
/*Get PreferredDomainNameOrigin 
  */ 
bool ui_sys_network_preferred_domainname_origin_get(network_type_t type, net_iface_config_origin_t * origin);
/*Set PreferredDomainNameOrigin 
  */ 
bool ui_sys_network_preferred_domainname_origin_set(network_type_t type, net_iface_config_origin_t origin);
/*Get ManualDomainName
  */    
bool ui_sys_network_manual_domainname_get(network_type_t type, char * domainname, const size_t size);
/*Set ManualDomainName
  */    
bool ui_sys_network_manual_domainname_set(network_type_t type, char * domainname);
/*Get ManualHostName
  */    
bool ui_sys_network_manual_hostname_get(network_type_t type, char * hostname, const size_t size);
/*Set ManualHostName
  */    
bool ui_sys_network_manual_hostname_set(network_type_t type, const char * hostname);
/*Get isIPv4enabled
  */    
bool ui_sys_network_ipv4_enabled_get(network_type_t type,bool * enabled);
/*Set isIPv4enabled
  */    
bool ui_sys_network_ipv4_enabled_set(network_type_t type,const bool enabled);
/*Get IPv4ConfigOptions
  */    
bool ui_sys_network_ipv4_config_options_get(network_type_t type, net_iface_ipv4_config_options_t * options);
/*Set IPv4ConfigOptions
  */    
bool ui_sys_network_ipv4_config_options_set(network_type_t type, const net_iface_ipv4_config_options_t options);
/*GetIPv4ConfigStatus
  */    
bool ui_sys_network_ipv4_config_status_get(network_type_t type, bool * configured);
/*GetIPv4LastConfig
  */    
bool ui_sys_network_ipv4_last_config_get(network_type_t type, net_iface_ipv4_config_method_t * method);
/*GetIPv4BootServer
  */    
bool ui_sys_network_ipv4_boot_server_get(network_type_t type, char * server_address, const size_t size);
/*GetDhcpDomainName
  */    
bool ui_sys_network_dhcp_domainname_get(network_type_t type, char * domainname, const size_t size);
/*GetDhcpHostName
  */    
bool ui_sys_network_dhcp_hostname_get(network_type_t type, char * hostname, const size_t size);
/*GetIPv4DNSServerOrigin
  */ 
bool ui_sys_network_ipv4_dns_server_origin_get(network_type_t type, net_iface_config_origin_t * origin);
/*GetIPv4DNSServerAddress
  */ 
bool ui_sys_network_ipv4_dns_server_address_get(network_type_t type, uint8_t dns_index, char * server_address, const size_t size);
/*GetIPv4DHCPDNSAddress
  */ 
bool ui_sys_network_ipv4_dhcp_dns_address_get(network_type_t type, uint8_t dns_index, char * server_address, const size_t size);
/*Set ManualDNSAddress
  */    
bool ui_sys_network_manual_dns_address_set(network_type_t type, uint8_t dns_index, char * server_address);
/*GetDHCPLeaseTime
  */    
bool ui_sys_network_dhcp_lease_time_get(network_type_t type, uint32_t * lease_remaining);
/*GetIPv4Address
  */    
bool ui_sys_network_ipv4_address_get(network_type_t type, char * ipv4_address, const size_t size);
/*GetIPv4SubnetMask
  */    
bool ui_sys_network_ipv4_subnet_mask_get(network_type_t type, char * mask, const size_t size);
/*GetIPv4Gateway
  */    
bool ui_sys_network_ipv4_gateway_get(network_type_t type, char *gateway_addr, const size_t size);
/*GetIPv4WinsAddress
  */    
bool ui_sys_network_ipv4_wins_address_get(network_type_t type, char * wins_addr, const size_t size);
/*Get IPv4ManualAddress
  */    
bool ui_sys_network_ipv4_manual_address_get(network_type_t type, char * address, const size_t size);
/*Set IPv4ManualAddress
  */    
bool ui_sys_network_ipv4_manual_address_set(network_type_t type, char * address);
/*Get IPv4ManualSubnetMask
  */    
bool ui_sys_network_ipv4_manual_subnet_mask_get(network_type_t type, char * mask, const size_t size);
/*Set IPv4ManualSubnetMask
  */    
bool ui_sys_network_ipv4_manual_subnet_mask_set(network_type_t type, char * mask);
/*Get IPv4ManualGateway
  */    
bool ui_sys_network_ipv4_manual_gateway_get(network_type_t type, char * gateway, const size_t size);
/*Set IPv4ManualGateway
  */    
bool ui_sys_network_ipv4_manual_gateway_set(network_type_t type, char * gateway);
/*Get ManualWinsAddress
  */    
bool ui_sys_network_manual_wins_address_get(network_type_t type, char * wins, const size_t size);
/*Set ManualWinsAddress
  */    
bool ui_sys_network_manual_wins_address_set(network_type_t type, char * wins);
/*Get isIPv6enabled
  */    
bool ui_sys_network_ipv6_enabled_get(network_type_t type, bool * enabled);
/*Set isIPv6enabled
  */    
bool ui_sys_network_ipv6_enabled_set(network_type_t type, const bool enabled);
/*Get IPv6ConfigOptions
  */    
bool ui_sys_network_ipv6_config_options_get(network_type_t type, net_iface_ipv6_config_options_t * options);
/*Set IPv6ConfigOptions
  */    
bool ui_sys_network_ipv6_config_options_set(network_type_t type, net_iface_ipv6_config_options_t options);
/*GetIPv6LocalConfigured
  */    
bool ui_sys_network_ipv6_local_configured_get(network_type_t type, bool * configured);
/*GetIPv6OverallConfigured
  */    
bool ui_sys_network_ipv6_overall_configured_get(network_type_t type, bool * configured);
/*GetIPv6DHCPDomainName
  */    
bool ui_sys_network_ipv6_dhcp_domainname_get(network_type_t type, char * domain, const size_t size);
/*GetIPv6DHCPHostName
  */
bool ui_sys_network_ipv6_dhcp_hostname_get(network_type_t type, char * hostname, const size_t size);
/*GetIPv6LinkLocalAddress
  */
bool ui_sys_network_ipv6_link_local_address_get(network_type_t type, char * ipv6_addr, const size_t size);
/*GetIPv6PreferredAddress
  */
bool ui_sys_network_ipv6_preferred_address_get(network_type_t type, char * ipv6_addr, const size_t size);
/*GetIPv6NextPreferredAddress
  */
bool ui_sys_network_ipv6_next_preferred_address_get(network_type_t type, char * ipv6_addr, const size_t size);
/*GetIPv6AddrInfo
  */
bool ui_sys_network_ipv6_addr_info_get(network_type_t type, net_iface_ipv6_addr_info_t * addr_info);
/*Get IPv6ManualAddress
  */    
bool ui_sys_network_ipv6_manual_address_get(network_type_t type, char * addr_str, const size_t size);
/*Set IPv6ManualAddress
  */    
bool ui_sys_network_ipv6_manual_address_set(network_type_t type, char * addr_str);

bool ui_sys_network_hostname_to_ipv4(const char * hostname, uint32_t *ipv4_addr);

bool ui_sys_network_set_tcpip_config(network_type_t type, tcpip_config_option_t option);

#ifdef HAVE_ETHERNET

#define NETWORK_MODULE_NAME              "network"

#define NETWORK_EVENT_LINK               "link"
#define NETWORK_EVENT_CONFIGV4           "config_v4"
#define NETWORK_EVENT_CONFIGV6           "config_v6"
#define NETWORK_DETAILS_UP               "up"
#define NETWORK_DETAILS_DOWN             "down"
#define NETWORK_DETAILS_CONFIGURED       "configured"
#define NETWORK_DETAILS_NOT_CONFIGURED   "not_configured"


void ui_sys_wired_network_get_link_status(bool * status);
void ui_sys_wired_network_get_link_speed(eth_mac_link_config_t * speed);
void ui_sys_wired_network_get_mac_address(uint8_t * phys_addr, size_t size);


bool ui_sys_wired_network_restore_default(void);

#endif  // HAVE_ETHERNET

#ifdef HAVE_WIRELESS

#define WPS_MODULE_NAME         "wps"

#define WPS_STATUS_IDLE         GENERAL_STATUS_IDLE
#define WPS_STATUS_SEARCHING    "searching"
#define WPS_STATUS_CONNECTING   "connecting"
#define WPS_STATUS_SUCCESS      "success"
#define WPS_STATUS_NO_ROUTERS   "no_routers"
#define WPS_STATUS_FAILED       "failed"
#define WPS_STATUS_CANCELLED    "cancelled"


#define WIFI_MODULE_NAME        "wireless"

#define WIFI_STATUS_READY       GENERAL_STATUS_READY
#define WIFI_STATUS_SELF_TEST   "testing"
#define WIFI_LINK_DOWN          "link_down"
#define WIFI_LINK_CONNECTING    "link_connecting"
#define WIFI_LINK_CONNECTED     "link_connected"
#define WIFI_LINK_ERROR         "link_error"
#define WIFI_STA_ENABLED        "sta_enabled"
#define WIFI_STA_DISABLED       "sta_disabled"
#define WIFI_SIG_STRENGTH       "Sig_Strength"


/*RestoreDefaults
  */
bool ui_sys_wireless_station_restore_defaults(void);
/*Get Station status
  returns: Wifi Station status as enabled or disabled
 */
bool ui_sys_wireless_station_enabled_get(bool * enabled);
/*Set Station status
 */
bool ui_sys_wireless_station_enabled_set(bool enabled);
/*Get Station SSID
  returns: Wifi Station SSID as a string
 */
bool ui_sys_wireless_station_ssid_get(char * ssid_data, const size_t size);
/*Set Station SSID
 */
bool ui_sys_wireless_station_ssid_set(char * ssid_data);
/*Get Station Security Mode
  returns: Wifi Station Security Mode (open, wep, wpa, wpa2)
 */
bool ui_sys_wireless_station_secuity_mode_get(wifi_intf_security_mode_t * sec_mode);
/*Set Station Security Mode (open, wep, wpa, wpa2)
  returns: true if success, false if failure   
 */
bool ui_sys_wireless_station_security_mode_set(wifi_intf_security_mode_t sec_mode);
/*Get Station Wep Authentication mode
  returns: Wifi Station Wep Authentication mode (open, shared, auto)
 */
bool ui_sys_wireless_station_wep_auth_mode_get(wifi_intf_wep_auth_mode_t * auth_mode);
/*Set Station WEP Authentication Mode (open, shared, auto)
  returns: true if success, false if failure   
 */
bool ui_sys_wireless_station_wep_authMode_set(wifi_intf_wep_auth_mode_t auth_mode);
/*Get Station WEP Key
  index: Numeric Value indicating the WEP key index to view
  returns: Wifi Station WEP key as a string (should correspond to 10 hex digits)    
 */
bool ui_sys_wireless_station_get_wep_key(const uint8_t index, char * key_data, const size_t size);
/*Set Station WEP Key
  index: Numeric Value indicating the WEP key index to view
  key  : Key as a string (should correspond to 10 hex digits)
*/
bool ui_sys_wireless_station_set_wep_key(const uint8_t index, const char * key);
/*Get Channel 
  returns: Channel as an integer
 */
bool ui_sys_wireless_station_get_channel(uint8_t * channel);
/*Get Station WEP Current Key : Current WEP key in use
  returns: Wifi Station WEP key index as an integer
 */
bool ui_sys_wireless_station_wep_current_key_get(uint8_t * cur_key);
/*Set Station WEP Current Key 
  index : index of the key (as an integer) to be used as the current key, should be in the range 0-3 
  returns : false if the key is not in the range 0-3, else true
 */
bool ui_sys_wireless_station_wep_current_key_set(uint8_t cur_key);
/*Get Station WPA passphrase : Current WPA/WPA2 passphrase in use
  returns: Wifi Station WPA/WPA2 passphrase as a string 
 */
bool ui_sys_wireless_station_wpa_passphrase_get(char * passphrase, const size_t size);
/*Set Station WPA/WPA2 passphrase 
 */
bool ui_sys_wireless_station_wpa_passphrase_set(const char *passphrase);
/*Get Station communication mode 
  returns: Wifi Station communication mode as a string (Adhoc or Infrastructure)
 */
bool ui_sys_wireless_station_comm_mode_get(wifi_intf_comm_mode_t * comm_mode);
/*Station Generate WPS PIN
returns: WPS PIN as a string

Noted: The caller has the responsibility to free the returned string by MEM_FREE_AND_NULL, otherwise memory leak.
 */
char * ui_sys_wireless_station_generate_wps_pin(void);
/*Station StartWPS
pin: provide WPS PIN as a string, ideally use GenerateWPSPin to generate a PIN first and 
     then provide that as an input to this function.
 */
bool ui_sys_wireless_station_start_wps(const char * start_type, const char * pin);

/*Station RefreshWPSTimer
 */
bool ui_sys_station_refresh_wps_timer(void);
/*Station StopWPS
 */
void ui_sys_wireless_station_stop_WPS(void);
/*Set Station communication mode 
  returns: false if there is an invalid communication mode, else true
 */
bool ui_sys_wireless_station_comm_mode_set(wifi_intf_comm_mode_t comm_mode);

/*Get Station link status 
  returns: link status
 */
bool ui_sys_wireless_station_get_link_status(uint32_t * wifi_link_status);
/*RestoreDefaults
  */
bool ui_sys_wireless_microap_restore_defaults(void);
/*Get uAP status
  returns: Wifi MicroAP status as enabled or disabled
 */
bool ui_sys_wireless_microap_enabled_get(bool * enabled);
/*Set uAP status
 */
bool ui_sys_wireless_microap_enabled_set(const bool enabled);
/*Get uAP SSID
  returns: Wifi uAP SSID as a string

 */
bool ui_sys_wireless_microap_ssid_get(char * ssid, const size_t size);
/*Set uAP SSID
 */
bool ui_sys_wireless_microap_ssid_set(const char * ssid);
/*Get uAP SSID Prefix
  returns: Wifi uAP SSID Prefix as a string
 */
bool ui_sys_wireless_microap_ssid_prefix_get(char * prefix, const size_t size);
/*Get uAP Security Mode
  returns: Wifi uAP Security Mode as a String (open, wep, wpa, wpa2)
 */
bool ui_sys_wireless_microap_security_mode_get(wifi_intf_security_mode_t * sec_mode);
/*Set uAP Security Mode (open, wep, wpa, wpa2)
  returns: true if success, false if failure   
 */
bool ui_sys_wireless_microap_security_mode_set(wifi_intf_security_mode_t sec_mode);
/*Get uAP Wep Authentication mode
  returns: Wifi uAP Wep Authentication mode as a String (open, shared, auto)
 */
bool ui_sys_wireless_microap_wep_auth_mode_get(wifi_intf_wep_auth_mode_t * auth_mode);
/*Set uAP WEP Authentication Mode (open, shared, auto)
  returns: true if success, false if failure   
 */
bool ui_sys_wireless_microap_wep_auth_mode_set(wifi_intf_wep_auth_mode_t auth_mode);
/*Get uAP WEP Key
  index: Numeric Value indicating the WEP key index to view
  returns: Wifi uAP WEP key as a string (should correspond to 10 hex digits)
 */
bool ui_sys_wireless_microap_get_wep_key(const uint8_t index, char * key_data, const size_t size);
/*Set uAP WEP Key
  index: Numeric Value indicating the WEP key index to view
  key  : Key as a string (should correspond to 10 hex digits)
*/
bool ui_sys_wireless_microap_set_wep_key(const uint8_t index, char * wep_key);
/*Get uAP WEP Current Key : Current WEP key in use
  returns: Wifi uAP WEP key index as an integer
 */
bool ui_sys_wireless_microap_wep_current_key_get(uint8_t * cur_key);
/*Set uAP WEP Current Key 
  index : index of the key (as an integer) to be used as the current key, should be in the range 0-3 
  returns : false if the key is not in the range 0-3, else true
 */
bool ui_sys_wireless_microap_wep_current_key_set(const uint8_t cur_key);
/*Get uAP WPA passphrase : Current WPA/WPA2 passphrase in use
  returns: Wifi uAP WPA/WPA2 passphrase as a string

    Noted: The caller has the responsility to free the returned string by MEM_FREE_AND_NULL 
 */
bool ui_sys_wireless_microap_wpa_passphrase_get(char * passphrase_data, const size_t size);
/*Set uAP WPA/WPA2 passphrase 
 */
bool ui_sys_wireless_microap_wpa_passphrase_set(const char * passphrase_data);
/*Get uAP Encryption Mode
  returns: Wifi uAP Encryption Mode as a String (tkip, aes, both)
 */
bool ui_sys_wireless_microap_encrypt_mode_get(wifi_intf_wpa_encryp_mode_t * enc_mode);
/*Set uAP Encryption Mode (tkip, aes, both)
  returns: true if success, false if failure   
 */
bool ui_sys_wireless_microap_encryp_mode_set(wifi_intf_wpa_encryp_mode_t enc_mode);
/*Generate a random passphrase for the specified length. 
 * Note that the min and max value for length
 * are specified by WIFI_UAP_MIN_PASSPHRASE_LENGTH and 
 * WIFI_UAP_MAX_PASSPHRASE_LENGTH respectively.
   returns: the randomly generated passphrase as a string   

    Noted: The caller has the responsility to free the returned string by MEM_FREE_AND_NULL 
 */
char * ui_sys_wireless_microap_generate_random_passphrase(void);
/*StartScan 
 */
bool ui_system_inteface_wireless_start_scan(const char *ssid);
/*isScanning 
 */
bool ui_sys_wireless_is_scanning(void);
/*GetScanResults 
 */
bool ui_sys_wireless_get_scan_results(wifi_intf_scan_info_t * scan_info);
/*ScanSleep: Used during the blocking WiFi scan 
*/
bool ui_sys_wireless_scan_sleep(void);
/*GetMacAddress: returns the MAC address as a string

    The buffer size of macOnBoard must >= MAC_ADDR_LENGTH    
*/
bool ui_sys_wireless_get_mac_address(uint8_t * macOnBoard);

bool ui_sys_wireless_restore_default(void);

#endif  //  HAVE_WIRELESS

#endif // ifndef __UI_SYSTEM_INTERFACE_NETWORK_H__

