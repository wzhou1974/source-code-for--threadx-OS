/*
 * ============================================================================
 * Copyright (c) 2011-2013   Marvell International, Ltd. All Rights Reserved
 *
 *                         Marvell Confidential
 * ============================================================================
 */

#ifndef __UI_SYSTEM_INTERFACE_STATUS_H__
#define __UI_SYSTEM_INTERFACE_STATUS_H__
 
#include "ATypes.h" 

/** \enum MSG_TYPE_TAG
 *  \brief The types of status messages
 * 
 */
typedef enum MSG_TYPE_TAG
{
	/** System online msg.*/
	MSG_READY = 1,
	
	/** Job processing msg, such as "Copying..", "Printing report" */
	MSG_JOB,
	
	/** Prompt msg that requires user's input */
	MSG_CONFIRM_PROMPT,
	
	/** Message that disappears after 2 sec.*/
	MSG_TRANSITORY,
	
	/** Alert message, user's intervene is required */
	MSG_ALERT,
	
	/** Warning message, it disappears after 5 sec */
	MSG_WARN,

    /** Notification message, no user interaction with the control panel needed */
    MSG_NOTIFY,
	
    /** Announce message, ignore all keypresses & ack resource after 6 sec */
    MSG_ANNOUNCE,

    /** Critical system error message*/
	MSG_FATAL,

    NUM_MSG_TYPES_PLUS_ONE
}MSG_TYPE;

/** \struct StatMsgStruct
 *  \brief The interface for a status message
 */
typedef struct StatMsgStruct
{
    // type of the status
    MSG_TYPE msgType;
    
    // severity and priority of the status
    uint32_t  StatMgrCode; 
    
    // name and state of the module sourcing this status
    const char *module;
    const char *state;

} StatMsg;


const char *findOnlineStatusForModule( const char *module_name );
StatMsg *find_module_state_entry(const char *module, const char *state);
const char *status_type_to_string(MSG_TYPE cur_type);


#endif // #ifndef __UI_SYSTEM_INTERFACE_STATUS_H__

