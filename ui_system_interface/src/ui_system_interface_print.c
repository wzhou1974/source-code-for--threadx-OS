/*
 * ============================================================================
 * Copyright (c) 2013   Marvell International, Ltd. All Rights Reserved
 *
 *                         Marvell Confidential
 * ============================================================================
 */

/** 
 * \file ui_system_interface_print.c
 * 
 * \brief Implementations of print part of ui_system_interface
 *        
 * 
 **/
#include "ui_system_interface_print.h"
#include "ui_system_interface_api.h"
#include "ui_system_interface_utils.h"
#include "event_observer.h"
#include "response_subject.h"
#include "print_events_common_api.h"
#include "print_status_codes.h"
#include "agConnectMgr.h"
#include "file_to_pipe_api.h"
#include "PrintMgr.h"
#include "lassert.h"

#define DBG_PRFX "ui_sys_print: "
#define LOGGER_MODULE_MASK  (DEBUG_LOGGER_MODULE_DPRINTF | LOGGER_SUBMODULE_BIT(14))


void ui_sys_print_default_papersize_set(const mediasize_t size)
{
    error_type_t e_res;
    e_res = printvar_set_default_papersize(&size);
}


void ui_sys_print_default_papersize_get(mediasize_t *size)
{
    error_type_t e_res;

    e_res = printvar_get_default_papersize(size);
    ASSERT(OK == e_res);
}

void ui_sys_print_default_papertype_set(const mediatype_t type)
{    
    printvar_set_default_papertype(type);
}

void ui_sys_print_default_papertype_get(mediatype_t *type)
{
    error_type_t e_res;

    e_res = printvar_get_default_papertype(type);
    ASSERT(OK == e_res);
}

void ui_sys_print_density_cal_power_on_time_set(const uint32_t time_in_minutes)
{ 
#ifdef COLOR_ENG
    eng_set_density_cal_power_on_time(time_in_minutes);
#endif // #ifdef COLOR_ENG
}

void ui_sys_print_density_cal_power_on_time_get(uint32_t *time_in_minutes)
{
#ifdef COLOR_ENG
    eng_get_density_cal_power_on_time(time_in_minutes);
#endif // #ifdef COLOR_ENG
}


#define PRINT_EVENT_ID 0xD88DD88D
#define PRINTMGR_MEDIA_PROMPT_ID 0xD88DD99D


TRY_AGAIN user_input = NULL;
USER_CANCEL user_cancel = NULL;

static bool paper_out_event_occurred = false; // paper-out event has been received
static bool paper_out_user_ok_received = false; // user has pressed paper-out ok button

static bool print_process_user_feedback(const char *command)
{
    //dbg_printf("%s command: %s user_cancel: %x\n",__func__, command,user_cancel );

    if (strcmp(UI_EVENT_BTN_CANCEL, command) == 0)
    {
        if( user_cancel != NULL)
        {
            // setting paper out OK here as well since a cancel during a paper out event clears
            // the paper out status as well.
            //paper_out_user_ok_received = true;
            user_cancel(NULL);
            paper_out_event_occurred = false;
        }
        return false;
    }
    else if (strcmp(UI_EVENT_RESPONSE_OK, command) == 0)
    {
        if( user_input != NULL)
        {
            // If the paper_out_event has already occurred then
            // it is ok to send the user_input.
            // Otherwise the the paper_out_user_ok is set so
            // when the paper_out_event occurs the user_input will
            // be sent at that time.
            paper_out_user_ok_received = true;
            if (paper_out_event_occurred)
            {
                user_input(NULL);
                paper_out_event_occurred = false;
            }
        }
        
        return false;
    }
    else
    {
        return false;
    }
}


static void print_media_event_cb(Observer_t *observer, Subject_t *s)
{

    printmgr_media_prompt_info_t media_info;

    //dbg_printf("%s\n",__func__);

    ASSERT(VALIDATE_OBJECT_KEY(observer, Observer_t, 
                               subject_observer_oid_key, PRINTMGR_MEDIA_PROMPT_ID));

    printmgr_media_prompt_get_media_info(s, &media_info);

    //dbg_printf("%s tray = %d size = %d\n",__func__, media_info.prompt_input_tray, media_info.prompt_mediasize);

    user_input = printmgr_media_prompt_get_retry_callback(s);
    user_cancel = printmgr_media_prompt_get_cancel_button_callback(s);

    data_block_handle_t d_hndl;
    d_hndl = ui_sys_create_data_block();
    //ui_sys_data_block_add_int(d_hndl, media_info->prompt_input_tray);

    paper_out_event_occurred = true;

    ui_sys_data_block_add_int(d_hndl, media_info.prompt_mediasize);
    ui_sys_data_block_add_int(d_hndl, media_info.prompt_mediatype);
    ui_sys_notify_ui_of_status(PRINT_MODULE_NAME, PRINT_STATUS_PAPER_OUT, d_hndl);
}

static void handle_print_event(uint32_t pec_status, uint32_t doc_type)
{
    // clear no PJL bit if set
    pec_status &= ~STATUS_NOPJL_MASK;

    //dbg_printf("%s status: %x\n", __func__,pec_status );

    // update the print extension status as indicated
    switch (pec_status)
    {
        case STATUS_INFO_PRINTING:
        case STATUS_JOBINFO_START_JOB:
            // reset the paper out flags
            paper_out_event_occurred = false;
            paper_out_user_ok_received = false;

            // if this is a print "app" job (i.e. not a copy or fax print), update our state
            if ((e_COPYPAGE != doc_type) && (e_FAX != doc_type))
            {
                // distinguish between printing and color engine calibrating
                switch(doc_type)
                {
                case e_CALPAGE:
                    ui_sys_notify_ui_of_status(PRINT_MODULE_NAME, PRINT_STATUS_CALIBRATING, NULL);
                    break;
                    
                case e_CLEANINGPAGE:
                        ui_sys_notify_ui_of_status(PRINT_MODULE_NAME, PRINT_STATUS_CLEANING, NULL);
                        break;
                default:
                        ui_sys_notify_ui_of_status(PRINT_MODULE_NAME, PRINT_STATUS_PRINTING, NULL);
                        break;
                }
            }
            else
            {
                // this allows the copy message to be promoted to the display
                ui_sys_notify_ui_of_status(PRINT_MODULE_NAME, PRINT_STATUS_IDLE, NULL);
            }
            break;
        case STATUS_JOBINFO_END_JOB:
            // if this is a print "app" job (i.e. not a copy or fax print), update our state
            if ((e_COPYPAGE != doc_type) && (e_FAX != doc_type))
            {
                //ui_sys_notify_ui_of_status(PRINT_MODULE_NAME, PRINT_STATUS_IDLE, NULL);
            }
            break;

        case STATUS_INFO_CANCELING:
            if ((e_COPYPAGE != doc_type) && (e_FAX != doc_type))
            {
                ui_sys_notify_ui_of_status(PRINT_MODULE_NAME, PRINT_STATUS_CANCELING, NULL);
            }
            else
            {
                // to remove any print related messages from the control panel such as load paper we must
                // send the idle message here.
                ui_sys_notify_ui_of_status(PRINT_MODULE_NAME, PRINT_STATUS_IDLE, NULL);
            }
            break;

        case STATUS_INIT:
            ui_sys_notify_ui_of_status(PRINT_MODULE_NAME, PRINT_STATUS_INITIALIZING, NULL);
            break;

        case STATUS_ONLINE:
            ui_sys_notify_ui_of_status(PRINT_MODULE_NAME, PRINT_STATUS_IDLE, NULL);
            break;

        case STATUS_DOOROPEN:
        case STATUS_DOOROPEN_FRONT:
            ui_sys_notify_ui_of_status(PRINT_MODULE_NAME, PRINT_STATUS_DOOR_OPEN, NULL);
            break;
        case STATUS_DOOROPEN_FD:
            ui_sys_notify_ui_of_status(PRINT_MODULE_NAME, PRINT_STATUS_REAR_DOOR_OPEN, NULL );
            break;
        case STATUS_INPUT_LOAD_TRAY:
        case STATUS_PAPER_OUT:
        case STATUS_PAPER_OUT_TRAY1:
            // ignoring paper out status from engine at this time. All paper prompts are handled throught eh print_media_event_cb.
            // this may change if we need to display a generic paper out message after a cancel has been done by user just before a 
            // paper out event.
            break;

    case STATUS_PAPER_OUT_ALL:
            //dbg_printf("ext: STATUS_PAPER_OUT_ALL paper_out_user_ok_received = %d paper_out_event_occurred = %d\n", paper_out_user_ok_received,paper_out_event_occurred );
            if(!paper_out_user_ok_received) // ok already received
            {
                // Don't send message if we already have a paper out event in progress
                if( !paper_out_event_occurred )
                {
                     paper_out_event_occurred = true;
                
                     ui_sys_notify_ui_of_status(PRINT_MODULE_NAME, 
                                                           PRINT_STATUS_PAPER_OUT_ALL, NULL);
                }
            }
             break;
        case STATUS_INPUT_LOAD_TRAY_1_NO_MEDIA_AVAILABLE:
            // If the paper_out_user_ok has already been received (the user
            // has pressed ok button on paper out prompt screen), then
            // it is ok to send the user_input now.
            // Otherwise, the the paper_out_event is set so
            // when the paper_out_user_ok occurs the user_input will
            // be sent at that time.
            paper_out_event_occurred = true;
            if(paper_out_user_ok_received) // ok already received
            {
                user_input(NULL);
                paper_out_user_ok_received = false;
            }
            
            //ui_sys_notify_ui_of_status(PRINT_MODULE_NAME, PRINT_STATUS_PAPER_OUT, NULL);
            break;

        case STATUS_PAPER_JAM_INPUT:
            // switch this to PAPER_JAM_INPUT once we have the ability to clear the jam by pressing OK button
            //ui_sys_notify_ui_of_status(PRINT_MODULE_NAME, PRINT_STATUS_PAPER_JAM_INPUT, NULL);
            ui_sys_notify_ui_of_status(PRINT_MODULE_NAME, PRINT_STATUS_PAPER_JAM, NULL);
            break;

        case STATUS_PAPER_JAM_DOOR_OPEN:
            ui_sys_notify_ui_of_status(PRINT_MODULE_NAME, PRINT_STATUS_PAPER_JAM, NULL);
            break;

        case STATUS_PAPER_JAM_TRAY1:
        case STATUS_PAPER_JAM_TRAY2:
        case STATUS_PAPER_JAM_TRAY3:
            // switch this to PAPER_JAM_INPUT once we have the ability to clear the jam by pressing OK button
            ui_sys_notify_ui_of_status(PRINT_MODULE_NAME, PRINT_STATUS_PAPER_JAM, NULL);
            break;

        case STATUS_PAPER_JAM_OUTPUT:
            ui_sys_notify_ui_of_status(PRINT_MODULE_NAME, PRINT_STATUS_PAPER_JAM_OUTPUT, NULL);
            break;

        case STATUS_INPUT_MANUAL_DUPLEX_TRAY_1:
            paper_out_event_occurred = true;
            ui_sys_notify_ui_of_status(PRINT_MODULE_NAME, PRINT_STATUS_MANUAL_DUPLEX, NULL);
            break;

        case STATUS_INFO_COOLDOWN:
            ui_sys_notify_ui_of_status(PRINT_MODULE_NAME, PRINT_STATUS_COOLDOWN, NULL);
            break;

        case STATUS_HIDDENRESTART:
        case STATUS_HIDDENCANCEL:
            ui_sys_notify_ui_of_status(PRINT_MODULE_NAME, PRINT_STATUS_IDLE, NULL);
            break;        

        default:
            break;
    }
}

static void print_event_cb(Observer_t *observer, Subject_t *s)
{
    uint32_t cur_pec_status;
    uint32_t doc_type;
    error_type_t err;

    ASSERT(VALIDATE_OBJECT_KEY(observer, Observer_t, 
                               subject_observer_oid_key, PRINT_EVENT_ID));

    // get the current print status and doc type
    cur_pec_status = pec_get_printstatus(s);
    err = printvar_get_doctype(&doc_type);
    user_input = pec_get_retry_callback(s);
    user_cancel = pec_get_user_cancel_callback(s);


    ASSERT(OK == err);

    // update the print extension status as needed
    handle_print_event(cur_pec_status, doc_type);

}

void ui_sys_print_init(void)
{
    uint32_t cur_pec_status;
    uint32_t doc_type;
    error_type_t err;

    // register print with status support
    ui_sys_register_module_for_status(PRINT_MODULE_NAME,  
             PRINT_STATUS_IDLE, print_process_user_feedback);

    // attach observer to print status subject
    pec_attachobserver_printstatus(Observer_Constructor(PRINT_EVENT_ID, print_event_cb));
    printmgr_media_prompt_AttachObserver(Observer_Constructor(PRINTMGR_MEDIA_PROMPT_ID,print_media_event_cb));

    // update our status with latest print/engine status
    cur_pec_status = pec_get_current_printstatus();
    err = printvar_get_doctype(&doc_type);
    ASSERT(FAIL != err);
    handle_print_event(cur_pec_status, doc_type);
}




