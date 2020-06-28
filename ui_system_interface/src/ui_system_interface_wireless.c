/*
 * ============================================================================
 * Copyright (c) 2013   Marvell International, Ltd. All Rights Reserved
 *
 *                         Marvell Confidential
 * ============================================================================
 */

/** 
 * \file ui_system_interface_wireless.c
 * 
 * \brief Implementations of wireless part of ui_system_interface
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
#include "wifi_intf_link_api.h"

#define WPS_EVENT_ID 0x3B069294

#define WIFI_INTF_STA_EVENT_ID 0x4C178305
#define WIFI_INTF_UAP_EVENT_ID 0x3B2674F6


delay_msec_timer_t   *wifi_signal_strength_timer = NULL;

static bool g_wifi_connected = false;
static bool g_ui_wps_session_active = false;

#define WIFI_STA_SIGSTR_TIMER_CALLBACK_TIME  5  //check signal strength every 5 seconds for active connection


/*---------------------------------------------------------------------------*/
void wifi_sig_strength_timer_expired(ULONG unused)
{
    int32_t  rssi;
    int sig_strength;
    data_block_handle_t sigstr_data; 
 
    DPRINTF( (DBG_SOFT | DBG_OUTPUT),("CP: Checking signal strength for active connection!\n")); 
    if (g_wifi_connected)  
    {       
       if (wifi_intf_get_var_rssi_dbm(WIFI_IFACE_ID_STATION, &rssi) == WIFI_INTF_OK)
       {        
            sig_strength = rssi_dbm_to_signal_strength(rssi);
            DPRINTF( (DBG_SOFT | DBG_OUTPUT),("CP: Signal Strength[1-5]: %d and dBm = %ld\n", sig_strength  , rssi));
            sigstr_data = ui_sys_create_data_block();
            ui_sys_data_block_add_int(sigstr_data, sig_strength);
            ui_sys_notify_ui_of_event(WIFI_MODULE_NAME, WIFI_SIG_STRENGTH, sigstr_data);                
       }           
    }    
}


static void wifi_sta_event_cb(Observer_t *o, Subject_t *s)
{
    wifi_event_t event_type;
     
    // Sanity check of the observer
    ASSERT( VALIDATE_OBSERVER( o, WIFI_INTF_STA_EVENT_ID ) );

    event_type = wifi_intf_sta_get_latest_event_type(s);
    switch(event_type)
    {
        case WIFI_LINK_DOWN_EVENT:                                                
            DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("From the Wifi STA Event Observer CB: Got Wifi Link Down Event\n"));
            ui_sys_notify_ui_of_event(WIFI_MODULE_NAME, WIFI_LINK_DOWN, NULL);
            g_wifi_connected = false;
            if (wifi_signal_strength_timer != NULL)
            {       
                wifi_signal_strength_timer = delay_msec_timer_cancel(wifi_signal_strength_timer);
            }       
            break;

        case WIFI_LINK_CONNECTING_EVENT:
            DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("From the Wifi STA Event Observer CB: Got Wifi Link Connecting Event\n"));                  
            ui_sys_notify_ui_of_event(WIFI_MODULE_NAME, WIFI_LINK_CONNECTING, NULL);
            g_wifi_connected = false;
            break;
        
        case WIFI_LINK_CONNECTED_EVENT:
            DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("From the Wifi STA Event Observer CB: Got Wifi Link Connected Event\n"));
            ui_sys_notify_ui_of_event(WIFI_MODULE_NAME, WIFI_LINK_CONNECTED, NULL);
            g_wifi_connected = true;
            wifi_signal_strength_timer =  delay_msec_timer_non_blocking(1000 * WIFI_STA_SIGSTR_TIMER_CALLBACK_TIME, 
                    (void *)wifi_sig_strength_timer_expired, NULL, true);
            break;
        
        case WIFI_LINK_ERROR_EVENT:
            DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("From the Wifi STA Event Observer CB: Got Wifi Link Error Event\n"));
            ui_sys_notify_ui_of_event(WIFI_MODULE_NAME, WIFI_LINK_ERROR, NULL); 
            g_wifi_connected = false;
            if (wifi_signal_strength_timer != NULL)
            {
                wifi_signal_strength_timer = delay_msec_timer_cancel(wifi_signal_strength_timer);
            }   
            break;

        case WIFI_STA_ENABLE_EVENT:
            DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("From the Wifi STA Event Observer CB: Got Wifi Sta enabled event\n"));
            ui_sys_notify_ui_of_event(WIFI_MODULE_NAME, WIFI_STA_ENABLED, NULL); 
            break;

        case WIFI_STA_DISABLE_EVENT:
            DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("From the Wifi STA Event Observer CB: Got Wifi Sta disabled event\n"));
            ui_sys_notify_ui_of_event(WIFI_MODULE_NAME, WIFI_STA_DISABLED, NULL); 
            break;    
       
        default:
            break;                 
    }
}

static void wifi_uap_event_cb(Observer_t *o, Subject_t *s)
{
    wifi_intf_link_status_t link_status;
   
    // Sanity check of the observer
    ASSERT( VALIDATE_OBSERVER( o, WIFI_INTF_UAP_EVENT_ID ) );

    link_status = wifi_intf_uap_get_link_status(s);
    switch(link_status)
    {
        case Link_Down:                                                
            DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("From the Wifi UAP Event Observer CB: Got Wifi Link Down Event\n"));
            break;

        case Link_Connecting:            
            DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("From the Wifi UAP Event Observer CB: Got Wifi Link Connecting Event\n"));
            break;
        
        case Link_Connected:
            DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("From the Wifi UAP Event Observer CB: Got Wifi Link Connected Event\n"));
            break;
        
        case Link_Error:
            DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("From the Wifi UAP Event Observer CB: Got Wifi Link Error Event\n"));
            break;
            
        default:
            break;                 
    }
}


static void wps_event_cb(Observer_t *o, Subject_t *s)
{
    wps_event_t event_type;
    data_block_handle_t wps_data; 
    char *cur_wps_pin;

    // Sanity check of the observer
    ASSERT( VALIDATE_OBSERVER( o, WPS_EVENT_ID ) );

    event_type = wps_get_latest_event_type(s);
    switch(event_type)
    {
        case WPS_NOT_YET_RUN_EVENT:
            DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("From the WPS Event Observer CB: Got WPS Not Yet Run Event.\n"));
            break;

        case WPS_PROCESSING_EVENT:
            g_ui_wps_session_active = true;            
            DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("From the WPS Event Observer CB: Got WPS Processing Event with time : %d seconds\n", 
                    wps_get_latest_event_details(s)));
            wps_data = ui_sys_create_data_block();
            ui_sys_data_block_add_int(wps_data, wps_get_latest_event_details(s));
            cur_wps_pin = wifi_intf_get_wps_pin();
            if (cur_wps_pin)
            {
                // we have a pin so include it with this status update
                ui_sys_data_block_add_string(wps_data, cur_wps_pin);
            }
            ui_sys_notify_ui_of_status(WPS_MODULE_NAME, WPS_STATUS_SEARCHING, wps_data);                    
            break;
         
        case WPS_CONNECTING_EVENT:
            g_ui_wps_session_active = true;
            DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("From the WPS Event Observer CB: Got WPS Connecting Event\n"));                  
            ui_sys_notify_ui_of_status(WPS_MODULE_NAME, WPS_STATUS_CONNECTING, NULL);         
            break;

        case WPS_SUCCESS_EVENT:
            if (g_ui_wps_session_active)
            {
                DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("From the WPS Event Observer CB: Got WPS Success Event\n"));                  
                ui_sys_notify_ui_of_status(WPS_MODULE_NAME, WPS_STATUS_SUCCESS, NULL);
                g_ui_wps_session_active = false;        
            }          
            break;
        
        case WPS_TIMEOUT_EVENT:
            if (g_ui_wps_session_active)
            {
                DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("From the WPS Event Observer CB: Got WPS Timeout Event\n")); 
                ui_sys_notify_ui_of_status(WPS_MODULE_NAME, WPS_STATUS_NO_ROUTERS, NULL);
                g_ui_wps_session_active = false;        
            }
            break;
        
        case WPS_SESSION_OVERLAP_EVENT:
            if (g_ui_wps_session_active)
            {
                DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("From the WPS Event Observer CB: Got WPS Session Overlap Event\n"));     
                ui_sys_notify_ui_of_status(WPS_MODULE_NAME, WPS_STATUS_FAILED, NULL);
                g_ui_wps_session_active = false;
            }        
            break;  
            
        case WPS_UNKNOWN_FAILURE_EVENT:
            if (g_ui_wps_session_active)
            {
                DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("From the WPS Event Observer CB: Got WPS Unknown Failure Event\n")); 
                ui_sys_notify_ui_of_status(WPS_MODULE_NAME, WPS_STATUS_FAILED, NULL);
                g_ui_wps_session_active = false;        
            }
            break;  
           
        case WPS_CANCEL_EVENT:
            if (g_ui_wps_session_active)
            {
                DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("From the WPS Event Observer CB: Got WPS Cancel Event\n"));
                ui_sys_notify_ui_of_status(WPS_MODULE_NAME, WPS_STATUS_IDLE, NULL);
                g_ui_wps_session_active = false;        
            }
            break;  
    }
}

static bool wps_process_user_feedback(const char *command)
{
    bool processed = true;

    if (g_ui_wps_session_active && (strcmp(UI_EVENT_BTN_CANCEL, command) == 0))
    {
        //inform WPS first so that we dont receive anymore "searching" notifications
        CancelWPSSession();

        //Stop WPS
        wifi_intf_stop_wps();
        
        //update the wps module status, so that things look "clean" while displaying messages on GUI 
        ui_sys_notify_ui_of_status(WPS_MODULE_NAME, WPS_STATUS_CANCELLED, NULL);
    }
    else if (strcmp(UI_EVENT_RESPONSE_OK, command) == 0)
    {        
        //We would have reached here when one of the following cases is true:
        //1. On a successful WPS connection
        //2. On WPS connection failure
        //3. When no routers were found in WPS search
        //4. When there was a WPS session overlap
        //5. Any other unknown failure
        //In all cases, push the WPS module to idle state.  
        ui_sys_notify_ui_of_status(WPS_MODULE_NAME, WPS_STATUS_IDLE, NULL);
    }    
    else
    {
        processed = false;
    }

    return processed;
}

void ui_sys_wireless_init(void)
{
   uint32_t wifi_link_status;
       
   // before attaching any observers, register for extension utils status support
   ui_sys_register_module_for_status(WIFI_MODULE_NAME,  WIFI_STATUS_READY,  NULL);
   ui_sys_register_module_for_status(WPS_MODULE_NAME,  WPS_STATUS_IDLE,  wps_process_user_feedback);

   // attach observers to desired subjects
   wps_attach_event_observer( Observer_Constructor( WPS_EVENT_ID,   wps_event_cb) );
   wifi_intf_sta_attach_event_observer( Observer_Constructor( WIFI_INTF_STA_EVENT_ID,   wifi_sta_event_cb) );
   wifi_intf_uap_attach_event_observer( Observer_Constructor( WIFI_INTF_UAP_EVENT_ID,   wifi_uap_event_cb) );   
     
   //If the link is already established before the extension was initialized, check for the current link status and
   //send an event accordingly
   wifi_link_status = wifi_intf_sta_get_current_link_status();

   switch(wifi_link_status)
   {
        case Link_Down:                                                
            DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("From the Wifi kinoma extension init: Link Down\n"));        
            ui_sys_notify_ui_of_event(WIFI_MODULE_NAME, WIFI_LINK_DOWN, NULL);
            g_wifi_connected = false;
            if (wifi_signal_strength_timer != NULL)
            {       
                wifi_signal_strength_timer = delay_msec_timer_cancel(wifi_signal_strength_timer);
            }   
            break;

        case Link_Connecting:            
            DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("From the Wifi kinoma extension init: Link Connecting\n"));                  
            ui_sys_notify_ui_of_event(WIFI_MODULE_NAME, WIFI_LINK_CONNECTING, NULL);
            g_wifi_connected = false;
            break;
        
        case Link_Connected:
            DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("From the Wifi kinoma extension init: Link Connected\n"));
            ui_sys_notify_ui_of_event(WIFI_MODULE_NAME, WIFI_LINK_CONNECTED, NULL);
            g_wifi_connected = true;
            wifi_signal_strength_timer =  delay_msec_timer_non_blocking(1000 * WIFI_STA_SIGSTR_TIMER_CALLBACK_TIME, 
                    (void *)wifi_sig_strength_timer_expired, NULL, true);
            break;
        
        case Link_Error:
            DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("From the Wifi kinoma extension init: Link Error\n"));
            ui_sys_notify_ui_of_event(WIFI_MODULE_NAME, WIFI_LINK_ERROR, NULL); 
            g_wifi_connected = false;
            if (wifi_signal_strength_timer != NULL)
            {
                wifi_signal_strength_timer = delay_msec_timer_cancel(wifi_signal_strength_timer);
            }   
            break;
            
        default:
            break;                 
   } 
   
}

/*Station related functions*/
/********************************************************/

/*RestoreDefaults
  */
bool ui_sys_wireless_station_restore_defaults(void)
{
    wifi_intf_var_restore_defaults(WIFI_IFACE_ID_STATION, WIFI_INTF_VAR_ALL);
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("In Sta Restoring Defaults\n") );

    return  true;
}

/*Get Station status
  returns: Wifi Station status as enabled or disabled
 */
bool ui_sys_wireless_station_enabled_get(bool * enabled)
{
    ASSERT(enabled != NULL);

    eth_mac_link_status_t link_status;

    net_eth_get_var_link_status(&link_status);

    //If the ethrnet cable is plugged in, report that the station is disabled. UI layer uses this to display if wifi is on/off.
    if (link_status == ETH_MAC_LINK_STATUS_DOWN)    
        wifi_intf_get_var_is_enabled(WIFI_IFACE_ID_STATION, enabled);
    else
        *enabled = false; 
    
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("In Sta isEnabled getter: Currently WiFi Station is : %s\n", enabled ? "Enabled" : "Disabled") );

    return  true;
}

/*Set Station status
 */
bool ui_sys_wireless_station_enabled_set(bool enabled)
{
    bool result = true;    

    wifi_intf_set_var_is_enabled(WIFI_IFACE_ID_STATION, enabled);
    wifi_intf_get_var_is_enabled(WIFI_IFACE_ID_STATION, &enabled);
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("In Sta isEnabled setter: Currently WiFi Station is : %s\n", enabled ? "Enabled" : "Disabled") );

    return  result;
}

/*Get Station SSID
  returns: Wifi Station SSID as a string
 */
bool ui_sys_wireless_station_ssid_get(char * ssid_data, const size_t size)
{    
    ASSERT(ssid_data != NULL);
    if(size < 128)
    {
        return  false;
    }

    wifi_intf_get_var_ssid(WIFI_IFACE_ID_STATION, (uint8_t *)ssid_data, size);  
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("In Sta get ssid: SSID is : %s\n", ssid_data) );

    return  true;
}

/*Set Station SSID
 */
bool ui_sys_wireless_station_ssid_set(char * ssid_data)
{    
    ASSERT(ssid_data != NULL);

    bool result = true;  

    wifi_intf_set_var_ssid(WIFI_IFACE_ID_STATION, (uint8_t*)ssid_data, strlen(ssid_data) + 1);
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("In Sta set ssid: SSID received : %s\n", ssid_data) );

    return  result;
}

/*Get Station Security Mode
  returns: Wifi Station Security Mode (open, wep, wpa, wpa2)
 */
bool ui_sys_wireless_station_secuity_mode_get(wifi_intf_security_mode_t * sec_mode)
{    
    ASSERT(sec_mode != NULL);

    wifi_intf_get_var_sec_mode(WIFI_IFACE_ID_STATION, sec_mode);  

    XASSERT(*sec_mode == WEP        ||
            *sec_mode == WPA_PSK    ||
            *sec_mode == WPA2_PSK   ||
            *sec_mode == OPEN_SYSTEM, *sec_mode);                       
    
    return  true;
}

/*Set Station Security Mode (open, wep, wpa, wpa2)
  returns: true if success, false if failure   
 */
bool ui_sys_wireless_station_security_mode_set(wifi_intf_security_mode_t sec_mode)
{
    bool result = true; 

    XASSERT(sec_mode == WEP         ||
            sec_mode == WPA_PSK     ||
            sec_mode == WPA2_PSK    ||
            sec_mode == OPEN_SYSTEM, sec_mode);                     


    wifi_intf_set_var_sec_mode(WIFI_IFACE_ID_STATION, sec_mode);
    
    return  result;
}

/*Get Station Wep Authentication mode
  returns: Wifi Station Wep Authentication mode (open, shared, auto)
 */
bool ui_sys_wireless_station_wep_auth_mode_get(wifi_intf_wep_auth_mode_t * auth_mode)
{    
    ASSERT(auth_mode != NULL);

    wifi_intf_get_var_wep_auth_mode(WIFI_IFACE_ID_STATION, auth_mode); 
    
    return  true;
}

/*Set Station WEP Authentication Mode (open, shared, auto)
  returns: true if success, false if failure   
 */
bool ui_sys_wireless_station_wep_authMode_set(wifi_intf_wep_auth_mode_t auth_mode)
{
    bool result = true; 
    
    wifi_intf_set_var_wep_auth_mode(WIFI_IFACE_ID_STATION, auth_mode);
    
    return  result;
}

/*Get Station WEP Key
  index: Numeric Value indicating the WEP key index to view
  returns: Wifi Station WEP key as a string (should correspond to 10 hex digits)    
 */
bool ui_sys_wireless_station_get_wep_key(const uint8_t index, char * key_data, const size_t size)
{    
    ASSERT(key_data != NULL);
    if(size < 128)
    {
        return  false;
    }

    wifi_intf_get_var_wep_key(WIFI_IFACE_ID_STATION, index, (uint8_t *)key_data, size);  
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("In Sta get WepKey: Key Requested for is : %d and Key data is %s\n", index, key_data) );

    return  true;
}

/*Set Station WEP Key
  index: Numeric Value indicating the WEP key index to view
  key  : Key as a string (should correspond to 10 hex digits)
*/
bool ui_sys_wireless_station_set_wep_key(const uint8_t index, const char * key)
{    
    ASSERT(key != NULL);

    bool result = true;

    wifi_intf_set_var_wep_key(WIFI_IFACE_ID_STATION, index, (uint8_t*)key, strlen(key) + 1);
    wifi_intf_set_var_wep_key_len(WIFI_IFACE_ID_STATION, strlen(key));
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("In Sta Set WepKey: Key Requested for is : %d and Key data is %s\n", index, key) );

    return  result;
}

/*Get Channel 
  returns: Channel as an integer
 */
bool ui_sys_wireless_station_get_channel(uint8_t * channel)
{
    ASSERT(channel != NULL);    

    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("In Sta get channel") );
    wifi_intf_get_var_channel(WIFI_IFACE_ID_STATION, channel);

    return  true;
}

/*Get Station WEP Current Key : Current WEP key in use
  returns: Wifi Station WEP key index as an integer
 */
bool ui_sys_wireless_station_wep_current_key_get(uint8_t * cur_key)
{    
    ASSERT(cur_key != NULL);

    wifi_intf_get_var_wep_cur_key(WIFI_IFACE_ID_STATION, cur_key);
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("In Sta get WepCurrentKey: The current key is  %d\n", cur_key) );

    return  true;    
}

/*Set Station WEP Current Key 
  index : index of the key (as an integer) to be used as the current key, should be in the range 0-3 
  returns : false if the key is not in the range 0-3, else true
 */
bool ui_sys_wireless_station_wep_current_key_set(uint8_t cur_key)
{               
    wifi_intf_set_var_wep_cur_key(WIFI_IFACE_ID_STATION, cur_key);
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("In Sta set WepCurrentKey: The current key is  %d\n", cur_key) );
    
    return  true;
}

/*Get Station WPA passphrase : Current WPA/WPA2 passphrase in use
  returns: Wifi Station WPA/WPA2 passphrase as a string 
 */
bool ui_sys_wireless_station_wpa_passphrase_get(char * passphrase, const size_t size)
{   
    ASSERT(passphrase != NULL);
    if(size < 128)
    {
        return  false;
    }

    wifi_intf_get_var_wpa_passphrase(WIFI_IFACE_ID_STATION, (uint8_t *)passphrase, size);
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("In Sta get WpaPassphrase: Current passphrase is %s\n", passphrase) );

    return  true;
}

/*Set Station WPA/WPA2 passphrase 
 */
bool ui_sys_wireless_station_wpa_passphrase_set(const char *passphrase)
{    
    ASSERT(passphrase != NULL);

    bool result = true;

    wifi_intf_set_var_wpa_passphrase(WIFI_IFACE_ID_STATION, (uint8_t *)passphrase, strlen(passphrase) + 1);       
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("In Sta set WpaPassphrase: Current passphrase is %s\n", passphrase) );

    return  result;
}

/*Get Station communication mode 
  returns: Wifi Station communication mode as a string (Adhoc or Infrastructure)
 */
bool ui_sys_wireless_station_comm_mode_get(wifi_intf_comm_mode_t * comm_mode)
{    
    ASSERT(comm_mode != NULL);

    wifi_intf_get_var_comm_mode(WIFI_IFACE_ID_STATION, comm_mode);

    return  true;
}

/*Station Generate WPS PIN
returns: WPS PIN as a string

Noted: The caller has the responsibility to free the returned string by MEM_FREE_AND_NULL, otherwise memory leak.
 */
char * ui_sys_wireless_station_generate_wps_pin(void)
{   
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Control Panel Generated PIN\n") );     

    return generate_wps_pin();
}

/*Station StartWPS
pin: provide WPS PIN as a string, ideally use GenerateWPSPin to generate a PIN first and 
     then provide that as an input to this function.
 */
bool ui_sys_wireless_station_start_wps(const char * start_type, const char * pin)
{
   if (strcmpi("pin", start_type) == 0)
   {
       DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Control Panel has started WPS with PIN: %s\n", pin) );
       wifi_intf_start_wps(NULL, 0, (uint8_t *)pin, strlen(pin));
   }
   else if (strcmpi("pb", start_type) == 0)
   {
       DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Control Panel has started WPS with Push Button mode\n"));
       wifi_intf_start_wps(NULL, 0, NULL, 0);   
   }
   
   return   true;
}

/*Station RefreshWPSTimer
 */
bool ui_sys_station_refresh_wps_timer(void)
{
   DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Control Panel is attempting to Refresh the WPS Timer\n"));   
   wifi_intf_start_wps(NULL, 0, NULL, 0);   
       
   return   true;
}

/*Station StopWPS
 */
void ui_sys_wireless_station_stop_WPS(void)
{
   DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Control Panel is Stopping WPS!\n") );    
   wifi_intf_stop_wps();    
   return;
}

/*Set Station communication mode 
  returns: false if there is an invalid communication mode, else true
 */
bool ui_sys_wireless_station_comm_mode_set(wifi_intf_comm_mode_t comm_mode)
{   
    bool result = true; 
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("In Sta set comm mode") );
    
    if (comm_mode == AD_HOC)
    {
       DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Setting Communication Mode to Ad Hoc\n") );                  
    }
    else if (comm_mode == INFRASTRUCTURE)
    {
       DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Setting Communication mode to Infrastructure\n") );      
    }       
    else
    {
       result = false;
    }    

    wifi_intf_set_var_comm_mode(WIFI_IFACE_ID_STATION, comm_mode);
    return  result;
}

/*Get Station link status 
  returns: link status
 */
bool ui_sys_wireless_station_get_link_status(uint32_t * wifi_link_status) 
{
    //If the link is already established before the extension was initialized, check for the current link status and
    //send an event accordingly
    *wifi_link_status = wifi_intf_sta_get_current_link_status(); 

    return  true;
}    

/*MicroAP related functions*/
/********************************************************/

/*RestoreDefaults
  */
bool ui_sys_wireless_microap_restore_defaults(void)
{
    bool enabled;

    error_type_t res;
        
    res = wifi_intf_var_restore_defaults(WIFI_IFACE_ID_UAP, WIFI_INTF_VAR_ALL);
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("In uAP Restoring Defaults\n") );

    enabled = res == OK ? true : false;

    return  enabled;
}

/*Get uAP status
  returns: Wifi MicroAP status as enabled or disabled
 */
bool ui_sys_wireless_microap_enabled_get(bool * enabled)
{
    wifi_intf_get_var_is_enabled(WIFI_IFACE_ID_UAP, enabled);
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("In uAP isEnabled getter: Currently WiFi uAP is : %s\n", enabled ? "Enabled" : "Disabled") );

    return  true;
}

/*Set uAP status
 */
bool ui_sys_wireless_microap_enabled_set(const bool enabled)
{
    bool result = true;    
    
    bool cur_status;

    wifi_intf_set_var_is_enabled(WIFI_IFACE_ID_UAP, enabled);
    wifi_intf_get_var_is_enabled(WIFI_IFACE_ID_UAP, &cur_status);
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("In uAP isEnabled setter: Currently WiFi uAP is : %s\n", cur_status ? "Enabled" : "Disabled") );

    return  result;
}

/*Get uAP SSID
  returns: Wifi uAP SSID as a string

 */
bool ui_sys_wireless_microap_ssid_get(char * ssid, const size_t size)
{    
    ASSERT(ssid != NULL);
    if(size < 128)
    {
        return  false;
    }   

    wifi_intf_get_var_ssid(WIFI_IFACE_ID_UAP, (uint8_t *)ssid, size);
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("In uAP get ssid: SSID is : %s\n", ssid) );

    return  true;
}

/*Set uAP SSID
 */
bool ui_sys_wireless_microap_ssid_set(const char * ssid)
{    
    bool result = true; 
    
    wifi_intf_set_var_ssid(WIFI_IFACE_ID_UAP, (uint8_t*)ssid, strlen(ssid) + 1);    
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("In uAP set ssid: SSID received : %s\n", ssid) );

    return  result;
}

/*Get uAP SSID Prefix
  returns: Wifi uAP SSID Prefix as a string
 */
bool ui_sys_wireless_microap_ssid_prefix_get(char * prefix, const size_t size)
{    
    ASSERT(prefix != NULL);
    if(size < 128)
    {
        return  false;
    }

    wifi_intf_get_var_ssid_prefix(WIFI_IFACE_ID_UAP, (uint8_t *)prefix, size);
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("In uAP get ssid prefix: SSID Prefix is : %s\n", prefix) );

    return  true;
}

/*Get uAP Security Mode
  returns: Wifi uAP Security Mode as a String (open, wep, wpa, wpa2)
 */
bool ui_sys_wireless_microap_security_mode_get(wifi_intf_security_mode_t * sec_mode)
{    

    ASSERT(sec_mode != NULL);

    wifi_intf_get_var_sec_mode(WIFI_IFACE_ID_UAP, sec_mode);  
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("In uAP get security mode: ") );

#ifdef  DEBUG
    switch(*sec_mode)
    {
        case OPEN_SYSTEM : 
              DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Security mode is OPEN\n") );
              break;

        case WEP : 
              DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Security mode is WEP\n") );
              break;

        case WPA_PSK : 
              DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Security mode is WPA\n") );
              break;

        case WPA2_PSK : 
              DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Security mode is WPA2\n") );
              break;

        default : 
              DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Error in get security mode!\n") );
              return    false;
    }
#endif

    return  true;
}

/*Set uAP Security Mode (open, wep, wpa, wpa2)
  returns: true if success, false if failure   
 */
bool ui_sys_wireless_microap_security_mode_set(wifi_intf_security_mode_t sec_mode)
{
    bool result = true; 

    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("In uAP set security mode: ") );

    switch(sec_mode)
    {
    case WEP:
        DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Setting security mode to WEP\n") );                     
        break;
    case WPA_PSK:
        DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Setting security mode to WPA\n") );         
        break;
    case WPA2_PSK:
        DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Setting security mode to WPA2\n") );        
        break;
    case OPEN_SYSTEM:
        DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Setting security mode to Open System\n") );         
        break;
    default:
        result = false;
    }
    wifi_intf_set_var_sec_mode(WIFI_IFACE_ID_UAP, sec_mode);
    
    return  result;
}

/*Get uAP Wep Authentication mode
  returns: Wifi uAP Wep Authentication mode as a String (open, shared, auto)
 */
bool ui_sys_wireless_microap_wep_auth_mode_get(wifi_intf_wep_auth_mode_t * auth_mode)
{    
    ASSERT(auth_mode != NULL);

    wifi_intf_get_var_wep_auth_mode(WIFI_IFACE_ID_UAP, auth_mode);  
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("In uAP get security mode: ") );

    switch(*auth_mode)
    {
        case AUTH_OPEN : 
            DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Auth mode is open\n") );
            break;
        
        case AUTH_SHARED_KEY : 
            DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Auth mode is shared key\n") );
            break;
        
        case AUTH_AUTO : 
            DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Auth mode is Auto\n") );
            break;

        default : 
            DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Error in get wep auth mode!\n") );
            ASSERT(false);
    }

    return  true;
}

/*Set uAP WEP Authentication Mode (open, shared, auto)
  returns: true if success, false if failure   
 */
bool ui_sys_wireless_microap_wep_auth_mode_set(wifi_intf_wep_auth_mode_t auth_mode)
{
    bool result = true; 

    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("In uAP set WepAuthMode: ") );

    if (auth_mode == AUTH_OPEN)
    {
       DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Setting WepAuthMode to Open\n") );                   
    }
    else if (auth_mode == AUTH_SHARED_KEY)
    {
       DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Setting WepAuthMode to Shared\n") );     
    }
    else if (auth_mode == AUTH_AUTO)
    {
       DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Setting WepAuthMode to Auto\n") );       
    }   
    else
    {
       result = false;
    }

    wifi_intf_set_var_wep_auth_mode(WIFI_IFACE_ID_UAP, auth_mode);
    
    return result;
}

/*Get uAP WEP Key
  index: Numeric Value indicating the WEP key index to view
  returns: Wifi uAP WEP key as a string (should correspond to 10 hex digits)
 */
bool ui_sys_wireless_microap_get_wep_key(const uint8_t index, char * key_data, const size_t size)
{    
    ASSERT(key_data != NULL);
    if(size < 128)
    {
        return  false;
    }

    wifi_intf_get_var_wep_key(WIFI_IFACE_ID_UAP, index, (uint8_t *)key_data, size);    
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("In uAP get WepKey: Key Requested for is : %d and Key data is %s\n", index, key_data) );

    return  true;
}

/*Set uAP WEP Key
  index: Numeric Value indicating the WEP key index to view
  key  : Key as a string (should correspond to 10 hex digits)
*/
bool ui_sys_wireless_microap_set_wep_key(const uint8_t index, char * wep_key)
{    
    bool result = true;

    wifi_intf_set_var_wep_key(WIFI_IFACE_ID_UAP, index, (uint8_t*)wep_key, strlen(wep_key) + 1);
    wifi_intf_set_var_wep_key_len(WIFI_IFACE_ID_UAP, strlen(wep_key));
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("In uAP Set WepKey: Key Requested for is : %d and Key data is %s\n", index, wep_key) );

    return  result;
}

/*Get uAP WEP Current Key : Current WEP key in use
  returns: Wifi uAP WEP key index as an integer
 */
bool ui_sys_wireless_microap_wep_current_key_get(uint8_t * cur_key)
{    
    ASSERT(cur_key != NULL);

    wifi_intf_get_var_wep_cur_key(WIFI_IFACE_ID_UAP, cur_key);
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("In uAP get WepCurrentKey: The current key is  %d\n", cur_key) );

    return  true;
}

/*Set uAP WEP Current Key 
  index : index of the key (as an integer) to be used as the current key, should be in the range 0-3 
  returns : false if the key is not in the range 0-3, else true
 */
bool ui_sys_wireless_microap_wep_current_key_set(const uint8_t cur_key)
{    
    wifi_intf_set_var_wep_cur_key(WIFI_IFACE_ID_UAP, cur_key);
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("In uAP set WepCurrentKey: The current key is  %d\n", cur_key) );
    
    return  true;
}

/*Get uAP WPA passphrase : Current WPA/WPA2 passphrase in use
  returns: Wifi uAP WPA/WPA2 passphrase as a string

    Noted: The caller has the responsility to free the returned string by MEM_FREE_AND_NULL 
 */
bool ui_sys_wireless_microap_wpa_passphrase_get(char * passphrase_data, const size_t size)
{    
    ASSERT(passphrase_data != NULL);
    if(size < 128)
    {
        return  false;
    }

    wifi_intf_get_var_wpa_passphrase(WIFI_IFACE_ID_UAP, (uint8_t *)passphrase_data, size);       
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("In uAP get WpaPassphrase: Current passphrase is %s\n", passphrase_data) );

    return  true;
}

/*Set uAP WPA/WPA2 passphrase 
 */
bool ui_sys_wireless_microap_wpa_passphrase_set(const char * passphrase_data)
{    
    bool result = true;

    wifi_intf_set_var_wpa_passphrase(WIFI_IFACE_ID_UAP, (uint8_t *)passphrase_data, strlen(passphrase_data) + 1);       
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("In uAP set WpaPassphrase: Current passphrase is %s\n", passphrase_data ) );

    return  result;
}

/*Get uAP Encryption Mode
  returns: Wifi uAP Encryption Mode as a String (tkip, aes, both)
 */
bool ui_sys_wireless_microap_encrypt_mode_get(wifi_intf_wpa_encryp_mode_t * enc_mode)
{    
    ASSERT(enc_mode != NULL);

    wifi_intf_get_var_wpa_encryp_mode(WIFI_IFACE_ID_UAP, enc_mode);  
    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("In uAP get security mode: ") );

    switch(*enc_mode)
    {
        case WPA_TKIP : 
           DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Enc mode is tkip\n") );
           break;

        case WPA_AES  : 
           DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Enc mode is aes\n") );
           break;

        case WPA_TKIP_AND_AES : 
           DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Enc mode is both tkip and aes\n") );
           break;

        default :
           DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Error in get EncMode!\n") );
           ASSERT(false);
    }

    return  true;
}

/*Set uAP Encryption Mode (tkip, aes, both)
  returns: true if success, false if failure   
 */
bool ui_sys_wireless_microap_encryp_mode_set(wifi_intf_wpa_encryp_mode_t enc_mode)
{
    bool result = true; 

    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("In uAP set EncMode: ") );

    if (enc_mode == WPA_TKIP)
    {
       DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Setting EncMode to tkip\n") );                   
    }
    else if (enc_mode == WPA_AES)
    {
       DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Setting EncMode to aes\n") );        
    }
    else if (enc_mode == WPA_TKIP_AND_AES)
    {
       DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Setting EncMode to both tkip and aes\n") );             
    }   
    else
    {
       result = false;
    }

    wifi_intf_set_var_wpa_encryp_mode(WIFI_IFACE_ID_UAP, enc_mode);
    
    return  result;
}

/*Generate a random passphrase for the specified length. 
 * Note that the min and max value for length
 * are specified by WIFI_UAP_MIN_PASSPHRASE_LENGTH and 
 * WIFI_UAP_MAX_PASSPHRASE_LENGTH respectively.
   returns: the randomly generated passphrase as a string   

    Noted: The caller has the responsility to free the returned string by MEM_FREE_AND_NULL 
 */
char * ui_sys_wireless_microap_generate_random_passphrase(void)
{
    uint8_t *WPAPassPhrase = (uint8_t *)MEM_MALLOC(WIFI_UAP_MAX_PASSPHRASE_LENGTH + 1);
    ASSERT(WPAPassPhrase != NULL);  
    
    wifi_gen_random_passphrase(WIFI_IFACE_ID_UAP, WPAPassPhrase, WIFI_UAP_MAX_PASSPHRASE_LENGTH + 1);
    
    return  (CHAR *)WPAPassPhrase;
}

/*Wifi Scan related functions*/
/********************************************************/

/*StartScan 
 */
bool ui_system_inteface_wireless_start_scan(const char *ssid)
{    
    bool result = true;
    int len; 
    
    if (ssid == NULL)
    {
        /*Generic Scan*/        
        wifi_intf_start_scan(NULL, 0);
    }
    else
    {
        len = strlen (ssid);
        /*Directed Scan*/
        wifi_intf_start_scan( (uint8_t*)ssid, len );
    }

    DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("In Wifi StartScan\n"));

    return  result;
}

/*isScanning 
 */
bool ui_sys_wireless_is_scanning(void)
{    
    bool scanning;

    scanning = true;
    wifi_intf_get_var_is_wireless_scanning(&scanning);
       
    if (scanning)
    {       
        DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Still Scanning!\n"));
    }   
    else
    {       
        DPRINTF( (DBG_SOFT | DBG_OUTPUT), ("Scanning complete!\n"));
    }

    return  scanning;
}

/*GetScanResults 
 */
bool ui_sys_wireless_get_scan_results(wifi_intf_scan_info_t *scan_info)
{
    ASSERT(scan_info != NULL);
    
    wifi_intf_get_var_scan_info(scan_info);

    return  true;   
}

/*ScanSleep: Used during the blocking WiFi scan 
*/
bool ui_sys_wireless_scan_sleep(void)
{
    DPRINTF( (DBG_SOFT | DBG_OUTPUT),("CP: In ScanSleep \n"));  
    tx_thread_sleep(10);    
    return true;
}

/*GetMacAddress: returns the MAC address as a string

    The buffer size of macOnBoard must >= MAC_ADDR_LENGTH    
*/
bool ui_sys_wireless_get_mac_address(uint8_t * macOnBoard)
{
    char buffer[64];

    wifi_intf_get_var_mac_addr(0, macOnBoard, MAC_ADDR_LENGTH);
    DPRINTF( (DBG_SOFT | DBG_OUTPUT),("Control Panel is printing out the MAC Address: "));
    DPRINTF( (DBG_SOFT | DBG_OUTPUT),("%02X:%02X:%02X:%02X:%02X:%02X\n",
                macOnBoard[0], macOnBoard[1], macOnBoard[2],
                macOnBoard[3], macOnBoard[4], macOnBoard[5]));


    minSprintf(buffer, "%02X:%02X:%02X:%02X:%02X:%02X" ,
            macOnBoard[0], macOnBoard[1], macOnBoard[2],
            macOnBoard[3], macOnBoard[4], macOnBoard[5]);   

    DPRINTF( (DBG_SOFT | DBG_OUTPUT),("MAC as a string : %s\n",
                buffer));

    return  true;
}


bool ui_sys_wireless_restore_default(void)
{
    //Restore all wireless settings to defaults
    wifi_intf_var_oem_restore_wireless_defaults();

    bool isStationMode = false;
    ui_sys_wireless_station_enabled_get(&isStationMode);
    if(isStationMode)
    {
        ui_sys_wireless_station_restore_defaults();
    }

    bool isuAPMode = false;
    ui_sys_wireless_microap_enabled_get(&isuAPMode);
    if(isuAPMode)   
    {
        ui_sys_wireless_microap_restore_defaults();
    }   
        
    return  true;
}
