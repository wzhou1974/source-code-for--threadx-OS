/*
 * ============================================================================
 * Copyright (c) 2013   Marvell International, Ltd. All Rights Reserved
 *
 *                         Marvell Confidential
 * ============================================================================
 */

/** 
 * \file ui_system_interface_scan.c
 * 
 * \brief Implementations of scan part of ui_system_interface
 *        
 * 
 **/

#include <string.h>
#include "tx_api.h"
#include "ATypes.h"
#include "platform_api.h"
#include "event_observer.h"
#include "response_subject.h"
#include "scantypes.h"              // for scanman.h
#include "scanman.h"
#include "cal.h"
#include "calstats.h"
#include "scanhwerr.h"
#include "scanmech.h"
#include "adfsensor.h"

#ifdef HAVE_WSD_SCAN
#include "wsdscan_observer.h"
#endif

#ifdef HAVE_SOAP_SCAN
#include "soapscan_observer.h"
#endif

#include "lassert.h"

#include "ui_system_interface_api.h"
#include "ui_system_interface_utils.h"
#include "ui_system_interface_scan.h"
#include "ui_system_interface_oem_scan.h"

#ifdef DEBUG
#include "logger.h"
#define mydbg_prrintf dbg_printf
#else
#define mydbg_prrintf(...)
#endif

typedef enum
{
    SCANCORE_EVENT_NULL=0, 
    SCANCORE_EVENT_IDLE=1,
    SCANCORE_EVENT_SCAN_START=2,
    SCANCORE_EVENT_SCAN_RUNNING=3,
    SCANCORE_EVENT_SCAN_COMPLETE=4,
    SCANCORE_EVENT_HOST_CANCEL=5,
    SCANCORE_EVENT_DEVICE_CANCEL=6,
    SCANCORE_EVENT_TIMEOUT=7,
    SCANCORE_EVENT_SCAN_OFFLINE=8,
    SCANCORE_EVENT_SCAN_JAM,
    SCANCORE_EVENT_SCAN_NOPICK,
    SCANCORE_EVENT_BUSY,
    SCANCORE_EVENT_OFFLINE,
} scancore_event_t;

#define SCAN_EVENT_ID 0xD85E1E4D

static int last_known_paper_status = 0;
static bool is_wsdscan_running;
static bool is_soapscan_running;
static bool is_aspscan_running = false;
static bool bad_pp_flag = false;




#ifdef HAVE_SOAP_SCAN
static soapscan_event_t current_soapscan_state = SOAPSCAN_EVENT_NULL;
soapscan_event_t get_soapscan_status()
{
    return current_soapscan_state;
}
#endif

#ifdef HAVE_WSD_SCAN
static wsdscan_event_t current_wsdscan_state = WSDSCAN_EVENT_NULL;
wsdscan_event_t get_wsdscan_status()
{
    return current_wsdscan_state;
}
#endif

static scancore_event_t scan_device_state = SCANCORE_EVENT_NULL;

scancore_event_t get_scancore_status()
{
    return scan_device_state;
}

static void send_paper_present_ui_event(int paper_present)
{
    data_block_handle_t state_data;
    if(-1 == paper_present)
    {
        //dbg_printf("get into\n");
        bad_pp_flag = true;
        ui_sys_notify_ui_of_status(SCAN_MODULE_NAME,SCAN_STATUS_PP_FAILURE, NULL);  
        return;
    }
    else if(bad_pp_flag)
    {
        //dbg_printf("clear it\n");
        bad_pp_flag = false;
        ui_sys_notify_ui_of_status(SCAN_MODULE_NAME,SCAN_STATUS_IDLE, NULL);  
    }

    state_data = ui_sys_create_data_block();
    ui_sys_data_block_add_int(state_data, paper_present);
    ui_sys_notify_ui_of_event(SCAN_MODULE_NAME, SCAN_EVENT_PAPER_PRESENT, state_data);              
}

#ifdef HAVE_SOAP_SCAN
static void soapscan2pc_event_cb(Observer_t *o, Subject_t *s)
{
    // Sanity check of the observer
    ASSERT( VALIDATE_OBSERVER( o, SCAN_EVENT_ID ) );
    if (s != NULL)
    {
        soapscan_observer_data_t *subject = (soapscan_observer_data_t *)s;

        switch (subject->event)
        {
        case SOAPSCAN_EVENT_SCAN_RUNNING:
            {
              //  if(current_scan_state != SOAPSCAN_EVENT_SCAN_RUNNING)
                {
                    mydbg_prrintf("ui_sys_scan:SOAPSCAN_EVENT_SCAN_RUNNING\n");
                    is_soapscan_running = true;
                    is_wsdscan_running = false;
                    current_soapscan_state = SOAPSCAN_EVENT_SCAN_RUNNING;
                    ui_sys_notify_ui_of_status(SCAN_MODULE_NAME,SCAN_STATUS_RUNNING, NULL);  
                }
            }
            break;
        case SOAPSCAN_EVENT_SCAN_COMPLETE:
            {
                mydbg_prrintf("ui_sys_scan:SOAPSCAN_EVENT_SCAN_COMPLETE\n");
                is_soapscan_running=false;
               // if(current_scan_state != SOAPSCAN_EVENT_NULL)
                {
                    current_soapscan_state = SOAPSCAN_EVENT_SCAN_COMPLETE;
                    ui_sys_notify_ui_of_status(SCAN_MODULE_NAME,SCAN_STATUS_IDLE, NULL);  
                }
            }
            break;
        case SOAPSCAN_EVENT_TIMEOUT:
            {
                mydbg_prrintf("ui_sys_scan:SOAPSCAN_EVENT_TIMEOUT device busy\n");
                is_soapscan_running = false;
                {
                    current_soapscan_state = SOAPSCAN_EVENT_SCAN_COMPLETE;
                    ui_sys_notify_ui_of_status(SCAN_MODULE_NAME,SCAN_STATUS_BUSY, NULL);  
                }
            }
            break;
        }
    }
}
#endif

#ifdef HAVE_WSD_SCAN
static void wsdscan2pc_event_cb(Observer_t *o, Subject_t *s)
{
    // Sanity check of the observer
    ASSERT( VALIDATE_OBSERVER( o, SCAN_EVENT_ID ) );
    if (s != NULL)
    {
        wsdscan_observer_data_t *subject = (wsdscan_observer_data_t *)s;

        switch (subject->event)
        {
        case WSDSCAN_EVENT_SCAN_RUNNING:
            {
                {
                    mydbg_prrintf("ui_sys_scan:WSDSCAN_EVENT_SCAN_RUNNING\n");
                    current_wsdscan_state = WSDSCAN_EVENT_SCAN_RUNNING;
                    is_wsdscan_running = true;
                    is_soapscan_running = false;
                    ui_sys_notify_ui_of_status(SCAN_MODULE_NAME,SCAN_STATUS_RUNNING, NULL);  
                }
            }
            break;
        case WSDSCAN_EVENT_SCAN_COMPLETE:
            {
                mydbg_prrintf("ui_sys_scan:WSDSCAN_EVENT_SCAN_COMPLETE\n");
                {
                    current_wsdscan_state = WSDSCAN_EVENT_SCAN_COMPLETE;
                    is_wsdscan_running = false;
                    ui_sys_notify_ui_of_status(SCAN_MODULE_NAME,SCAN_STATUS_IDLE, NULL);  
                }
            }
            break;
        default:
            {
                // do nothing, only want to make compiler happy!
            }
        }
    }
}
#endif

static void scanner_event_cb(Observer_t *o, Subject_t *s)
{
    data_block_handle_t scan_err_data; 

    // Sanity check of the observer
    ASSERT( VALIDATE_OBSERVER( o, SCAN_EVENT_ID ) );

    if (s != NULL)
    {
        scanman_observer_data_t *subject = (scanman_observer_data_t *)s;

        switch (subject->event)
        {
        case SCANMAN_EVENT_ADF_PAPER_PRESENT:
            {

                if (subject->value != last_known_paper_status)
                {
                    last_known_paper_status = subject->value;
                    send_paper_present_ui_event(subject->value);
                }
                break;
            }
        case SCANMAN_EVENT_ADF_PAPER_JAM:
            {
                mydbg_prrintf("ui_sys_scan:SCAN_ADF_PAPER_JAM\n");
                scan_device_state = SCANCORE_EVENT_SCAN_JAM;
                ui_sys_notify_ui_of_status(SCAN_MODULE_NAME,SCAN_STATUS_PAPER_JAM, NULL);  
            }
            break;
        case SCANMAN_EVENT_ADF_PAPER_NOPICK:
            {
                mydbg_prrintf("ui_sys_scan:SCANMAN_EVENT_ADF_PAPER_NOPICK\n");
                scan_device_state = SCANCORE_EVENT_SCAN_NOPICK;
                ui_sys_notify_ui_of_status(SCAN_MODULE_NAME,SCAN_STATUS_PAPER_NOPICK, NULL);  
            }
            break;
        case SCANMAN_EVENT_SCAN_CALIBRATION_RUNNING:
            {
                mydbg_prrintf("ui_sys_scan:SCANMAN_EVENT_SCAN_CALIBRATION_RUNNING\n");
             //   ui_sys_notify_ui_of_status(SCAN_MODULE_NAME,SCAN_STATUS_IDLE, NULL); 
                ui_sys_notify_ui_of_status(SCAN_MODULE_NAME,SCAN_STATUS_CALBRATING, NULL);  
            }
            break;
        case SCANMAN_EVENT_SCAN_CANCEL:
            {
                if(is_wsdscan_running || is_soapscan_running || is_aspscan_running)
                {
                    mydbg_prrintf("ui_sys_scan:SCANMAN_EVENT_SCAN_CANCEL\n");
                    if(scan_device_state != SCANCORE_EVENT_SCAN_JAM)//to remove the race conditon between SCANMAN_EVENT_SCAN_CANCEL and SCANMAN_EVENT_ADF_PAPER_JAM
                    {
                        ui_sys_notify_ui_of_status(SCAN_MODULE_NAME,SCAN_STATUS_CANCELING, NULL);  
                    }
                }
            }
            break;
        case SCANMAN_EVENT_SCAN_CALIBRATION_COMPLETE:
            {
                mydbg_prrintf("ui_sys_scan:SCANMAN_EVENT_SCAN_CALIBRATION_COMPLETE\n");
                if(is_wsdscan_running || is_soapscan_running || is_aspscan_running)
                {
                    mydbg_prrintf("ui_sys_scan:scan has not finished, send out scan running msg again\n");
                    ui_sys_notify_ui_of_status(SCAN_MODULE_NAME,SCAN_STATUS_RUNNING, NULL);  
                }
                else
                {
                    ui_sys_notify_ui_of_status(SCAN_MODULE_NAME,SCAN_STATUS_IDLE, NULL);
                }
            }
            break;
        case SCANMAN_EVENT_IDLE:
            {
                mydbg_prrintf("ui_sys_scan:SCANMAN_EVENT_IDLE \n");
                scan_device_state = SCANCORE_EVENT_IDLE;
            }
            break;
        case SCANMAN_EVENT_INITIALIZING:
            {
                mydbg_prrintf("ui_sys_scan:SCANMAN_EVENT_INITIALIZING \n");
                if(SCANCORE_EVENT_SCAN_START == scan_device_state)
                {
                    ui_sys_notify_ui_of_status(SCAN_MODULE_NAME,SCAN_STATUS_INITIALIZING, NULL);
                }
            }
            break;
        case SCANMAN_EVENT_INITIALIZED:
            {
                mydbg_prrintf("ui_sys_scan:SCANMAN_EVENT_INITIALIZED \n");
                scan_device_state = SCANCORE_EVENT_IDLE;
                ui_sys_notify_ui_of_status(SCAN_MODULE_NAME,SCAN_STATUS_IDLE, NULL);
            }
            break;
        case SCANMAN_EVENT_SCAN_COMPLETE:
            {
                mydbg_prrintf("ui_sys_scan:SCANMAN_EVENT_SCAN_COMPLETE\n");
                scan_device_state = SCANCORE_EVENT_IDLE;
            }
            break;
        case SCANMAN_EVENT_SCAN_START:
        case SCANMAN_EVENT_SCAN_RUNNING:
            {
                mydbg_prrintf("ui_sys_scan:SCANMAN_EVENT_SCAN_START or SCANMAN_EVENT_SCAN_RUNNING\n");
                scan_device_state = SCANCORE_EVENT_BUSY;
            }
            break;
        case SCANMAN_EVENT_SCAN_OFFLINE:
            {
                mydbg_prrintf("ui_sys_scan:SCANMAN_EVENT_SCAN_OFFLINE\n");
                scan_device_state = SCANCORE_EVENT_OFFLINE;

                //Send out offline(HW failure message) to UI
                scan_err_data = ui_sys_create_data_block();
                ui_sys_data_block_add_int(scan_err_data, scan_get_hwerr());
                ui_sys_notify_ui_of_status(SCAN_MODULE_NAME,SCAN_STATUS_OFFLINE, scan_err_data);
            }
            break;
        default:
            {
                // do nothing, only want to make compiler happy!
            }
        }
    }
}

// <!-- paperPresent (Read-only) -->
bool ui_sys_scan_ADF_paper_present_get(void)
{
    return  adf_sensor_paper_present();
}

// <!-- paperPresentDisabled (Read-only) -->
bool ui_sys_scan_ADF_paper_present_disabled_get(void)
{
    return  false;//is_adf_paper_present_disabled();//this function is still under discuss
}

static bool scan_process_user_feedback(const char *command)
{
    bool processed = true;
    bool jammed = false;

    if (strcmp(UI_EVENT_RESPONSE_OK, command) == 0)
    {
        mydbg_prrintf("ui_sys_scan:ok request from CP when state=%d\n",scan_device_state);

        if ((scan_device_state != SCANCORE_EVENT_SCAN_JAM)
            && (scan_device_state != SCANCORE_EVENT_SCAN_NOPICK))
        {
            if(bad_pp_flag)
            {
                ui_sys_notify_ui_of_status(SCAN_MODULE_NAME,SCAN_STATUS_IDLE, NULL);
                bad_pp_flag = false;
            }
            return processed;
        }

        if (scan_device_state == SCANCORE_EVENT_SCAN_JAM)
        {
            mydbg_prrintf("check adf sensor_path_in_path\n");
            // make sure ADF is not still jammed
            if (adf_sensor_paper_in_path())
            {
                if (adf_sensor_paper_in_path())//double check it for safety.
                {
                    jammed = true;
                }
            }
            mydbg_prrintf("soap ui_sys_scan:jamed=%d\n",jammed);

            if(! jammed)
            {
                #ifdef HAVE_WSD_SCAN
                if(is_wsdscan_running)
                {
                    wsd_clear_error_condition();//WSD workaround, fixme.
                }
                #endif
                scanmech_remove_adf_jam();
            }
        }

        if(! jammed)
        {
            scan_device_state = SCANCORE_EVENT_NULL;
            ui_sys_notify_ui_of_status(SCAN_MODULE_NAME,SCAN_STATUS_IDLE, NULL);
        }
    }
    else if (strcmp(UI_EVENT_BTN_CANCEL, command) == 0)
    {
        mydbg_prrintf("ui_sys_scan:cancel request from CP when state=%d\n",scan_device_state);
#ifdef HAVE_SOAP_SCAN
        if(is_soapscan_running)
        {
            dbg_printf("soap scan is running...,cancel it\n");
            gsoapscan_cancel_job();
        }
#endif
#ifdef HAVE_WSD_SCAN
        if (is_wsdscan_running)
        {
            dbg_printf("wsd scan is running...,cancel it\n");
            wsd_scanapp_cancel_job();
        }
#endif
    }
    return processed;
}

void ui_sys_scan_init(void)
{
    scanman_event_t last_scan_event;
    uint32_t last_scan_value;

    ui_sys_register_module_for_status(SCAN_MODULE_NAME,SCAN_STATUS_IDLE,scan_process_user_feedback);

    scanman_observer_attach(Observer_Constructor(SCAN_EVENT_ID, scanner_event_cb));

#ifdef HAVE_SOAP_SCAN
    soapscan_observer_attach(Observer_Constructor(SCAN_EVENT_ID, soapscan2pc_event_cb));
#endif

    scanman_observer_get_last_event(&last_scan_event, &last_scan_value);
    if (SCANMAN_EVENT_INITIALIZING == last_scan_event)
    {
        scan_device_state = SCANCORE_EVENT_SCAN_START;
        ui_sys_notify_ui_of_status(SCAN_MODULE_NAME, SCAN_STATUS_INITIALIZING, NULL);
    }

    if (adf_sensor_paper_present())
    {
        // Paper was already present when we initialized - send an event since 
        // the observer wasn't attached yet and missed any notifications
        last_known_paper_status = 1;
        send_paper_present_ui_event(1);
    }
}

#ifdef HAVE_WSD_SCAN
void attach_wsd_observer()
{
    wsdscan_observer_attach(Observer_Constructor(SCAN_EVENT_ID, wsdscan2pc_event_cb));
}
void dettach_wsd_observer()
{
    wsdscan_observer_detach(Observer_Constructor(SCAN_EVENT_ID, wsdscan2pc_event_cb));
}
#endif

//end
