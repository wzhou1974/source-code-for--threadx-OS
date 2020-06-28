/*
 * ============================================================================
 * Copyright (c) 2013   Marvell International, Ltd. All Rights Reserved
 *
 *                         Marvell Confidential
 * ============================================================================
 */

/** 
 * \file ui_system_interface_api.h
 * 
 * \brief Public interface for the UI System Interface module.
 * 
 * The user interface code needs to communicate with various parts of the system
 * product code. This API simplifies that process by providing a single 
 * consistent access point for that communication.
 * 
 **/

#ifndef __UI_SYSTEM_INTERFACE_API_H__
#define __UI_SYSTEM_INTERFACE_API_H__


#include "error_types.h"
#include "stdbool.h"

/** 
 *  @brief Called at bootup to initialize the ui_system_interface module.
 * 
 */
void ui_sys_init();


/************ API for defining & prioritizing status messages *********************/

#define STATUS_SEVERITY_MASK    0x70000000   //Bits 30-28: 8 possible severities
#define STATUS_PRIORITY_MASK    0x0C000000   //Bits 27-26: 4 possible priorities for each severity
#define STATUS_SEVPRI_MASK    (STATUS_SEVERITY_MASK|STATUS_PRIORITY_MASK)   //mask to get the severity and priority

//Status severities: 
// Fatal:     Can't print, requires service - motor, sensor, fan failures etc. 
// Critical:  Can't print but should be fixable by user - out of paper, out of toner, paper jam etc
// Testing:   Can't print, running diagnostics or doing a calibration
// Info:      Printing or canceling a job, more important that a warning but less important than errors
// Warning:   Can still print- calibration needed, toner low etc.
#define UI_SYS_STATSEV_FATAL     0x70000000
#define UI_SYS_STATSEV_CRITICAL  0x60000000
#define UI_SYS_STATSEV_TESTING   0x40000000
#define UI_SYS_STATSEV_INFO_HIGH 0x30000000
#define UI_SYS_STATSEV_INFO      0x20000000
#define UI_SYS_STATSEV_WARNING   0x10000000
#define UI_SYS_STATSEV_NONE      0x00000000  //means everything is okay, no warning, no errors

//Status priorities: allows prioritization among different status of the
//same severity. Whichever status has the highest severity and highest
//priority will be displayed first
#define UI_SYS_STATPRI_0       0x00000000  //used for online category only so that it is the lowest sev and priority
#define UI_SYS_STATPRI_1       0x04000000
#define UI_SYS_STATPRI_2       0x08000000
#define UI_SYS_STATPRI_3       0x0C000000  //highest priority

//compare stat1 and stat2, return stat1 if it is greater else return stat2
#define STATGETWORSE(stat1, stat2) (((stat1&STATUS_SEVPRI_MASK) > (stat2&STATUS_SEVPRI_MASK))?stat1:stat2)


/************ API for Data Blocks that accompany status updates and events *********************/

struct data_block_s
{
    uint32_t  num_fields;
    uint32_t  cur_buffer_capacity;
    uint32_t  cur_buffer_size;
    char     *data_buffer;
};
typedef struct data_block_s *data_block_handle_t;

/** 
 *  @brief Create a data block to hold paramters associated with
 *         a status message or event.
 * 
 *  @return Instance handle for the new data block.  NULL if
 *          there was an error.
 *  
 *  @note The caller does not need to worry about freeing the
 *        data block.  It should be provided as a parameter to a
 *        status update or event notification and will be freed
 *        by those systems when appropriate.
 */
data_block_handle_t ui_sys_create_data_block();


/** 
 *  @brief Free a data block that was created, but wasn't sent
 *         along with a state or event.
 * 
 *  @return Instance handle for the block.
 *  
 *  @note If the data block was passed as a parameter to
 *        ui_sys_notify_ui_of_status or
 *        ui_sys_notify_ui_of_event, this SHOULD NOT be called.
 *        Those routines will do it for you.
 */
void ui_sys_dispose_data_block(data_block_handle_t old_block);

/** 
 *  @brief Add an integer parameter to an existing data block
 * 
 *  @param data_block Instance handle of the block that will
 *                    store the int value
 *  
 *  @param value The integer to store
 *  
 *  @return OK on success, FAIL otherwise
 * 
 */
error_type_t ui_sys_data_block_add_int(data_block_handle_t data_block, int value);

/** 
 *  @brief Add a string parameter to an existing data block
 * 
 *  @param data_block Instance handle of the block that will
 *                    store the int value
 *  
 *  @param value The string to store
 *  
 *  @return OK on success, FAIL otherwise
 * 
 */
error_type_t ui_sys_data_block_add_string(data_block_handle_t data_block, const char *value);


/************ API for status updates and events *********************/

#define GENERAL_STATUS_IDLE             "idle"
#define GENERAL_STATUS_READY            "ready"
#define GENERAL_STATUS_INITIALIZING     "initializing"


// Module name and idle status strings for the system module.  These are placed
// here instead of in the module that implements the system module status since
// this is a shared module.
#define SYSTEM_MODULE_NAME              "system"
#define SYSTEM_STATUS_IDLE              GENERAL_STATUS_IDLE


/** 
 *  @brief Send an event to the UI from a given module.
 * 
 *  @param module Name of the module sourcing the event
 *  
 *  @param event_type Name of the event
 *  
 *  @param data A data block containing all relevant info about
 *              the event (signal strength in bars, etc).
 * 
 */
void ui_sys_notify_ui_of_event(const char *module, const char *event_type, data_block_handle_t data);


/************ API for user feedback to modules *********************/

#define UI_EVENT_BTN_CANCEL         "cancel"
#define UI_EVENT_RESPONSE_OK        "ok"
#define UI_EVENT_RESPONSE_DONE      "done"

/// Callback used by UI to send feedback to a module.
///   Returns true if the command was handled, false otherwise.
typedef bool (*ui_sys_input_callback_t)(const char *command);

/** 
 *  @brief Register a module with the UI's status system.  Needs
 *         to be done before the module can source status
 *         updates.
 * 
 *  @param module Name of the module
 *  
 *  @param initial_state Initial state of the module
 *  
 *  @param input_callback Callback that should be used to inform
 *                        the module of user events (cancel
 *                        button, responses to prompts, etc)
 *                        related to this module.
 * 
 */
void ui_sys_register_module_for_status(const char *module, const char *initial_state, ui_sys_input_callback_t input_callback);

/** 
 *  @brief Set the status of a given module.  Optionally
 *         provides a data block with extra info about the
 *         status.
 * 
 *  @param module Name of the module
 *  
 *  @param new_state Name of the module's new state
 *  
 *  @param data A data block containing all relevant info about
 *              the status (page number, etc).
 * 
 */
void ui_sys_notify_ui_of_status(const char *module, const char *new_state, data_block_handle_t data);


/** 
 *  @brief Forward the UI user input to the pertinent ui_system_interface
 *         module.
 * 
 *  @param event The user input to forward.  If may optionally include the name
 *               of the module to forward to, otherwise the input is forwarded
 *               to the module which currently has the most significant status.
 * 
 */
void ui_sys_user_feedback_to_module(const char* event);


/************ API for ui_system_interface notification to UI application *********************/

typedef void (*ui_sys_output_callback_t)(char *message);

/** 
 *  @brief Register the requested callback for notification of
 *         ui_system_interface output (status and events).
 * 
 *  @param callback The callback function to register.  Note that the
 *               implementation of this callback is expected to copy the content
 *               of its received message to its own buffer as the original
 *               message will be freed after notifying all registered
 *               callbacks.
 */
void ui_sys_register_output_callback(ui_sys_output_callback_t callback);

#endif // #ifndef __UI_SYSTEM_INTERFACE_API_H__

