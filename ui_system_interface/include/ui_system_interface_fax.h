/*
 * ============================================================================
 * Copyright (c) 2011-2013   Marvell International, Ltd. All Rights Reserved
 *
 *                         Marvell Confidential
 * ============================================================================
 */

/** 
 * \file ui_system_interface_fax.h
 * 
 * \brief 
 * 
 * 
 **/

#ifndef __UI_SYSTEM_INETRFACE_FAX_H__
#define __UI_SYSTEM_INETRFACE_FAX_H__

#include <stdint.h>
#include <stdbool.h>
#include "fax_app_vars.h"
#include "ui_system_interface_fax.h"

/**
 * module name related to fax module
 */
#define FAX_MODULE_NAME                 "fax"
#define FAXSCAN_MODULE_NAME             "fax_scan"
#define FAXPRINT_MODULE_NAME            "fax_print"

/**
 * status message related to fax module
 */

#define FAX_STATUS_READY                "ready"
#define FAX_STATUS_INCOMING_CALL        "incoming_call"
#define FAX_STATUS_DIALING              "dialing"
#define FAX_STATUS_CONNECTING           "connecting"
#define FAX_STATUS_SENDING              "sending"
#define FAX_STATUS_RECEIVING            "receiving"
#define FAX_STATUS_CANCELING_SEND       "canceling_send"
#define FAX_STATUS_CANCELING_RECV       "canceling_recv"
#define FAX_STATUS_SELF_TEST            "self_test"
#define FAX_STATUS_VOICE_CALL           "voice_call"
#define FAX_STATUS_VOICE_CANCELING      "voice_canceling"

#define FAX_RESULT_SENT_OK              "sent_ok"
#define FAX_RESULT_RCVD_OK              "received_ok"
#define FAX_RESULT_SENT_RTN             "sent_rtn"
#define FAX_RESULT_RCVD_RTN             "received_rtn"
#define FAX_RESULT_NO_FAX_DETECTED      "no_fax_detected"
#define FAX_RESULT_SENT_PARTIAL         "sent_partial"
#define FAX_RESULT_RCVD_PARTIAL         "received_partial"
#define FAX_RESULT_FS_ERROR_SEND        "file_error_send"
#define FAX_RESULT_FS_ERROR_RECV        "file_error_recv"
#define FAX_RESULT_FS_FULL_SEND         "file_sys_full_send"
#define FAX_RESULT_FS_FULL_RECV         "file_sys_full_recv"
#define FAX_RESULT_LINE_BUSY            "line_busy"
#define FAX_RESULT_NO_ANSWER            "no_answer"
#define FAX_RESULT_COMM_ERROR           "comm_error"
#define FAX_RESULT_NO_DIAL_TONE         "no_dial_tone"
#define FAX_RESULT_BLOCKED              "blocked"

#define FAX_EVENT_USER_DIAL             "user_dial"
#define FAX_EVENT_TEST_DONE             "test_done"

#define FAX_EVENT_DETAILS_START         "start"
#define FAX_EVENT_DETAILS_STOP          "stop"

#define FAX_TEST_RESULT_PASS            "pass"
#define FAX_TEST_RESULT_FAIL            "fail"

#define FAX_SCAN_STATUS_READY           GENERAL_STATUS_IDLE
#define FAX_SCAN_STATUS_STORING         "storing"
#define FAX_SCAN_STATUS_PROMPT_MORE     "prompt_more"
#define FAX_SCAN_STATUS_LOADPAGE        "load_page"
#define FAX_SCAN_STATUS_CANCELING       "canceling"
#define FAX_SCAN_STATUS_SYSTEM_BUSY     "system_busy"
#define FAX_SCAN_STATUS_JOB_ADDED       "job_added"
#define FAX_SCAN_STATUS_JOB_SCHEDULED   "job_scheduled"

#define FAX_PRINT_STATUS_READY          GENERAL_STATUS_READY
#define FAX_PRINT_STATUS_PRINTING       "printing"
#define FAX_PRINT_STATUS_CANCELING      "canceling"

#define FAX_IPAGE_T30                   "T30Trace"
#define FAX_IPAGE_ACTIVITY_LOG          "FaxActivityLog"


typedef enum
{
    e_UI_SYS_FAX_FAST,
    e_UI_SYS_FAX_MEDIUM,
    e_UI_SYS_FAX_SLOW
} ui_sys_fax_max_speed_t;


bool ui_sys_fax_settings_volume_phoneline_get(fax_volume_setting_t * const volume);
bool ui_sys_fax_settings_volume_phoneline_set(const fax_volume_setting_t volume);
bool ui_sys_fax_settings_volume_ring_get(fax_volume_setting_t * const volume);
bool ui_sys_fax_settings_volume_ring_set(const fax_volume_setting_t volume);
bool ui_sys_fax_settings_recv_action_get(fax_recv_action_t * const action);
bool ui_sys_fax_fettings_recv_action_set(const fax_recv_action_t action);
bool ui_sys_fax_settings_recv_answer_mode_get(fax_ans_mode_t * const ans_mode);
bool ui_sys_fax_settings_recv_answer_mode_set(const fax_ans_mode_t ans_mode);
void ui_sys_fax_settings_recv_rings_to_answer_get(uint32_t * rings_to_answer);
void ui_sys_fax_settings_recv_rings_to_answer_set(uint32_t rings_to_answer);
bool ui_sys_fax_settings_recv_ring_detect_get(fax_distinct_ring_t * const ring_type);
bool ui_sys_fax_settings_recv_ring_detect_set(const fax_distinct_ring_t ring_type);
bool ui_sys_fax_settings_recv_caller_id_detect_get(fax_caller_id_t * const cid_type);
bool ui_sys_fax_settings_recv_caller_id_detect_set(const fax_caller_id_t cid_type);
void ui_sys_fax_settings_recv_forward_dest_get(char * forwardDest, const uint32_t buf_size);
void ui_sys_fax_settings_recv_forward_dest_set(char * forwardDest);
bool ui_sys_fax_settings_recv_stamp_fax_get(void);
void ui_sys_fax_settings_recv_stamp_fax_set(const bool stamp);
bool ui_sys_fax_settings_recv_fit_to_page_get(void);
void ui_sys_fax_settings_recv_fit_to_page_set(const bool fit_to_page);
bool ui_sys_fax_settings_recv_faxtel_ringtime_get(uint32_t * const ring_time);
bool ui_sys_fax_settings_recv_faxtel_ringtime_set(const uint32_t ring_time);
bool ui_sys_fax_settings_recv_block_junkfaxes_get(bool * const block_junk);
bool ui_sys_fax_settings_recv_block_junkfaxes_set(const bool block_junk);
bool ui_sys_fax_settings_send_scan_resolution_get(fax_resolution_t * const resolution);
bool ui_sys_fax_settings_send_scan_resolution_set(const fax_resolution_t resolution_str);
bool ui_sys_fax_settings_send_scan_brightness_get(uint32_t * const brightness);
bool ui_sys_fax_settings_send_scan_brightness_set(const uint32_t brightness);
char * ui_sys_fax_settings_send_scan_glass_size_get(void);
void ui_sys_fax_settings_send_scan_glass_size_set(char * glass_str);
void ui_sys_fax_settings_send_dial_mode_get(fax_dial_mode_t * const dial_mode);
void ui_sys_fax_settings_send_dial_mode_set(const fax_dial_mode_t dial_mode);
void ui_sys_fax_Settings_Send_prefix_get(char * const dial_prefix, size_t buf_size);
void ui_sys_fax_settings_send_prefix_set(const char *dial_prefix);
void ui_sys_fax_settings_send_prefix_enabled_get(bool * const prefix_enabled);
void ui_sys_fax_settings_send_prefix_enabled_set(const bool prefix_enabled);
void ui_sys_fax_settings_send_redial_if_busy_get(bool * const redial);
void ui_sys_fax_settings_send_redial_if_busy_set(const bool redial);
void ui_sys_fax_settings_send_redial_if_no_answer_get(bool * const redial);
void ui_sys_fax_settings_send_redial_if_no_answer_set(const bool redial);
void ui_sys_fax_settings_send_redial_if_comm_error_get(bool const * redial);
void ui_sys_fax_settings_send_redial_if_comm_error_set(const bool redial);
bool ui_sys_fax_settings_send_require_dial_tone_get(bool * const require_dial_tone);
void ui_sys_fax_settings_send_require_dial_tone_set(const bool require_dial_tone);
void ui_sys_fax_settings_header_name_get(char * const header_name, size_t buf_size);
void ui_sys_fax_settings_header_name_set(const char *header_name);
void ui_sys_fax_settings_header_number_get(char * const header_number, size_t buf_size);
void ui_sys_fax_settings_header_number_set(const char *header_number);
bool ui_sys_fax_settings_reports_add_thumbnail_get(bool * const addThumbnail);
bool ui_sys_fax_settings_reports_add_thumbnail_set(const bool addThumbnail);
bool ui_sys_fax_settings_reports_confirmation_get(fax_autoprint_confirm_t * const rpt_confirmation);
bool ui_sys_fax_settings_reports_confirmation_set(const fax_autoprint_confirm_t rpt_confirmation);
bool ui_sys_fax_settings_reports_error_report_get(fax_autoprint_error_t * const rpt_error);
bool ui_sys_fax_settings_reports_error_report_set(const fax_autoprint_error_t rpt_error);
bool ui_sys_fax_settings_reports_activity_log_get(bool * const auto_print_log);
bool ui_sys_fax_settings_reports_activity_log_set(const bool auto_print_log);
bool ui_sys_fax_settings_reports_t30_trace_get(fax_autoprint_t30_t * const rpt_t30);
bool ui_sys_fax_settings_reports_t30_trace_set(const fax_autoprint_t30_t rpt_t30);
bool ui_sys_fax_settings_max_speed_get(ui_sys_fax_max_speed_t * const speed);
bool ui_sys_fax_settings_max_speed_set(const ui_sys_fax_max_speed_t speed);
bool ui_sys_fax_settings_allow_ECM_get(bool * const allow_ecm);
bool ui_sys_fax_settings_allow_ECM_set(const bool allow_ecm);

#endif // ifndef __UI_SYSTEM_INETRFACE_FAX_H__

