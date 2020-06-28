/*
 * ============================================================================
 * Copyright (c) 2011-2013   Marvell International, Ltd. All Rights Reserved
 *
 *                         Marvell Confidential
 * ============================================================================
 */

/** 
 * \file ui_system_interface_utils.c
 * 
 * \brief Implementations of shared routines for UI System Interface. 
 * 
 **/

#include "lassert.h"
#include "logger.h"
#include "memAPI.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "ui_system_interface_utils.h"
#include "ui_system_interface_api.h"
#include "ui_system_interface_status.h"
#include "delay_api.h"
#include "tx_api.h"

#if defined( HAVE_CMD )
#include "cmd_proc_api.h"
#endif // defined( HAVE_CMD )

int ui_sys_debug_level = 0;

/******************** Prototypes for local Routines *********************/

static void ui_sys_utils_init();
static void event_notify_init();
#if defined( HAVE_CMD )
static void register_ui_sys_util_debug_commands();
#endif // defined( HAVE_CMD )

/************ Definitions for Data Blocks that accompany status updates and events *********************/

#define URL_PARAM_COUNT_TEXT "&pcnt=%d&"

typedef enum
{
    e_DATA_TYPE_INT,
    e_DATA_TYPE_STRING,
} data_field_type_t;

#define NUM_DATA_FIELD_TYPES 2

char *data_field_type_as_string[] =
{
    "int",
    "str",
};

/******************** Shared Data *********************/

static TX_MUTEX ui_sys_status_mutex;

/******************** output callback notification support *********************/

#define MAX_NUM_CALLBACKS 5
static ui_sys_output_callback_t saved_output_callbacks[MAX_NUM_CALLBACKS];

static void init_output_callbacks()
{
    int index;
    for (index=0; index < MAX_NUM_CALLBACKS; index++)
    {
        saved_output_callbacks[index] = NULL;
    }
}

void ui_sys_register_output_callback(ui_sys_output_callback_t callback) {
    int index;
    for (index = 0; index < MAX_NUM_CALLBACKS; index++)
    {
        if (NULL == saved_output_callbacks[index])
        {
            saved_output_callbacks[index] = callback;
            break;
        }
    }
}

static void notify_registered_output_callbacks(char *message) {
    int index;
    for (index=0; index < MAX_NUM_CALLBACKS; index++)
    {
        if (NULL != saved_output_callbacks[index])
        {
            saved_output_callbacks[index](message);
        }
    }

    // free the original message after all registrered callbacks notified
    MEM_FREE_AND_NULL(message);
}


/******************** Initialization *********************/

#include "posix_ostools.h"
#include "sys_init_api.h"
static pthread_t ui_sys_thread_info;
static void *ui_sys_init_level1(void* unused);

void ui_sys_init()
{
    int posix_res;

    // initialize the ui_system_interface utils
    ui_sys_utils_init();

    // initialize the ui_system_interface event queue
    event_notify_init();

    // Start temporary ui_system_interface level-1 init thread.  Note that this thread
    // is a posix detached thread so the thread and all its resources will be destroyed
    // when its thread routine exits.
    SysRegisterInitLevel(INITLEVEL_1);
    posix_res = posix_create_thread(&ui_sys_thread_info,
                                    ui_sys_init_level1,
                                    NULL,
                                    "ui_sys level-1 init",
                                    NULL,
                                    0,
                                    POSIX_THR_PRI_NORMAL);
    ASSERT( 0 == posix_res );
    posix_res = pthread_detach(ui_sys_thread_info);
    ASSERT( 0 == posix_res );
}


static void *ui_sys_init_level1(void *unused)
{
    // wait for level 0 to finish
    SysWaitForInitLevel(INITLEVEL_1);

    // intialize the defined module-specific interfaces
    #ifdef HAVE_PLATFORM
    ui_sys_platform_init();
    #endif

    #ifdef HAVE_COMMON_CONSUMABLES_API
    ui_sys_consumables_init();    
    #endif

    #ifdef HAVE_COPY_SUPPORT
    ui_sys_copy_init();
    #endif // #ifdef HAVE_COPY_SUPPORT

    #ifdef HAVE_PRINT_SUPPORT
    ui_sys_print_init();
    #endif

    #ifdef HAVE_SCAN_SUPPORT
    ui_sys_scan_init();
    #endif // #ifdef HAVE_SCAN_SUPPORT

    #ifdef HAVE_FAX_SUPPORT
//    ui_sys_fax_init();
    #endif // #ifdef HAVE_FAX_SUPPORT

    #ifdef HAVE_NETWORK
    ui_sys_network_init();
    #endif

    #ifdef HAVE_WIRELESS
    ui_sys_wireless_init();
    #endif // #ifdef HAVE_WIRELESS

    #ifdef HAVE_ETHERNET
    ui_sys_wired_network_init();    
    #endif

    // init the test harness if we have one
    #ifdef HAVE_UI_SYSTEM_INTERFACE_TEST
    ui_sys_test_init();
    #endif // #ifdef HAVE_UI_SYSTEM_INTERFACE_TEST

    SysInitDone(INITLEVEL_1);

    return 0;
}

static void ui_sys_utils_init()
{
    uint32_t tx_res;

    // create our mutex
    tx_res = tx_mutex_create(&ui_sys_status_mutex, "ui_sys_status_mutex", TX_INHERIT);
    ASSERT(TX_SUCCESS == tx_res);

    // init our output callbacks array
    init_output_callbacks();

    #if defined( HAVE_CMD )
    // register our debug commands
    register_ui_sys_util_debug_commands();
    #endif // defined( HAVE_CMD )
}

/******************** Status Management *********************/

static void ui_sys_post_module_status(bool scope_overall, const char *module_name, const char *module_state, const char *status_type, data_block_handle_t status_data)
{
    char *msg_command = NULL;
    uint32_t msg_len = (module_state == NULL) ? 100 : 100 + strlen(module_state);

    if (status_data != NULL)
    {
        msg_len += status_data->cur_buffer_size;
        msg_len += strlen(URL_PARAM_COUNT_TEXT);     // We will attach a param count in addition to the params
    }

    msg_command = MEM_CALLOC_ALIGN(1, msg_len, e_8_byte);
    ASSERT(msg_command != NULL);
    strcat((char *) msg_command, "scope=");
    if (scope_overall)
    {
        strcat((char *) msg_command, "overall"); // new overall worst status
    }
    else
    {
        strcat((char *) msg_command, "module"); // new status for specified module
    }
    strcat((char *) msg_command, "&module=");
    strcat((char *) msg_command, module_name);
    strcat((char *) msg_command, "&state=");
    strcat((char *) msg_command, module_state);
    strcat((char *) msg_command, "&type=");
    strcat((char *) msg_command, status_type);

    if (status_data != NULL)
    {
        ASSERT(status_data->data_buffer != NULL);
        sprintf((char *) msg_command + strlen((char *)msg_command), URL_PARAM_COUNT_TEXT, (int)status_data->num_fields);
        strcat((char *) msg_command, status_data->data_buffer);
    }

    DPRINTF((DBG_OUTPUT|DBG_LOUD), ("Status update: %s\n", msg_command));
    notify_registered_output_callbacks(msg_command);
}

typedef struct module_status_struct
{
    const char          *module_name;
    StatMsg             *module_state_entry;
    delay_msec_timer_t  *module_status_timer;
    data_block_handle_t  status_data;
    ui_sys_input_callback_t module_input_callback;
} module_status;

#define MAX_NUM_MODULES 20
static uint32_t worst_index; // module with the current "worst" status
static module_status module_statuses[MAX_NUM_MODULES] = 
{
    {NULL, NULL, NULL, NULL, NULL},
    {NULL, NULL, NULL, NULL, NULL},
    {NULL, NULL, NULL, NULL, NULL},
    {NULL, NULL, NULL, NULL, NULL},
    {NULL, NULL, NULL, NULL, NULL},
    {NULL, NULL, NULL, NULL, NULL},
    {NULL, NULL, NULL, NULL, NULL},
    {NULL, NULL, NULL, NULL, NULL},
    {NULL, NULL, NULL, NULL, NULL},
    {NULL, NULL, NULL, NULL, NULL},
    {NULL, NULL, NULL, NULL, NULL},
    {NULL, NULL, NULL, NULL, NULL},
    {NULL, NULL, NULL, NULL, NULL},
    {NULL, NULL, NULL, NULL, NULL},
    {NULL, NULL, NULL, NULL, NULL},
    {NULL, NULL, NULL, NULL, NULL},
    {NULL, NULL, NULL, NULL, NULL},
    {NULL, NULL, NULL, NULL, NULL},
    {NULL, NULL, NULL, NULL, NULL},
    {NULL, NULL, NULL, NULL, NULL},    
};

// rebroadcast the current "worst" status to the UI
void rebroadcast_current_ui_sys_status()
{
    ui_sys_post_module_status(true, 
                              module_statuses[worst_index].module_name,
                              (module_statuses[worst_index].module_state_entry)->state,
                              status_type_to_string((module_statuses[worst_index].module_state_entry)->msgType),
                              module_statuses[worst_index].status_data);
}

static void ui_sys_status_timer_expired(void *module_index)
{
    char *module_name = (char *) module_statuses[(uint32_t)module_index].module_name;

    // done with previous status, so set the desired module back to "online"
    ui_sys_notify_ui_of_status(module_name, findOnlineStatusForModule(module_name), NULL);
}

// update the stored state information for the specified module
static void update_module_status(const char *name, const char *state, data_block_handle_t data)
{
    data_block_handle_t old_data;
    uint32_t tx_res;
    StatMsg *pStatMsg;
    int i;

    // acquire the ui system interface status mutex
    tx_res = tx_mutex_get(&ui_sys_status_mutex, TX_WAIT_FOREVER);
    ASSERT(TX_SUCCESS == tx_res);

    for (i=0; i<MAX_NUM_MODULES; i++)
    {
        if ((module_statuses[i].module_name != NULL) && (!strcmp(name, module_statuses[i].module_name)))
        {
            // have seen this module before, so use this slot
            break;
        }
    }
    ASSERT(MAX_NUM_MODULES > i);

    // lookup desired action for module's status
    DPRINTF((DBG_LOUD | DBG_OUTPUT), ("%s@%d: Received status with module=%s and state=%s\n", __FILE__, __LINE__, name, state));
    pStatMsg = find_module_state_entry(name, state);

    // update status info for this module if requested module/state is in the table
    if (pStatMsg)
    {
        // update the status entry and data block for this module
        module_statuses[i].module_state_entry = pStatMsg;
        old_data = module_statuses[i].status_data;
        module_statuses[i].status_data = data;
        ui_sys_dispose_data_block(old_data);

        // cancel any running status timer for desired module
        if (module_statuses[i].module_status_timer != NULL)
        {
            module_statuses[i].module_status_timer = delay_msec_timer_cancel(module_statuses[i].module_status_timer);
            module_statuses[i].module_status_timer = NULL;
        }

        // start timer if new state should timeout
        if (MSG_TRANSITORY == module_statuses[i].module_state_entry->msgType)
        {
            module_statuses[i].module_status_timer = delay_msec_timer_non_blocking(1000 * 3, ui_sys_status_timer_expired, (void*)i, false);
        }

        // notify the UI of this module status change
        ui_sys_post_module_status(false,
                                  name,
                                  state,
                                  status_type_to_string(pStatMsg->msgType),
                                  data);
    }
    else
    {
        // dispose any new data block as we're ignoring this unknown status
        ui_sys_dispose_data_block(data);
    }

    // release the ui system interface status mutex
    tx_res = tx_mutex_put(&ui_sys_status_mutex);
    ASSERT(TX_SUCCESS == tx_res);
}

// return index of the module that currently has the "worst" status
static uint32_t get_worst_module_index(void)
{
    int i;
    uint32_t previous_worst = UI_SYS_STATSEV_NONE | UI_SYS_STATPRI_0;
    uint32_t current_worst;
    uint32_t worst_index = 0;
    uint32_t tx_res;

    // acquire the ui system interface status mutex
    tx_res = tx_mutex_get(&ui_sys_status_mutex, TX_WAIT_FOREVER);
    ASSERT(TX_SUCCESS == tx_res);

    // loop through all active modules & find the current worst status
    for (i=0; i<MAX_NUM_MODULES; i++)
    {
        if (NULL == module_statuses[i].module_name)
        {
            // no more active modules, so we can quit now
            break;
        }
        else if (!module_statuses[i].module_state_entry)
        {
            // current module doesn't have a supported status table entry
            //ASSERT(0); // 20130403: For now commenting out this assert as we are starting
                         // to bump into this condition when the scan module has been
                         // added as a known module but has not had time to update its initial
                         // state before the consumables module tries to report a low
                         // toner cartridge.
            continue;
        }
        else
        {
            // update our index if this is the worst we've seen so far
            current_worst = STATGETWORSE(previous_worst, (module_statuses[i].module_state_entry)->StatMgrCode);
            if (current_worst != previous_worst)
            {
                previous_worst = current_worst;                
                worst_index = i;
            }
        }
    }

    // release the ui system interface status mutex
    tx_res = tx_mutex_put(&ui_sys_status_mutex);
    ASSERT(TX_SUCCESS == tx_res);

    return worst_index;
}


// function to alert the UI of desired changes in response to a module state change notification
void ui_sys_notify_ui_of_status(const char *module, const char *new_state, data_block_handle_t state_data)
{
    static uint32_t prev_worst_index = 0;
    static StatMsg *prev_worst_entry = NULL;
    StatMsg *worst_entry;
    const char *status_type = NULL;

    // update this module's status
    update_module_status(module, new_state, state_data);

    // determine the worst status right now
    worst_index = get_worst_module_index();
    worst_entry = module_statuses[worst_index].module_state_entry;

    // bail if the event we received didn't give us a new "worst"
    if ((worst_entry == prev_worst_entry) && 
        ((strcmp(worst_entry->module, module) != 0) || (state_data == NULL))
       )
    {
        return;
    }
    else
    {
        prev_worst_index = worst_index;
        prev_worst_entry = worst_entry;
    }

    // if at least one module isn't idle, lookup its status type
    if ((UI_SYS_STATSEV_NONE|UI_SYS_STATPRI_0) != worst_entry->StatMgrCode)
        
    {
        status_type = status_type_to_string(worst_entry->msgType);
    }
    else
    {
        status_type = status_type_to_string(MSG_READY);
    }

    // push out this change of the "worst" status
    ui_sys_post_module_status(true,
                              module_statuses[worst_index].module_name,
                              (module_statuses[worst_index].module_state_entry)->state,
                              status_type,
                              module_statuses[worst_index].status_data);
}

void ui_sys_register_module_for_status(const char *module, const char *initial_state, ui_sys_input_callback_t input_callback)
{
    uint32_t tx_res;
    int i;

    // acquire the ui system interface status mutex
    tx_res = tx_mutex_get(&ui_sys_status_mutex, TX_WAIT_FOREVER);
    ASSERT(TX_SUCCESS == tx_res);

    for (i = 0; i < MAX_NUM_MODULES; i++)
    {
        if (module_statuses[i].module_name == NULL)
        {
            module_statuses[i].module_name = module;
            module_statuses[i].module_input_callback = input_callback;
            break;
        }
    }
    ASSERT(i < MAX_NUM_MODULES);

    // release the ui system interface status mutex
    tx_res = tx_mutex_put(&ui_sys_status_mutex);
    ASSERT(TX_SUCCESS == tx_res);

    ui_sys_notify_ui_of_status(module, initial_state, NULL);
}

static void param_list_locate_value(const char *param_list, const char *name_to_find, char **value_ptr, int *length)
{
    int name_len; 

    ASSERT(param_list != NULL);
    ASSERT(name_to_find != NULL);
    ASSERT(value_ptr != NULL);
    ASSERT(length != NULL);

    name_len = strlen(name_to_find);
    *length = 0;

    *value_ptr = strstr(param_list, name_to_find);
    while ((*value_ptr != NULL) && (*(*value_ptr + name_len) != '='))
    {
        *value_ptr = strstr(*value_ptr + name_len, name_to_find);
    }
    
    if (*value_ptr != NULL)
    {
        *value_ptr += name_len + 1;

        while(((*value_ptr)[*length] != '\0') && ((*value_ptr)[*length] != '&'))
        {
            (*length)++;
        }
    }
}

/******************** Listener for all input broadcasted by UI *********************/

void ui_sys_user_feedback_to_module(const char* event)
{
    /*
        The event has one of the following formats:

        1. The event is forwarded to the system overall, not the specific module.
        The module that has the most significant status will handle it.

        status_response?&action=ok | cancel | done

        2. The event is forwarded to the specific module (for example, copy module).

        status_response?module=copy&action=ok | cancel | done
    */

	#define EVENT_PREFIX "status_response?"

    size_t event_prefix_len = strlen(EVENT_PREFIX);

    if (strncmp(event, EVENT_PREFIX, event_prefix_len) != 0)
    {
        // invalid event format
        DPRINTF((DBG_OUTPUT|DBG_LOUD), ( "UI listener received invalid event: %s\n", event ));
        return;
    }

    char *event_ptr = NULL;
    int   event_len = 0;
    param_list_locate_value(event, "action", &event_ptr, &event_len);

    // make sure the event format is valid
    ASSERT(event_ptr != NULL && event_len != 0);
    ASSERT((strncmp(event_ptr, UI_EVENT_BTN_CANCEL,    event_len) == 0 && event_len == strlen(UI_EVENT_BTN_CANCEL))  || 
           (strncmp(event_ptr, UI_EVENT_RESPONSE_OK,   event_len) == 0 && event_len == strlen(UI_EVENT_RESPONSE_OK)) || 
           (strncmp(event_ptr, UI_EVENT_RESPONSE_DONE, event_len) == 0 && event_len == strlen(UI_EVENT_RESPONSE_DONE)));

    // capture desired action in stack string
    char action[7];  // 7 == strlen("cancel") + 1 > strlen("done") + 1 > strlen("ok") + 1 
    bzero(action, 7);
    strncpy(action, event_ptr, event_len);

    DPRINTF((DBG_OUTPUT|DBG_LOUD), ( "UI listener received event: %s\n", event ));

    // Handle simple event (no module specified) and detailed event (module specificed)
    // separately.
    if (strncmp(event + event_prefix_len, "module", 6 /* strlen("module") */) != 0)
    {
        int worst_index;

        // Simple event - no module specified.
        // Determine who has most significant status right now and send the event to them.
        worst_index = get_worst_module_index();
        if (module_statuses[worst_index].module_input_callback != NULL)
        {			
            module_statuses[worst_index].module_input_callback(action);
        }
    }
    else
    {
        char *module_ptr = NULL;
        int   module_len = 0;
        int   i;

        // Detailed event - they told us the module.
        param_list_locate_value(event, "module", &module_ptr, &module_len);

        if  ((module_ptr != NULL) && (module_len > 0))
        {
            for (i = 0; i < MAX_NUM_MODULES; i++)
            {
                if ((module_statuses[i].module_name != NULL) &&
                    (strlen(module_statuses[i].module_name) == module_len) &&
                    (strncmp(module_ptr, module_statuses[i].module_name, module_len) == 0))
                {
                    if (module_statuses[i].module_input_callback != NULL)
                    {
                        module_statuses[i].module_input_callback(action);
                    }
                    break;
                }
            }
            ASSERT(i < MAX_NUM_MODULES);
        }
    }
}

/******************** Debug Commands *********************/

#if defined( HAVE_CMD )

#if defined( DEBUG )
static data_block_handle_t dbg_cmd_get_data_block(int argc, char *argv[], int first_index)
{
    data_block_handle_t data_block = NULL;

    if (first_index < argc)
    {
        int cur_type_index = first_index;
        int cur_value_index = cur_type_index + 1;

        data_block = ui_sys_create_data_block();
        ASSERT(data_block != NULL);

        while (cur_type_index < argc)
        {
            data_field_type_t field_type = 0;
            int i;

            // Figure out the type for the next parameter
            for (i = 0; i < NUM_DATA_FIELD_TYPES; i++)
            {
                if (strcmp(data_field_type_as_string[i], argv[cur_type_index]) == 0)
                {
                    field_type = (data_field_type_t)i;
                    break;
                }
            }

            if ((field_type != i) || (cur_value_index >= argc))
            {
                // They either gave us a bad type, or there isn't a value for this field.
                // Either way, bail out.
                ui_sys_dispose_data_block(data_block);
                data_block = NULL;
                return NULL;
            }
            
            switch(field_type)
            {
                case e_DATA_TYPE_INT:
                    ui_sys_data_block_add_int(data_block, atoi(argv[cur_value_index]));
                    break;

                case e_DATA_TYPE_STRING:
                    ui_sys_data_block_add_string(data_block, argv[cur_value_index]);
                    break;

                default:
                    XASSERT("Unexpected field type" == 0, field_type);
                    break;
            }

            cur_type_index += 2;
            cur_value_index += 2;
        }
    }

    return data_block;
}

static int dbg_cmd_state_set(int argc, char *argv[])
{
    int result = CMD_USAGE_ERROR;

    if (argc >= 3)
    {
        data_block_handle_t data_block = dbg_cmd_get_data_block(argc, argv, 3);
        if ((argc == 3) || (data_block != NULL))
        {
            ui_sys_notify_ui_of_status(argv[1], argv[2], data_block);
            result = CMD_OK;
        }
    }

    return result;
}
#endif // defined( DEBUG )

static int dbg_show_module_state(module_status *module_status_info)
{
    cmd_printf("%-12s %-20s 0x%08x   %-8s %-10s %s\n", 
               module_status_info->module_name, 
               (module_status_info->module_state_entry)->state, 
               (unsigned int)((module_status_info->module_state_entry)->StatMgrCode), 
               module_status_info->module_status_timer != NULL ? "Yes" : "No",
               (&module_statuses[worst_index] == module_status_info) ? "Yes" : "No",
               module_status_info->status_data != NULL ? module_status_info->status_data->data_buffer : "");

    return CMD_OK;
}

static int dbg_cmd_state_get(int argc, char *argv[])
{
    uint32_t tx_res;
    int result = CMD_USAGE_ERROR;
    int i;

    // acquire the ui system interface status mutex
    tx_res = tx_mutex_get(&ui_sys_status_mutex, TX_WAIT_FOREVER);
    ASSERT(TX_SUCCESS == tx_res);

    if ((argc == 1) || (argc == 2))
    {
        cmd_printf("%-12s %-20s %-12s %-8s %-10s %s\n", "Module", "State", "Code", "Timed?", "Overall?", "Data");
    }

    if ((argc == 2) || (argc == 3))
    {
        for (i = 0; i < MAX_NUM_MODULES; i++)
        {
            if ((module_statuses[i].module_name != NULL) && (strcmp(module_statuses[i].module_name, argv[1]) == 0))
            {
                if (argc == 2)
                {
                    dbg_show_module_state(&module_statuses[i]);
                    result = CMD_OK;
                }
                else if (argc == 3)
                {
                    if (strcmpi("state", argv[2]) == 0)
                    {
                        cmd_printf("State: ");
                        cmd_append_result("%s", (module_statuses[i].module_state_entry)->state);
                        result = CMD_OK;
                    }
                    else if (strcmpi("code", argv[2]) == 0)
                    {
                        cmd_printf("Code: ");
                        cmd_append_result("%08x", (unsigned int)(module_statuses[i].module_state_entry)->StatMgrCode);
                        result = CMD_OK;
                    }
                    else if (strcmpi("data", argv[2]) == 0)
                    {
                        cmd_printf("Data: ");
                        cmd_append_result("%s", module_statuses[i].status_data != NULL ? module_statuses[i].status_data->data_buffer : "" );
                        result = CMD_OK;
                    }
                    else if (strcmpi("timed", argv[2]) == 0)
                    {
                        cmd_printf("Timed: ");
                        cmd_append_result("%s", module_statuses[i].module_status_timer ? "Yes" : "No");
                        result = CMD_OK;
                    }
                }
                break;
            }
        }
    }
    else if (argc == 1)
    {
        int i;

        for (i = 0; i < MAX_NUM_MODULES; i++)
        {
            if (module_statuses[i].module_name != NULL)
            {
                dbg_show_module_state(&module_statuses[i]);
            }
        }
        result = CMD_OK;
    }

    // release the ui system interface status mutex
    tx_res = tx_mutex_put(&ui_sys_status_mutex);
    ASSERT(TX_SUCCESS == tx_res);

    return result;
}

#if defined( DEBUG )
static int dbg_cmd_send_event(int argc, char *argv[])
{
    int result = CMD_USAGE_ERROR;

    if (argc >= 3)
    {
        data_block_handle_t data_block = dbg_cmd_get_data_block(argc, argv, 3);
        if ((argc == 3) || (data_block != NULL))
        {
            ui_sys_notify_ui_of_event(argv[1], argv[2], data_block);
            result = CMD_OK;
        }
    }

    return result;
}
#endif // defined( DEBUG )

static int dbg_cmd_xu_debug( int argc, char *argv[] )
{
    int result = CMD_OK;

    if ( argc > 1 )
    {
        int t = atoi( argv[1] );

        if ( t >= 0 )
        {
            ui_sys_debug_level = t;
        }
        else
        {
            result = CMD_USAGE_ERROR;
            cmd_printf( "ERROR: Printer debug value must be zero or greater!\n" );
        }
    }

    cmd_printf( "ui_sys_debug_level = %d\n", ui_sys_debug_level );

    return result;
}

static int dbg_cmd_user_feedback( int argc, char *argv[] )
{
    int result = CMD_OK;

    if (argc == 2)
    {
        char *event = NULL;

        if (!strcmp(argv[1], "ok"))
        {
            event = "status_response?action=ok";
        }
        else if (!strcmp(argv[1], "cancel"))
        {
            event = "status_response?action=cancel";
        }
        else if (!strcmp(argv[1], "done"))
        {
            event = "status_response?action=done";
        }
        else
        {
            result = CMD_USAGE_ERROR;
        }

        if (event) 
        {
            ui_sys_user_feedback_to_module(event);
        }
    }
    else
    {
        result = CMD_USAGE_ERROR;
    }

    return result;
}

static void register_ui_sys_util_debug_commands()
{
    cmd_register("ui_sys",
                 "ui_system_interface utilities",
                 NULL, NULL, NULL);

    cmd_register("ui_sys user_feedback", 
                 "Send user input to module with the most significant status",
                 "ok|cancel|done",
                 "",
                 dbg_cmd_user_feedback);

    cmd_register("ui_sys state", 
                 "Debug ui_system_interface module states",
                 NULL, NULL, NULL);

    cmd_register("ui_sys state get", 
                 "Show what state a module is in",
                 "[<module>] [state|code|data|timed]",
                 "If no module name is given, all known modules will be listed",
                 dbg_cmd_state_get);

#if defined( DEBUG )
    cmd_register("ui_sys state set", 
                 "Set the state of a module",
                 "<module> <state> [<param_type> <param_value> ...]",
                 "param_type may be 'int' (integer) or 'str' (string).",
                 dbg_cmd_state_set);

    cmd_register("ui_sys event", 
                 "Send an event to the UI",
                 "<module> <event> [<param_type> <param_value> ...]",
                 "param_type may be 'int' (integer), or 'str' (string).",
                 dbg_cmd_send_event);
#endif // defined( DEBUG )

    cmd_register( "ui_sys debug",
                  "Set or get the debug level for ui_system_interface_utils.",
                  "[<level>]",
                  "<level> must be 0 (zero) for false or off, or a positive integer for true or on."
                  " If <level> is not specified, the current value is printed.",
                  dbg_cmd_xu_debug );
}

#endif // defined( HAVE_CMD )


#if defined(DEBUG)
  #define EXT_UTILS_TRACE             if( ui_sys_debug_level > 3 ) { dbg_printf( "%s(%u)\n", __func__, __LINE__ ); }
  #define DUMP_KMSG_QUEUE_ENTRY(t)    if( ui_sys_debug_level > 1 ) { dump_kmsg_queue_entry( t ); }
#else
  #define EXT_UTILS_TRACE
  #define DUMP_KMSG_QUEUE_ENTRY(t) 
#endif

/******************** Prototypes for local Routines *********************/

static void ui_sys_post_ui_event(const char *module, const char *event_type, data_block_handle_t status_data);


/******************** Shared Data *********************/

static bool event_notify_initialized = false;

/* ********************* Structures, data, and routines for the kmsg queue. ********** */
typedef struct kmsg_queue_entry_t
{
    char *module;
    char *event_type;
    data_block_handle_t status_data;
    struct kmsg_queue_entry_t *next;
} kmsg_queue_entry_t;

static kmsg_queue_entry_t *kmsg_queue = 0;

static TX_SEMAPHORE kmsg_queue_semaphore;
static int kmsg_queue_semaphore_created = 0;

#if defined(DEBUG)
static void dump_kmsg_queue_entry( kmsg_queue_entry_t *p )
{
    if ( p ) 
    {
        dbg_printf( "kmsg_queue_entry\n" );
        dbg_printf( "      module: %s\n", p->module ? p->module : "NULL" );
        dbg_printf( "  event_type: %s\n", p->event_type ? p->event_type : "NULL" );
        dbg_printf( " status_data: %p\n", p->status_data );
        dbg_printf( "        next: %p\n", p->next );
    }
}
#endif // DEBUG


static int kmsg_queue_not_empty(void)
{
    return kmsg_queue != 0;
}

static void ensure_kmsg_queue_semaphore(void)
{
    EXT_UTILS_TRACE 

    if ( kmsg_queue_semaphore_created == 0 ) 
    {
        unsigned int status;
        status = tx_semaphore_create( &kmsg_queue_semaphore, "KMSG_QUEUE", 1 );
        ASSERT( status == TX_SUCCESS );

        kmsg_queue_semaphore_created = 1;
    }

    EXT_UTILS_TRACE 
}

static int acquire_kmsg_queue_semaphore(void)
{
    unsigned int status;

    EXT_UTILS_TRACE

    // Initialize the semaphore, if needed, before we try to acquire it.
    ensure_kmsg_queue_semaphore();

    ASSERT( kmsg_queue_semaphore_created );

    status = tx_semaphore_get( &kmsg_queue_semaphore, TX_WAIT_FOREVER );
    ASSERT(status == TX_SUCCESS);

    EXT_UTILS_TRACE

    return status == TX_SUCCESS;
}

static int release_kmsg_queue_semaphore(void)
{
    unsigned int status;

    EXT_UTILS_TRACE

    ASSERT( kmsg_queue_semaphore_created );

    status = tx_semaphore_put( &kmsg_queue_semaphore );
    ASSERT(status == TX_SUCCESS);

    EXT_UTILS_TRACE

    return status == TX_SUCCESS;
}

static void dispose_kmsg( kmsg_queue_entry_t *p )
{
    EXT_UTILS_TRACE 

    if ( p ) 
    {
        if ( p->module ) 
        {
            MEM_FREE_AND_NULL( p->module );
        }

        if ( p->event_type ) 
        {
            MEM_FREE_AND_NULL( p->event_type );
        }

        // We're not responsible for the status_data pointer. The UI should deallocate that when the message is posted.

        MEM_FREE_AND_NULL( p );
    }

    EXT_UTILS_TRACE 
}

static kmsg_queue_entry_t * new_kmsg( const char *module, const char *event_type, data_block_handle_t status_data )
{
    kmsg_queue_entry_t *t = 0;
    int fail = 0;

    EXT_UTILS_TRACE 

    t = (kmsg_queue_entry_t *) MEM_MALLOC_ALIGN( sizeof(kmsg_queue_entry_t), e_8_byte );

    if ( t ) 
    {
        if ( module ) 
        {
            t->module = MEM_MALLOC_ALIGN(strlen(module)+1, e_8_byte);
            if (t->module)
            {
                strcpy(t->module, module);
            }

            if ( t->module == 0 ) 
            {
                fail = 1;
                dbg_printf( "%s(%u): MEM_MALLOC_ALIGN failed!\n", __func__, __LINE__ );
            }
        }
        else
        {
            t->module = 0;
        }

        if ( event_type ) 
        {
            t->event_type = MEM_MALLOC_ALIGN(strlen(event_type)+1, e_8_byte);
            if (t->event_type)
            {
                strcpy(t->event_type, event_type);
            }

            if ( t->event_type == 0 ) 
            {
                fail = 1;
                dbg_printf( "%s(%u): MEM_MALLOC_ALIGN failed!\n", __func__, __LINE__ );
            }
        }
        else
        {
            t->event_type = 0;
        }

        t->status_data = status_data;
        t->next = 0;

        if ( fail )
        {
            dispose_kmsg( t );
            t = 0;
        }
    }
    else
    {
        dbg_printf( "%s(%u): MEM_MALLOC_ALIGN failed!\n", __func__, __LINE__ );
    }

    EXT_UTILS_TRACE 

    return t;
}

static kmsg_queue_entry_t *enqueue_ui_event( const char *module, const char *event_type, data_block_handle_t status_data )
{
    kmsg_queue_entry_t *t = 0;

    EXT_UTILS_TRACE 

    t = new_kmsg( module, event_type, status_data );

    if ( t ) 
    {
        if( acquire_kmsg_queue_semaphore() )
        {
            if ( kmsg_queue == 0 ) 
            {
                kmsg_queue = t;
            }
            else
            {
                kmsg_queue_entry_t *p = kmsg_queue;

                while ( p->next ) 
                {
                    p = p->next;
                }

                p->next = t;
            }

            release_kmsg_queue_semaphore();

            DUMP_KMSG_QUEUE_ENTRY( t );
        }
    }
    else
    {
        dbg_printf( "%s(%u): MEM_CALLOC failed!\n", __func__, __LINE__ );
    }

    EXT_UTILS_TRACE 

    return t;
}


static kmsg_queue_entry_t *dequeue_ui_event(void)
{
    kmsg_queue_entry_t *p = 0;

    EXT_UTILS_TRACE 

    if( acquire_kmsg_queue_semaphore() )
    {
        p = kmsg_queue;

        if ( kmsg_queue ) 
        {
            kmsg_queue = kmsg_queue->next;
        }

        release_kmsg_queue_semaphore();
    }

    EXT_UTILS_TRACE 

    return p;
}


static int purge_kmsg_queue(void)
{
    int count = 0;

    EXT_UTILS_TRACE 

    while( kmsg_queue_not_empty() ) 
    {
        kmsg_queue_entry_t *p = dequeue_ui_event();

        ASSERT( p );

        ui_sys_post_ui_event( p->module, p->event_type, p->status_data );

        DUMP_KMSG_QUEUE_ENTRY( p );

        dispose_kmsg( p );

        ++count;
    }

    EXT_UTILS_TRACE 

    return count;
}

static void event_notify_init()
{
    event_notify_initialized = true;

    // Send any UI events that may have been queued on to the UI.
    purge_kmsg_queue();
}

/** 
 * \brief Replace any restricted url characters with their 
 *        encoded values so they can be included in a url
 * 
 * \param str - original string
 *  
 * \note The returned buffer is a new memory allocation and must be freed by the caller.
 *  
 * Replaces characters that are reserved with the correct 
 * encoding.  EX: "Me & you" becomes "Me%20%26%20you" 
 * 
 **/
char *url_encode(char *str)
{
    int len = strlen(str);
    int i, j;
    char *encoded;
    int newLen;

    // First, figure out how long the encoded string will be.
    newLen = len;
    for ( i = 0; i < len; i++ )
    {
        if ( !isalnum( str[i] ) )
        {
            newLen += 2;
        }
    }

    // Allocate a big enough buffer and copy characters while replacing the ones
    // that need to be encoded.
    encoded = MEM_CALLOC_ALIGN(1, newLen + 1, e_8_byte);
    if ( encoded == NULL )
    {
        return NULL;
    }

    j = 0;
    for ( i = 0; i < len; i++ )
    {
        if ( isalnum( str[i] ) )
        {
            encoded[j] = str[i];
            j++;
        }
        else
        {
            sprintf( encoded + j, "%%%2X", str[i] );
            j += 3;
        }
    }
    ASSERT(j == newLen);
    encoded[newLen] = 0;

    return encoded;
}

/************ Routines for Data Blocks that accompany status updates and events *********************/

data_block_handle_t ui_sys_create_data_block()
{
    data_block_handle_t new_data_block = NULL;

    new_data_block = MEM_CALLOC_ALIGN(1, sizeof(struct data_block_s), e_8_byte);

    if (new_data_block != NULL)
    {
        new_data_block->num_fields = 0;
        new_data_block->cur_buffer_capacity = 32;   // Most data blocks are small.  They will grow if needed.
        new_data_block->cur_buffer_size = 1;        // Even when empty, they contain a null terminator.
        new_data_block->data_buffer = MEM_CALLOC_ALIGN(1, new_data_block->cur_buffer_capacity, e_8_byte);

        if (new_data_block->data_buffer == NULL)
        {
            MEM_FREE_AND_NULL(new_data_block);
            new_data_block = NULL;
        }
        else
        {
            new_data_block->data_buffer[0] = '\0';
        }
    }

    return new_data_block;
}

void ui_sys_dispose_data_block(data_block_handle_t old_block)
{
    if (old_block != NULL)
    {
        if (old_block->data_buffer != NULL)
        {
            MEM_FREE_AND_NULL(old_block->data_buffer);
        }
        MEM_FREE_AND_NULL(old_block);
    }
}

static error_type_t ui_sys_data_block_add_field(data_block_handle_t data_block, data_field_type_t field_type, const char *field_as_str)
{
    error_type_t result = FAIL;

    // ToDo - we should probably url encode the string before we use it.

    if (data_block != NULL && field_as_str != NULL)
    {
        int bytes_needed = 0;
        int next_field_index = data_block->num_fields + 1;

        // First, figure out how much space the data block is going to need
        // We will add in the form "[&]pt#=<type>&pv#=<value>"

        if (next_field_index != 1)
        {
            bytes_needed++; // For initial '&' since this isn't the first param
        }

        // need space for "pt=" and "&pv="
        bytes_needed += 7;

        // need space for type and value
        bytes_needed += strlen(data_field_type_as_string[field_type]);
        bytes_needed += strlen(field_as_str);

        // need space for field index (*2 because it's on both type and value)
        if (next_field_index < 10)
        {
            bytes_needed += 1 * 2;  // single digit (1 char for index, name and value)
        }
        else if (next_field_index < 100)
        {
            bytes_needed += 2 * 2;  // double digit (2 chars for index, name and value)
        }
        else
        {
            // If the assert fires, compute how many bytes you need for your index.
            ASSERT("640k wasn't big enough?  You want more than 99 parameters, too?" == 0);
        }

        // If there isn't room for the new data make more space
        if ((data_block->cur_buffer_size + bytes_needed) > data_block->cur_buffer_capacity)
        {
            char *new_data_buffer = NULL;
            int   new_capacity = data_block->cur_buffer_capacity;

            // Keep doubling the capacity until we have all the space we need.
            while((data_block->cur_buffer_size + bytes_needed) > new_capacity)
            {
                new_capacity *= 2;
            }

            new_data_buffer = MEM_CALLOC_ALIGN(1, new_capacity, e_8_byte);

            if (new_data_buffer != NULL)
            {
                memcpy(new_data_buffer, data_block->data_buffer, data_block->cur_buffer_size);

                MEM_FREE_AND_NULL(data_block->data_buffer);

                data_block->cur_buffer_capacity = new_capacity;
                data_block->data_buffer = new_data_buffer;
            }
        }

        // Double-check size one last time before we write to make sure we didn't run out of
        // memory on a re-alloc.
        if ((data_block->cur_buffer_size + bytes_needed) <= data_block->cur_buffer_capacity)
        {
            int bytes_used;

            // Now actually write the parameter
            bytes_used = sprintf(data_block->data_buffer + data_block->cur_buffer_size - 1, "%spt%d=%s&pv%d=%s",
                                 next_field_index > 1 ? "&" : "",
                                 next_field_index,
                                 data_field_type_as_string[field_type],
                                 next_field_index,
                                 field_as_str);

            // if this fires, we need to check the math and reserve the correct number of bytes
            ASSERT(bytes_needed == bytes_used);

            data_block->cur_buffer_size += bytes_used;
            ASSERT(data_block->cur_buffer_size <= data_block->cur_buffer_capacity);

            data_block->num_fields++;
            result = OK;
        }
    }

    return result;
}

error_type_t ui_sys_data_block_add_int(data_block_handle_t data_block, int value)
{
    error_type_t result = FAIL;

    if (data_block != NULL)
    {
        char int_as_str[15];   // Enough space to hold any 32-bit int as a string

        sprintf(int_as_str, "%d", value);
        result = ui_sys_data_block_add_field(data_block, e_DATA_TYPE_INT, int_as_str);
    }

    return result;
}

error_type_t ui_sys_data_block_add_string(data_block_handle_t data_block, const char *value)
{
    error_type_t result = FAIL;

    if ((data_block != NULL) && (value != NULL))
    {
        char *encoded_value;

        encoded_value = url_encode((char *)value);

        if (encoded_value != NULL)
        {
            result = ui_sys_data_block_add_field(data_block, e_DATA_TYPE_STRING, encoded_value);

            MEM_FREE_AND_NULL(encoded_value);
        }
    }

    return result;
}

static void ui_sys_post_ui_event(const char *module, const char *event_type, data_block_handle_t status_data)
{
    ASSERT( event_notify_initialized == true );

    if (module != NULL && event_type != NULL)
    {
        char *msg_command = NULL;
        uint32_t msg_len = 100;
        
        msg_len += strlen(module);
        msg_len += strlen(event_type);
        
        if (status_data != NULL)
        {
            msg_len += status_data->cur_buffer_size;
            msg_len += strlen(URL_PARAM_COUNT_TEXT);     // We will attach a param count in addition to the params
        }
        
        DPRINTF((DBG_OUTPUT|DBG_SOFT), ("Event notification: %s, %s\n", module, event_type));
        
        msg_command = MEM_CALLOC_ALIGN(1, msg_len, e_8_byte);
        ASSERT(msg_command != NULL);
        
        strcat((char *) msg_command, "module=");
        strcat((char *) msg_command, module);
        strcat((char *) msg_command, "&event=");
        strcat((char *) msg_command, event_type);
        
        if (status_data != NULL)
        {
            ASSERT(status_data->data_buffer != NULL);
            sprintf((char *) msg_command + strlen((char *)msg_command), URL_PARAM_COUNT_TEXT, (int)(status_data->num_fields));
            strcat((char *) msg_command, status_data->data_buffer);
            ui_sys_dispose_data_block(status_data);
            status_data = NULL;
        }
        
        notify_registered_output_callbacks(msg_command);
    }
}

void ui_sys_notify_ui_of_event(const char *module, const char *event_type, data_block_handle_t status_data)
{
    if( event_notify_initialized == false )
    {
        if ( module && event_type ) 
        {
            enqueue_ui_event( module, event_type, status_data );
        }
        else
        {
            dbg_printf( "%s(%u) Invalid params passed to ui_sys_notify_ui_of_event: "
                        "module = %p, event_type = %p\n", __func__, __LINE__, module, event_type );
        }
    }
    else
    {
        purge_kmsg_queue();

        ui_sys_post_ui_event( module, event_type, status_data );
    }
}


