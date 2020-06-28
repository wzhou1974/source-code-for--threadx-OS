/*
 * ============================================================================
 * Copyright (c) 2012   Marvell International, Ltd. All Rights Reserved
 *
 *                         Marvell Confidential
 * ============================================================================
 */

/** 
 * \file ui_system_interface_copy.c
 * 
 * \brief Implementations of copy part of ui_system_interface
 *        
 * 
 **/




#include "ATypes.h"
#include "lassert.h"
#include "PrintJobInfo.h"
#include "print_job_types.h"
#include "logger.h"
#include "agConnectMgr.h"
#include "event_observer.h"
#include "scancont.h"
#include "response_subject.h"
#include "JobMgr.h"
#include "print_events_common_api.h"
#include "print_status_codes.h"
#include "ResourceMap.h"
#include "sjm_observers_api.h"
#include "ui_system_interface_api.h"
#include "ui_system_interface_utils.h"
#include "ui_system_interface_copy.h"

#define COPY_EVENT_ID       0xB83E76A1

// turn on/off dev debug printf
#define cdbg_printf dbg_printf
//#define cdbg_printf

#define PRECDBG "COPY UI_SYSTEM_INTERFACE: "

#define PRINT_JOB_TYPE (job_type == ejob_PrintIO || job_type == ejob_InternalPagePrint || job_type == ejob_FaxPrint)

static uint32_t total_pages =0;
static uint32_t copy_pagenumber =0;
static uint32_t copy_setnumber =0;
static uint32_t number_of_scans = 0;
static uint32_t current_copy_job_id = 0;
static uint32_t pending_copy_job_id = 0;
static uint32_t copy_count = 0;
static bool copy_from_adf =false;
static bool collate =false;

// Walter:
// Could I assume idcopy_active variable is always false if HAVE_ID_COPY disable ?
// The current implementation could not isolate "id copy" code easily.
static bool idcopy_active = false;  

static bool continuouscopy_active = false;
static bool device_busy = false;
static bool waiting_for_user_input = false;
static bool device_canceling = false;


/*
  send updated copy page numbers to CP
*/
static void update_copy_page_number(uint32_t pages_printed)
{
    char module_name[10]={0};
    uint32_t page_count = pages_printed +1; // copying the next

    /* due to CP bug I will handle the other modules in this extension below
    if(idcopy_active)
    {
        strcpy(module_name, IDCOPY_MODULE_NAME);
    }
    else if(continuouscopy_active)
    {
        strcpy(module_name, CONTINUOUSCOPY_MODULE_NAME);
    }
    else 
    */ 
    {
        strcpy(module_name, COPY_MODULE_NAME);
    }

    if (number_of_scans == 0) number_of_scans = 1;
    cdbg_printf("%supdate_copy_page_number pgs:%d total:%d nscans:%d module:%s\n", PRECDBG, 
        pages_printed, total_pages, number_of_scans, module_name);
    if(pages_printed >= total_pages * number_of_scans)
    {
        cdbg_printf("%sCOPY-IDLE======\n", PRECDBG);
        ui_sys_notify_ui_of_status(module_name,
                             COPY_STATUS_IDLE, NULL);
        return;   // finished
    }

    if(!idcopy_active && collate)
    {
        copy_pagenumber = page_count % number_of_scans;
        copy_pagenumber = copy_pagenumber ==0 ? number_of_scans: copy_pagenumber;
        if(copy_pagenumber == 1) 
        {
            copy_setnumber++;
        }
    }
    else
    {
        copy_setnumber = page_count % total_pages;
        copy_setnumber = copy_setnumber ==0 ? total_pages: copy_setnumber;
        if(copy_setnumber == 1) 
        {
            copy_pagenumber++;
        }
        if (idcopy_active)
        {
            copy_pagenumber = 1;
        }
    }

    data_block_handle_t d_hndl;
    d_hndl = ui_sys_create_data_block();
    ui_sys_data_block_add_int(d_hndl, copy_pagenumber);
    ui_sys_data_block_add_int(d_hndl, copy_setnumber);
    ui_sys_data_block_add_int(d_hndl, total_pages);
    ui_sys_notify_ui_of_status(module_name,
                         COPY_STATUS_PAGECOUNT, d_hndl);

    cdbg_printf("%sC%d======--%d--========Copying: %d  %d/%d\n", PRECDBG,
      collate, page_count, copy_pagenumber, copy_setnumber, total_pages);
}

/*
  initialize copy job parameters
*/
void initialize_copy_vars()
{
    copy_from_adf = false;
    number_of_scans = 1000;  // init to an arbitrary large number
    copy_setnumber =0;
    copy_pagenumber =0;
}

/*
  restore the copy config nvram settings to factory default values
*/
void ui_sys_copy_settings_setdefaults()
{
    copyapp_set_copy_config_defaults();
}

/*
  save the copy config settings to nvram
*/
void ui_sys_copy_settings_set(CopyConf *copyConfig)
{
    // don't want region so init to 0
    copyConfig->region.x = 0;
    copyConfig->region.y = 0;
    copyConfig->region.width = 0;
    copyConfig->region.height = 0;

    collate = copyConfig->copyCollation;
    total_pages = copyConfig->copyNumber;

    copyapp_set_copy_config(copyConfig);
}

void start_copy(CopyConf *copyConfig)
{
    // don't want region so init to 0
    copyConfig->region.x = 0;
    copyConfig->region.y = 0;
    copyConfig->region.width = 0;
    copyConfig->region.height = 0;

    collate = copyConfig->copyCollation;
    total_pages = copyConfig->copyNumber;

    copyapp_set_copyjob_config(copyConfig);

#ifdef HAVE_ID_COPY
    if(idcopy_active)
    {
        copyapp_id_copy_start_job();
    }
    else
    {
        copyapp_start_job();
    }
#else
    copyapp_start_job();
#endif
}

/*
  execute a copy job using the copy config settings
*/
void ui_sys_copy_startjob(CopyConf *copyConfig)
{
    cdbg_printf("%s copy job button pressed\n", PRECDBG);
    cdbg_printf("        device_busy:%d copy_count:%d\n", device_busy, copy_count);

    if(!is_scanner_alive()) 
    {
        cdbg_printf("%s Device is not ready.\n", PRECDBG);
        ui_sys_notify_ui_of_status(SYSTEM_MODULE_NAME, 
                                   COPY_STATUS_BUSY, NULL);  
        return;
    }
    if(copy_count > 1)
    {
        cdbg_printf("%s Device is busy.\n", PRECDBG);
        ui_sys_notify_ui_of_status(SYSTEM_MODULE_NAME, 
                                   COPY_STATUS_BUSY, NULL);
        return;
    }
    else 
    {
        copy_count++;
        cdbg_printf("%s copy pending devbusy:%d jobid:%d cc:%d\n", PRECDBG,
                    device_busy, current_copy_job_id, copy_count);
        if(device_busy || current_copy_job_id != 0)
        {
            cdbg_printf("%s copy pending...\n", PRECDBG);
            ui_sys_notify_ui_of_status(SYSTEM_MODULE_NAME, 
                                    COPY_STATUS_PENDING, NULL);                
        }
    }

    

    start_copy(copyConfig);
}

#ifdef  HAVE_ID_COPY
/*
  execute a copyID job using the copy config settings
*/
void ui_sys_copy_copyid_jobstart(CopyConf *copyConfig)
{
    cdbg_printf("%s ID copy job button pressed\n", PRECDBG);

    if(device_busy || copy_count > 0)
    {
        ui_sys_notify_ui_of_status(SYSTEM_MODULE_NAME, 
                                           COPY_STATUS_BUSY, NULL);                
        return;
    }

    idcopy_active = true;
    start_copy(copyConfig);
}

/*
  Issue "Next" to copy ID job 
*/
void ui_sys_copy_copyid_jobnext()
{
    cdbg_printf("%s ID copy Next button pressed\n", PRECDBG);
    copyapp_id_copy_input(e_COPY_NEXT);
}

/*
  Issue "Finish" to copy ID job
*/
void ui_sys_copy_copyid_jobfinish()
{
    cdbg_printf("%s ID copy Finish button pressed\n", PRECDBG);
    copyapp_id_copy_input(e_COPY_FINISH);
}

/*
  Issue "Cancel" to copy ID job
*/
void ui_sys_copy_copyid_jobcancel()
{
    cdbg_printf("%s ID copy Cancel button pressed\n", PRECDBG);
    copyapp_id_copy_input(e_COPY_CANCEL);
}

#endif

/*
  get (Retrieve the current copy config values stored in nvram)
    returns:  CopyConfig object populated with the contents of that entry.
*/
void ui_sys_copy_settings_get(CopyConf *copyConfig)
{
    copyapp_get_copy_config(copyConfig);
}


/*
  set copy parameters in non-volatile memory
*/
void ui_sys_copy_settings_copy_quality_set(e_COPYQUALITY quality)
{
    copyapp_set_fp_qualitymode(quality);
}
void ui_sys_copy_settings_colormode_set(copy_cmode_t ccmode)
{
    copyapp_set_fp_colormode(ccmode);
}
void ui_sys_copy_settings_resize_set(e_COPYRESIZE resize_var, uint32_t custom_resize)
{
    copyapp_set_fp_resize(resize_var, custom_resize);
}
void ui_sys_copy_settings_numcopies_set(int num_copies)
{
    copyapp_set_fp_num_copies(num_copies);
}
void ui_sys_copy_settings_brightness_set(int brightness)
{
    copyapp_set_fp_brightness(brightness);
}
void ui_sys_copy_settings_print_quality_set(PRINT_QUALITY print_quality)
{
  copyapp_set_fp_print_quality(print_quality);
}
void ui_sys_copy_settings_inputtray_set(trayenum_t input_tray)
{
  copyapp_set_fp_input_tray(input_tray);
}
void ui_sys_copy_settings_collation_set(BOOL enable)
{
  copyapp_set_fp_collation(enable);
}
void ui_sys_copy_settings_print_duplex_set(BOOL enable)
{
  copyapp_set_fp_print_duplex(enable);
}
void ui_sys_copy_settings_scan_duplex_set(BOOL enable)
{
  copyapp_set_fp_scan_duplex(enable);
}
void ui_sys_copy_settings_color_tint_set(int r, int g, int b)
{
   copyapp_set_fp_color_balance(r, g, b);
}
void ui_sys_copy_settings_background_set(int background)
{
  copyapp_set_fp_background(background);
}
void ui_sys_copy_settings_sharpness_set(int sharpness)
{
  copyapp_set_fp_sharpness(sharpness);
}
void ui_sys_copy_settings_contrast_set(int contrast)
{
  copyapp_set_fp_contrast(contrast);
}
void ui_sys_copy_settings_papersize_set(mediasize_t paper_size)
{
    copyapp_set_fp_paper_size(paper_size);
}

void ui_sys_copy_start_continuous_scan()
{
    scan_continuous_start(0);
    ui_sys_notify_ui_of_status(CONTINUOUSSCAN_MODULE_NAME,
                          COPY_SCAN_STATUS_RUNNING, NULL);                
}
void ui_sys_copy_stopcontinuousscan()
{
    scan_continuous_stop();
}

void ui_sys_copy_start_continuous_copy()
{
    cdbg_printf("%s start continuous copy...\n", PRECDBG);
    continuouscopy_active = true;
    total_pages = 1;
    copy_continuous_start();
    //ui_sys_notify_ui_of_status(CONTINUOUSCOPY_MODULE_NAME, 
    ui_sys_notify_ui_of_status(COPY_MODULE_NAME, 
                                           COPY_STATUS_PAGECOUNT, NULL);                
}
void ui_sys_copy_stop_continuous_copy()
{
    cdbg_printf("%s stop continuous copy...\n", PRECDBG);
    continuouscopy_active = false;
    copy_continuous_stop();
}


/*
  status and event callbacks
*/
static void print_devicestatus_cb(Observer_t *o, Subject_t *s)
{
    // Sanity check of the observer
    ASSERT( VALIDATE_OBSERVER( o, COPY_EVENT_ID) );

    if (s != NULL)
    {
        uint32_t status;
        status = pec_get_printstatus(s) & ~STATUS_NOPJL_MASK; // clear no PJL bit if set

        if(status == STATUS_JOBINFO_START_JOB)
        {
            cdbg_printf("%s print_devicestatus_cb  START_JOB\n", PRECDBG);
        }

        else if(status == STATUS_JOBINFO_END_JOB ||
               status == STATUS_JOBINFO_CANCEL ||
               status == STATUS_JOBINFO_JOB_ABORTED)
        {
            cdbg_printf("%s print_devicestatus_cb  END_JOB\n", PRECDBG);
        }
    }
}

static void sjm_event_cb(Observer_t *o, Subject_t *s)
{
    ASSERT( VALIDATE_OBSERVER( o, COPY_EVENT_ID) );

    if (s != NULL)
    {
        JOB_TYPES job_type = SJM_Get_JobType(s);
        sjm_jobstate_enum_t job_status = SJM_Get_JobStatus(s);
        uint32_t job_id = SJM_Get_JobId(s);

        cdbg_printf(
            "\n%sCOPY JOB UPDATE, job type = %d job = %d status = %d\n", PRECDBG,
             job_type, job_id, job_status);

        if(job_status == esjm_JobStart)
        {
            if(job_type == ejob_Copy)
            {
                device_canceling = false;

                if(current_copy_job_id == 0)
                {
                    cdbg_printf("%sCopy Job Start==-=-=\n", PRECDBG);
                    current_copy_job_id = job_id;
                    initialize_copy_vars();
                    update_copy_page_number(0); //reset page_count counters
                }
                else
                {
                    pending_copy_job_id = job_id;
                }
            }
            else if(PRINT_JOB_TYPE)
            {
                cdbg_printf("%s NonCopy job started -- set device busy\n", PRECDBG);
                device_busy = true;
            }
        }
        else if(job_status == esjm_JobDone)
        {
            if(job_type == ejob_Copy) 
            {
                if(copy_count > 0) copy_count--;
                current_copy_job_id = 0;

                if(pending_copy_job_id != 0)
                {
                    cdbg_printf("%sCopy Job Done==-=-=\n", PRECDBG);
                    // there's a pending job so reinitialize vars
                    current_copy_job_id = pending_copy_job_id;
                    pending_copy_job_id = 0;
                    initialize_copy_vars();
                    update_copy_page_number(0); //reset page_count counters
                }
                else
                {
                    cdbg_printf("%sCopy Jobs Done==-=-=\n", PRECDBG);
                    // there's no pending job so goto idle
                    ui_sys_notify_ui_of_status(COPY_MODULE_NAME,
                                             COPY_STATUS_IDLE, NULL);
                    // reset idcopy flag just in case
                    idcopy_active = false;
                }
            }
            else
            {
                cdbg_printf("%s NonCopy job finished -- clear device busy\n", PRECDBG);
                device_busy = false;
            }
        }
        else if(job_status == esjm_JobCancel_at_device ||
                job_status == esjm_JobCancel_by_user)
        {
            if(job_type == ejob_Copy)
            {
                device_canceling = true;
            }
        }
    }
}

static void print_event_cb(Observer_t *o, Subject_t *s)
{
    // Sanity check of the observer
    ASSERT( VALIDATE_OBSERVER( o, COPY_EVENT_ID) );

    if (s != NULL)
    {
        JOB_DATA *job_data = 
             print_jobmgr_global_page_end_observer_get_job_data(s);

        if(job_data->DocType == e_COPYPAGE)
        {
            uint32_t pages_printed = job_data->pjob_info->pages_printed;
            cdbg_printf("%s print event - pages printed=%d\n", 
                          PRECDBG, pages_printed);
            if(! device_canceling)
            {
                update_copy_page_number(pages_printed);
            }
        }
    }
}

static void copy_event_cb(Observer_t *o, Subject_t *s)
{
     // Sanity check of the observer
    ASSERT( VALIDATE_OBSERVER( o, COPY_EVENT_ID) );
    char module_name[10]={0};

    /*
    if(idcopy_active)
    {
        strcpy(module_name, IDCOPY_MODULE_NAME);
    }
    else
    */
    {
        strcpy(module_name, COPY_MODULE_NAME);
    }

    if (s != NULL)
    {
        copyapp_observer_data_t *subject = 
                  (copyapp_observer_data_t *)s;

        if(subject->event == COPYAPP_EVENT_COPY_FROM_ADF)
        {        
            cdbg_printf("%s copy_event_cb  ADF-COPY\n", PRECDBG);
            copy_from_adf = true;
        }
        else if(subject->event == COPYAPP_EVENT_COPY_PAGE_START)
        {
            cdbg_printf("%s copy_event_cb  COPY-PAGE-START\n", PRECDBG);
            number_of_scans = subject->number_of_scans;
        }
        else if(subject->event == COPYAPP_EVENT_COPY_COMPLETE) 
        {
            cdbg_printf("%s copy_event_cb  COPY-COMPLETE\n", PRECDBG);
            number_of_scans = subject->number_of_scans;
        }
        else if(subject->event == COPYAPP_EVENT_CANCEL)
        {
            cdbg_printf("%s copy_event_cb  COPY-CANCEL\n", PRECDBG);
            idcopy_active = false;
            ui_sys_notify_ui_of_status(module_name,
                                     COPY_STATUS_CANCELING, NULL);
        }
        else if(subject->event == COPYAPP_EVENT_IDLE)
        {
            cdbg_printf("%s copy_event_cb  COPY IDLE\n", PRECDBG);
            ui_sys_notify_ui_of_status(module_name,
                                         COPY_STATUS_IDLE, NULL);
        }
        else if(subject->event == COPYAPP_EVENT_COPY_RUNNING)
        {
            cdbg_printf("%s copy_event_cb  COPY RUNNING\n", PRECDBG);
        }
        else if(subject->event == COPYAPP_EVENT_WAITING_FOR_USER_INPUT )
        {
            cdbg_printf("%s copy_event_cb  USER INPUT\n", PRECDBG);
            //ui_sys_notify_ui_of_status(module_name,
                 //COPY_STATUS_USER_INPUT, NULL);

            waiting_for_user_input = true;

            data_block_handle_t d_hndl;
            d_hndl = ui_sys_create_data_block();
            // indicate next copy to place on glass thus copy_pagenumber +1
            ui_sys_data_block_add_int(d_hndl, copy_pagenumber +1);
            ui_sys_notify_ui_of_status(module_name,
                 COPY_STATUS_USER_INPUT, d_hndl);
        }
        else
        {
            cdbg_printf("%s copy_event_cb  ev=%d\n", PRECDBG, subject->event);
        }
    }
}

#ifdef HAVE_ID_COPY
static bool idcopy_process_user_feedback(const char *command)
{
    bool processed = true;

    if (strcmp(UI_EVENT_BTN_CANCEL, command) == 0)
    {
        cdbg_printf("%s idcopy_process_user_feedback CANCEL BTN\n", PRECDBG);
        idcopy_active = false;
        copyapp_id_copy_input(e_COPY_CANCEL);
    }
    else if (strcmp(UI_EVENT_RESPONSE_OK, command) == 0)
    {
        cdbg_printf("%s idcopy_process_user_feedback OK BTN\n", PRECDBG);
        // reset the counters
        //copy_pagenumber =0;
        if(waiting_for_user_input)
        {
            update_copy_page_number(0);
            copyapp_id_copy_input(e_COPY_NEXT);
        }
    }
    else if (strcmp(UI_EVENT_RESPONSE_DONE, command) == 0)
    {
        cdbg_printf("%s idcopy_process_user_feedback DONE BTN\n", PRECDBG);
        if(waiting_for_user_input)
        {
            update_copy_page_number(0);
            copyapp_id_copy_input(e_COPY_FINISH);
        }
    }
    else
    {
        cdbg_printf("%s idcopy_process_user_feedback ?? %s\n", PRECDBG, command);
        processed = false;
    }

    waiting_for_user_input = false;
    return processed;
}
#endif

static bool continuouscopy_process_user_feedback(const char *command)
{
    bool processed = true;

    if (strcmp(UI_EVENT_BTN_CANCEL, command) == 0)
    {
        cdbg_printf("%s continuouscopy_process_user_feedback CANCEL BTN\n", PRECDBG);
        continuouscopy_active = false;
        copy_continuous_stop(); // stop continuous copies
        copyapp_copy_input(e_COPY_CANCEL); // cancel current copy
    }
    else
    {
        processed = false;
    }

    return processed;
}

static bool copy_process_user_feedback(const char *command)
{
    bool processed = true;

#ifdef HAVE_ID_COPY
    // a workaround for CP bug
    if(idcopy_active)
    {
        return idcopy_process_user_feedback(command);
    }
#endif
    if(continuouscopy_active)
    {
        return continuouscopy_process_user_feedback(command);
    }

    if (strcmp(UI_EVENT_BTN_CANCEL, command) == 0)
    {
        cdbg_printf("%s copy_process_user_feedback CANCEL BTN\n", PRECDBG);
        copyapp_copy_input(e_COPY_CANCEL);
    }
    else if (strcmp(UI_EVENT_RESPONSE_OK, command) == 0)
    {
        // this occurs during an n-up copy.  
        cdbg_printf("%s copy_process_user_feedback OK BTN\n", PRECDBG);
        // reset the counters
        //copy_pagenumber =0;
        if(waiting_for_user_input)
        {
            collate = false;//because N-up copy via flatbed has only one output page, though this page may need to be copied multiply times
            copy_pagenumber--;//update copy_pagenumber increments pagenumber
            update_copy_page_number(0);
            copyapp_copy_input(e_COPY_NEXT);
        }
    }
    else if (strcmp(UI_EVENT_RESPONSE_DONE, command) == 0)
    {
        cdbg_printf("%s copy_process_user_feedback DONE BTN\n", PRECDBG);
        if(waiting_for_user_input)
        {
            collate = false;//because N-up copy via flatbed has only one output page, though this page may need to be copied multiply times
            copy_pagenumber--;//update copy_pagenumber increments pagenumber
            update_copy_page_number(0);
            copyapp_copy_input(e_COPY_FINISH);
        }
    }
    else
    {
        cdbg_printf("%s idcopy_process_user_feedback ?? %s\n", PRECDBG, command);
        processed = false;
    }

    waiting_for_user_input = false;
    return processed;
}

static bool continuousscan_process_user_feedback(const char *command)
{
    bool processed = true;

    if (strcmp(UI_EVENT_BTN_CANCEL, command) == 0)
    {
        scan_continuous_stop();
    }
    else
    {
        processed = false;
    }

    return processed;
}


void ui_sys_copy_init()
{
    // before attaching any observers, register needed modules for extension utils status support
    ui_sys_register_module_for_status(COPY_MODULE_NAME,  
                         COPY_STATUS_IDLE, 
                         copy_process_user_feedback);

#ifdef HAVE_ID_COPY
    ui_sys_register_module_for_status(IDCOPY_MODULE_NAME,  
                         COPY_STATUS_IDLE, 
                         idcopy_process_user_feedback);
#endif

    ui_sys_register_module_for_status(CONTINUOUSCOPY_MODULE_NAME,  
                        COPY_STATUS_IDLE,
                        continuouscopy_process_user_feedback);
    ui_sys_register_module_for_status(CONTINUOUSSCAN_MODULE_NAME,  
                        COPY_STATUS_IDLE,
                        continuousscan_process_user_feedback);
    
    SJM_AttachObserver_JobStatus(
          Observer_Constructor(COPY_EVENT_ID, sjm_event_cb));

    copyapp_observer_attach(
          Observer_Constructor(COPY_EVENT_ID, copy_event_cb));

    print_jobmgr_global_page_end_observer_attach( 
          Observer_Constructor(COPY_EVENT_ID, print_event_cb));

    pec_attachobserver_printstatus(
          Observer_Constructor(COPY_EVENT_ID, print_devicestatus_cb));

}

