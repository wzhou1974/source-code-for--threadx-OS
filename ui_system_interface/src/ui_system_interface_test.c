/*
 * ============================================================================
 * Copyright (c) 2013   Marvell International, Ltd. All Rights Reserved
 *
 *                         Marvell Confidential
 * ============================================================================
 */

/** 
 * \file ui_system_interface_test.c
 * 
 * \brief Test harness for the UI System Interface. 
 *        
 * 
 **/

#include "ui_system_interface_api.h"
#include "logger.h"
#if defined( HAVE_CMD )
#include "cmd_proc_api.h"
#endif // defined( HAVE_CMD )

#if defined( HAVE_COPY_SUPPORT )
#include "copyapp_api.h"
#include "ui_system_interface_copy.h"
#endif



#if defined( HAVE_CMD )
static int test_advance(int argc, char *argv[])
{
    static int step = 0;

    DPRINTF((DBG_OUTPUT|DBG_LOUD), ("test_advance: advancing to next test step\n"));
    switch (step++)
    {
        case 0:
            // simulate SDK-supported module entering SDK-supported state
            ui_sys_notify_ui_of_status("print", "printing", NULL);
            break;

        case 1:
            // simulate SDK-supported module generating SDK-supported event
            ui_sys_notify_ui_of_event("wireless", "link_connected", NULL);
            break;

        case 2:
            // simulate SDK-supported module generating OEM custom status
            ui_sys_notify_ui_of_status("platform", "oem-status2", NULL);
            break;

        case 3:
            // register OEM custom module
            ui_sys_register_module_for_status("oem-module", "idle", NULL);
            break;

        case 4:
            // simulate OEM custom module entering OEM custom state
            ui_sys_notify_ui_of_status("oem-module", "oem-status1", NULL);
            break;

        default:
            break;
    }

    return CMD_OK;
}

#if defined( HAVE_COPY_SUPPORT )
static int test_start_copy(int argc, char *argv[])
{
    CopyConf test_copy_settings;

    // get the current non-volatile copy settings and use them to start a copy job
    DPRINTF((DBG_OUTPUT|DBG_LOUD), ("test_start_copy: starting a copy from the UI\n"));
    ui_sys_copy_settings_get(&test_copy_settings);
    ui_sys_copy_startjob(&test_copy_settings);

    return CMD_OK;
}
#endif  // defined( HAVE_COPY_SUPPORT )

#endif //     #if defined( HAVE_CMD )

static void test_output_callback(char *message)
{
    // output the received status or event message
    DPRINTF((DBG_OUTPUT|DBG_LOUD), ("test_output_callback: %s\n", message));
}

void ui_sys_test_init()
{
    // register our debug command(s)
    #if defined( HAVE_CMD )
    cmd_register("ui_sys test", 
                 "Test the ui_system_interface",
                 NULL, NULL, NULL);

    cmd_register("ui_sys test advance", 
                 "Advance to the next ui_system_interface test step",
                 "",
                 "",
                 test_advance);

    #if defined( HAVE_COPY_SUPPORT )
    cmd_register("ui_sys test start_copy", 
                 "Start a copy job from the ui_system_interface",
                 "",
                 "",
                 test_start_copy);
    #endif  // #if defined( HAVE_COPY_SUPPORT )

    #endif // #if defined( HAVE_CMD )

    // register our callback to receive ui_system_interface output
    ui_sys_register_output_callback(test_output_callback);
}



