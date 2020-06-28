/*
 * ============================================================================
 * Copyright (c) 2013   Marvell International, Ltd. All Rights Reserved
 *
 *                         Marvell Confidential
 * ============================================================================
 */

/** 
 * \file ui_system_interface_fax.c
 * 
 * \brief Implementations of fax part of ui_system_interface
 *        
 * 
 **/

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "lassert.h"
#include "ui_system_interface_fax.h"

extern void *fax_service_handle;

#define ABORT_IF_FSM_NOT_READY          if (fax_service_handle == NULL) { return; }
#define RETURN_NULL_IF_FSM_NOT_READY    if (fax_service_handle == NULL) { return NULL; }
#define RETURN_FALSE_IF_FSM_NOT_READY   if (fax_service_handle == NULL) { return false; }
#define RETURN_BOOLEAN                  return result == OK ? true : false;


/******************** Fax Parameters *********************/

bool ui_sys_fax_settings_volume_phoneline_get(fax_volume_setting_t * const volume)
{
    RETURN_FALSE_IF_FSM_NOT_READY

    ASSERT(volume != NULL);

    error_type_t result = fax_app_get_var_volume_phoneline(volume);

    RETURN_BOOLEAN
}

bool ui_sys_fax_settings_volume_phoneline_set(const fax_volume_setting_t volume)
{
    RETURN_FALSE_IF_FSM_NOT_READY

    error_type_t result = fax_app_set_var_volume_phoneline(&volume);

    RETURN_BOOLEAN
}

bool ui_sys_fax_settings_volume_ring_get(fax_volume_setting_t * const volume)
{
    RETURN_FALSE_IF_FSM_NOT_READY

    ASSERT(volume != NULL);

    error_type_t result = fax_app_get_var_volume_ring(volume);

    RETURN_BOOLEAN
}
bool ui_sys_fax_settings_volume_ring_set(const fax_volume_setting_t volume)
{
    RETURN_FALSE_IF_FSM_NOT_READY

    error_type_t result = fax_app_set_var_volume_ring(volume);

    RETURN_BOOLEAN
}

bool ui_sys_fax_settings_recv_action_get(fax_recv_action_t * const action)
{
    RETURN_FALSE_IF_FSM_NOT_READY

    error_type_t result = fax_app_get_var_recv_action(action);

    RETURN_BOOLEAN
}
bool ui_sys_fax_fettings_recv_action_set(const fax_recv_action_t action)
{
    RETURN_FALSE_IF_FSM_NOT_READY

    error_type_t result = fax_app_set_var_recv_action(action);

    RETURN_BOOLEAN
}

bool ui_sys_fax_settings_recv_answer_mode_get(fax_ans_mode_t * const ans_mode)
{
    RETURN_FALSE_IF_FSM_NOT_READY

    uint32_t answer;
    faxer_config_get_setting(fax_service_handle, FAXER_SETTING_INT_ANSWER_MODE, &answer, sizeof(answer));

    ASSERT(answer >= e_FAX_ANS_MANUAL && answer <= e_FAX_ANS_FAXTEL);

    if (answer >= e_FAX_ANS_MANUAL && answer <= e_FAX_ANS_FAXTEL)
    {
        *ans_mode = (fax_ans_mode_t)answer;
        return  true
    }
    else
    {
        return  false;
    }
}
bool ui_sys_fax_settings_recv_answer_mode_set(const fax_ans_mode_t ans_mode)
{
    RETURN_FALSE_IF_FSM_NOT_READY

    uint32_t answer = ans_mode;

    faxer_config_set_setting(fax_service_handle, FAXER_SETTING_INT_ANSWER_MODE, &ans_mode);

    return  true;
}

void ui_sys_fax_settings_recv_rings_to_answer_get(uint32_t * rings_to_answer)
{
    ABORT_IF_FSM_NOT_READY

    faxer_config_get_setting(fax_service_handle, FAXER_SETTING_INT_RINGS_TO_ANSWER, rings_to_answer, sizeof(*rings_to_answer));
}

void ui_sys_fax_settings_recv_rings_to_answer_set(uint32_t rings_to_answer)
{
    ABORT_IF_FSM_NOT_READY

    faxer_config_set_setting(fax_service_handle, FAXER_SETTING_INT_RINGS_TO_ANSWER, &rings_to_answer);
}

bool ui_sys_fax_settings_recv_ring_detect_get(fax_distinct_ring_t * const ring_type)
{
    RETURN_FALSE_IF_FSM_NOT_READY

    faxer_config_get_setting(fax_service_handle, FAXER_SETTING_INT_RING_PATTERN, ring_type, sizeof(*ring_type));

    ASSERT(*ring_type >= e_FAX_RING_ALL && *ring_type <= e_FAX_RING_DOUBLE_TRIPLE);

    return true;
}

bool ui_sys_fax_settings_recv_ring_detect_set(const fax_distinct_ring_t ring_type)
{
    RETURN_FALSE_IF_FSM_NOT_READY

    uint32_t ring = ring_type;

    faxer_config_set_setting(fax_service_handle, FAXER_SETTING_INT_RING_PATTERN, &ring);

    return  true;
}

bool ui_sys_fax_settings_recv_caller_id_detect_get(fax_caller_id_t * const cid_type)
{
    RETURN_FALSE_IF_FSM_NOT_READY

    uint32_t cid;
    faxer_config_get_setting(fax_service_handle, FAXER_SETTING_CALLER_ID_TYPE, &cid, sizeof(cid));

    *cid_type = cid;

    return true;
}

bool ui_sys_fax_settings_recv_caller_id_detect_set(const fax_caller_id_t cid_type)
{
    RETURN_FALSE_IF_FSM_NOT_READY

    uint32_t cid = cid_type;

    faxer_config_set_setting(fax_service_handle, FAXER_SETTING_CALLER_ID_TYPE, &cid);

    return  true;
}

/**
 *   buf_size must >= FAXER_MAX_TEL_DIGITS + 1
 *
 */
void ui_sys_fax_settings_recv_forward_dest_get(char * forwardDest, const uint32_t buf_size)
{
    ABORT_IF_FSM_NOT_READY

    faxer_config_get_setting(fax_service_handle, FAXER_SETTING_CHR_FORWARD_NUMBER, forwardDest, buf_size);
}

void ui_sys_fax_settings_recv_forward_dest_set(char * forwardDest)
{
    ABORT_IF_FSM_NOT_READY

    faxer_config_set_setting(fax_service_handle, FAXER_SETTING_CHR_FORWARD_NUMBER, forwardDest);
}

bool ui_sys_fax_settings_recv_stamp_fax_get(void)
{
    RETURN_FALSE_IF_FSM_NOT_READY

    bool stamp;

    fax_app_get_var_stamp_fax(&stamp);

    return  stamp;
}

void ui_sys_fax_settings_recv_stamp_fax_set(const bool stamp)
{
    ABORT_IF_FSM_NOT_READY

    fax_app_set_var_stamp_fax(&stamp);
}

bool ui_sys_fax_settings_recv_fit_to_page_get(void)
{
    RETURN_FALSE_IF_FSM_NOT_READY

    bool fit_to_page;

    fax_app_get_var_fit_page(&fit_to_page);

    return fit_to_page;
}

void ui_sys_fax_settings_recv_fit_to_page_set(const bool fit_to_page)
{
    ABORT_IF_FSM_NOT_READY

    fax_app_set_var_fit_page(&fit_to_page);
}

bool ui_sys_fax_settings_recv_faxtel_ringtime_get(uint32_t * const ring_time)
{
    RETURN_FALSE_IF_FSM_NOT_READY

    error_type_t result = fax_app_get_var_faxtel_ringtime(ring_time);

    RETURN_BOOLEAN
}

bool ui_sys_fax_settings_recv_faxtel_ringtime_set(const uint32_t ring_time)
{
    RETURN_FALSE_IF_FSM_NOT_READY

    error_type_t result = fax_app_set_var_faxtel_ringtime(&ring_time);

    RETURN_BOOLEAN
}

bool ui_sys_fax_settings_recv_block_junkfaxes_get(bool * const block_junk)
{
    RETURN_FALSE_IF_FSM_NOT_READY

    error_type_t result = fax_app_get_var_block_junkfax(block_junk);

    RETURN_BOOLEAN
}

bool ui_sys_fax_settings_recv_block_junkfaxes_set(const bool block_junk)
{
    RETURN_FALSE_IF_FSM_NOT_READY

    error_type_t result = fax_app_set_var_block_junkfax(&block_junk);

    RETURN_BOOLEAN
}


bool ui_sys_fax_settings_send_scan_resolution_get(fax_resolution_t * const resolution)
{
    RETURN_NULL_IF_FSM_NOT_READY

    error_type_t result = fax_app_get_var_scan_resolution(resolution);

    RETURN_BOOLEAN
}

bool ui_sys_fax_settings_send_scan_resolution_set(const fax_resolution_t resolution_str)
{
    RETURN_NULL_IF_FSM_NOT_READY

    error_type_t result = fax_app_set_var_scan_resolution(&resolution);

    RETURN_BOOLEAN
}

bool ui_sys_fax_settings_send_scan_brightness_get(uint32_t * const brightness)
{
    RETURN_NULL_IF_FSM_NOT_READY

    error_type_t result = fax_app_get_var_scan_contrast(brightness);

    RETURN_BOOLEAN
}

bool ui_sys_fax_settings_send_scan_brightness_set(const uint32_t brightness)
{
    RETURN_NULL_IF_FSM_NOT_READY

    error_type_t result = fax_app_set_var_scan_contrast(&brightness);

    RETURN_BOOLEAN
}

char * ui_sys_fax_settings_send_scan_glass_size_get(void)
{

    RETURN_NULL_IF_FSM_NOT_READY

    char *result=NULL;
    fax_media_size_t glass_size;

    fax_app_get_var_glass_size(&glass_size);
    result = fax_enum_get_string_from_val(map_glass_size_settings, glass_size);

    return result;
}

void ui_sys_fax_settings_send_scan_glass_size_set(char * glass_str)
{
    ABORT_IF_FSM_NOT_READY

    uint32_t temp;

    if (fax_enum_get_val_from_string(map_glass_size_settings,glass_str, &temp) == OK)
        {
             fax_media_size_t glass_size = (fax_media_size_t)temp;
             fax_app_set_var_glass_size(&glass_size);
        }
}

void ui_sys_fax_settings_send_dial_mode_get(fax_dial_mode_t * const dial_mode)
{
    ABORT_IF_FSM_NOT_READY

    faxer_config_get_setting(fax_service_handle, FAXER_SETTING_INT_DIALING_MODE, dial_mode,sizeof(*dial_mode));

    return;
}

void ui_sys_fax_settings_send_dial_mode_set(const fax_dial_mode_t dial_mode)
{
    ABORT_IF_FSM_NOT_READY

    uint32_t dial = dial_mode;

    faxer_config_set_setting(fax_service_handle, FAXER_SETTING_INT_DIALING_MODE, &dial);

    return;
}

/**
 * dialPrefix (Numeric string, no longer than
 * FAXER_MAX_TEL_DIGITS digits)
 *
 **/

void ui_sys_fax_Settings_Send_prefix_get(char * const dial_prefix, size_t buf_size)
{
    ABORT_IF_FSM_NOT_READY

    ASSERT(dial_prefix != NULL);
    ASSERT(buf_size > FAXER_MAX_TEL_DIGITS + 1);

    faxer_config_get_setting(fax_service_handle, FAXER_SETTING_CHR_PREFIX, dial_prefix, buf_size);
    dial_prefix[FAXER_MAX_TEL_DIGITS] = '\0';

    return;
}

void ui_sys_fax_settings_send_prefix_set(const char *dial_prefix)
{
    ABORT_IF_FSM_NOT_READY

    char prefix[FAXER_MAX_TEL_DIGITS + 1];
    bzero(prefix, FAXER_MAX_TEL_DIGITS + 1);

    // Guarantee length limit and null termination
    strncpy(prefix, dial_prefix, FAXER_MAX_TEL_DIGITS);
    prefix[FAXER_MAX_TEL_DIGITS] = '\0';
    faxer_config_set_setting(fax_service_handle, FAXER_SETTING_CHR_PREFIX, prefix);

    return;
}

void ui_sys_fax_settings_send_prefix_enabled_get(bool * const prefix_enabled)
{
    ABORT_IF_FSM_NOT_READY

    uint32_t enabled;
    faxer_config_get_setting(fax_service_handle, FAXER_SETTING_SW_DIAL_PREFIX, &enabled, sizeof(enabled));

    *prefix_enabled = prefix_enabled != 0 ? true : false;

    return;
}

void ui_sys_fax_settings_send_prefix_enabled_set(const bool prefix_enabled)
{
    ABORT_IF_FSM_NOT_READY

    uint32_t temp = (prefix_enabled == true ? 1 : 0);

    faxer_config_set_setting(fax_service_handle, FAXER_SETTING_SW_DIAL_PREFIX, &temp);
}


void ui_sys_fax_settings_send_redial_if_busy_get(bool * const redial)
{
    ABORT_IF_FSM_NOT_READY

    uint32_t temp;

    faxer_config_get_setting(fax_service_handle, FAXER_SETTING_SW_BUSY_REDIAL, &temp, sizeof(temp));

    *redial = temp != 0 ? true : false;
}

void ui_sys_fax_settings_send_redial_if_busy_set(const bool redial)
{
    ABORT_IF_FSM_NOT_READY

    uint32_t temp = (redial == true ? 1 : 0);

    faxer_config_set_setting(fax_service_handle, FAXER_SETTING_SW_BUSY_REDIAL, &temp);
}


void ui_sys_fax_settings_send_redial_if_no_answer_get(bool * const redial)
{
    ABORT_IF_FSM_NOT_READY

    uint32_t temp;

    faxer_config_get_setting(fax_service_handle, FAXER_SETTING_SW_NOANS_REDIAL, &temp, sizeof(temp));

    *redial = temp != 0 ? true : false;
}

void ui_sys_fax_settings_send_redial_if_no_answer_set(const bool redial)
{
    ABORT_IF_FSM_NOT_READY

    uint32_t temp = redial == true ? 1 : 0;

    faxer_config_set_setting(fax_service_handle, FAXER_SETTING_SW_NOANS_REDIAL, &temp);
}


void ui_sys_fax_settings_send_redial_if_comm_error_get(bool const * redial)
{
    ABORT_IF_FSM_NOT_READY

    uint32_t temp;

    faxer_config_get_setting(fax_service_handle, FAXER_SETTING_SW_COMMERR_REDIAL, &temp, sizeof(temp));

    *redial = temp != 0 ? true : false;
}

void ui_sys_fax_settings_send_redial_if_comm_error_set(const bool redial)
{
    ABORT_IF_FSM_NOT_READY

    uint32_t temp = (redial == true ? 1 : 0);

    faxer_config_set_setting(fax_service_handle, FAXER_SETTING_SW_COMMERR_REDIAL, &temp);
}

bool ui_sys_fax_settings_send_require_dial_tone_get(bool * const require_dial_tone)
{
    ABORT_IF_FSM_NOT_READY

    uint32_t temp;

    faxer_config_get_setting(fax_service_handle, FAXER_SETTING_SW_DETECT_DIAL_TONE, &temp, sizeof(temp));

    *require_dial_tone = temp != 0 ? true : false;
}

void ui_sys_fax_settings_send_require_dial_tone_set(const bool require_dial_tone)
{
    ABORT_IF_FSM_NOT_READY

    uint32_t temp = (require_dial_tone == true ? 1 : 0);

    faxer_config_set_setting(fax_service_handle, FAXER_SETTING_SW_DETECT_DIAL_TONE, &temp);
}

/**
 * buf_size must >= FAXER_SETTING_CHR_HEADER_NAME + 1
 */
void ui_sys_fax_settings_header_name_get(char * const header_name, size_t buf_size)
{
    ABORT_IF_FSM_NOT_READY

    ASSERT(buf_size > FAXER_SETTING_CHR_HEADER_NAME + 1);

    faxer_config_get_setting(fax_service_handle, FAXER_SETTING_CHR_HEADER_NAME, header_name, buf_size);
    header_name[FAXER_MAX_HEADER_SIZE] = '\0';
}

void ui_sys_fax_settings_header_name_set(const char *header_name)
{
    ABORT_IF_FSM_NOT_READY

    char *header = (char *)MEM_CALLOC(FAXER_MAX_HEADER_SIZE + 1,sizeof(char));
    ASSERT(header != NULL);
    bzero(header, FAXER_MAX_HEADER_SIZE + 1);

    // Guarantee length limit and null termination
    strncpy(header, header_name, FAXER_MAX_HEADER_SIZE);
    header[FAXER_MAX_HEADER_SIZE] = '\0';

    faxer_config_set_setting(fax_service_handle, FAXER_SETTING_CHR_HEADER_NAME, header);

    MEM_FREE_AND_NULL(header);
}

/**
 *
 * @param header_number buffer for saving FAX header number
 * @param buf_size the size of header_number, must >=
 *                 FAXER_MAX_SID_SIZE + 1
 */
void ui_sys_fax_settings_header_number_get(char * const header_number, size_t buf_size)
{
    ABORT_IF_FSM_NOT_READY

    ASSERT(buf_size >= FAXER_MAX_SID_SIZE + 1);

    faxer_config_get_setting(fax_service_handle, FAXER_SETTING_CHR_SID, header_number, buf_size);

    return;
}

void ui_sys_fax_settings_header_number_set(const char *header_number)
{
    ABORT_IF_FSM_NOT_READY

    char *header = (char *)MEM_CALLOC(FAXER_MAX_SID_SIZE + 1,sizeof(char));
    ASSERT(header != NULL);

    // Guarantee length limit and null termination
    strncpy(header, header_number, FAXER_MAX_SID_SIZE);
    header[FAXER_MAX_SID_SIZE] = '\0';

    faxer_config_set_setting(fax_service_handle, FAXER_SETTING_CHR_SID, header);

    MEM_FREE_AND_NULL(header);
}

bool ui_sys_fax_settings_reports_add_thumbnail_get(bool * const addThumbnail)
{
    RETURN_FALSE_IF_FSM_NOT_READY

    ASSERT(addThumbnail != NULL);

    error_type_t result = fax_app_get_var_thumbnail_report(addThumbnail);

    RETURN_BOOLEAN
}

bool ui_sys_fax_settings_reports_add_thumbnail_set(const bool addThumbnail)
{
    RETURN_FALSE_IF_FSM_NOT_READY

    error_type_t result = fax_app_set_var_thumbnail_report(&addThumbnail);

    RETURN_BOOLEAN
}

bool ui_sys_fax_settings_reports_confirmation_get(fax_autoprint_confirm_t * const rpt_confirmation)
{
    RETURN_FALSE_IF_FSM_NOT_READY

    error_type_t result = fax_app_get_var_print_confirm(rpt_confirmation);

    RETURN_BOOLEAN

}

bool ui_sys_fax_settings_reports_confirmation_set(const fax_autoprint_confirm_t rpt_confirmation)
{
    RETURN_FALSE_IF_FSM_NOT_READY

    error_type_t result = fax_app_set_var_print_confirm(&rpt_confirmation);

    RETURN_BOOLEAN
}

bool ui_sys_fax_settings_reports_error_report_get(fax_autoprint_error_t * const rpt_error)
{
    RETURN_FALSE_IF_FSM_NOT_READY

    error_type_t result = fax_app_get_var_print_error(&rpt_error);

    RETURN_BOOLEAN
}

bool ui_sys_fax_settings_reports_error_report_set(const fax_autoprint_error_t rpt_error)
{
    RETURN_FALSE_IF_FSM_NOT_READY

    error_type_t result = fax_app_set_var_print_error(&rpt_error);

    RETURN_BOOLEAN
}

bool ui_sys_fax_settings_reports_activity_log_get(bool * const auto_print_log)
{
    RETURN_FALSE_IF_FSM_NOT_READY

    error_type_t result = fax_app_get_var_print_activity_report(auto_print_log);

    RETURN_BOOLEAN
}

bool ui_sys_fax_settings_reports_activity_log_set(const bool auto_print_log)
{
    RETURN_FALSE_IF_FSM_NOT_READY

    error_type_t result = fax_app_set_var_print_activity_report(&auto_print_log);

    RETURN_BOOLEAN
}

bool ui_sys_fax_settings_reports_t30_trace_get(fax_autoprint_t30_t * const rpt_t30)
{
    RETURN_FALSE_IF_FSM_NOT_READY

    error_type_t result = fax_app_get_var_print_t30_report(rpt_t30);

    RETURN_BOOLEAN
}

bool ui_sys_fax_settings_reports_t30_trace_set(const fax_autoprint_t30_t rpt_t30)
{
    RETURN_FALSE_IF_FSM_NOT_READY

    error_type_t result = fax_app_set_var_print_t30_report(&rpt_t30);

    RETURN_BOOLEAN
}

bool ui_sys_fax_settings_max_speed_get(ui_sys_fax_max_speed_t * const speed)
{
    RETURN_FALSE_IF_FSM_NOT_READY

    uint32_t max_speed;

    faxer_config_get_setting(fax_service_handle, FAXER_SETTING_INT_MAX_SPEED, &max_speed, sizeof(max_speed));

    switch (max_speed)
    {
    case 26:
        *speed = e_UI_SYS_FAX_FAST;
        break;
    case 0:
        *speed = e_UI_SYS_FAX_MEDIUM;
        break;
    case 6:
        *speed = e_UI_SYS_FAX_SLOW;
        break;
    default:
        return  false;
    }
    return  true;
}

bool ui_sys_fax_settings_max_speed_set(const ui_sys_fax_max_speed_t speed)
{
    RETURN_FALSE_IF_FSM_NOT_READY

    uint32_t max_speed;

    switch (speed)
    {
    case e_UI_SYS_FAX_FAST:
        max_speed = 16;
        break;
    case e_UI_SYS_FAX_MEDIUM:
        max_speed = 0;
        break;
    case e_UI_SYS_FAX_SLOW:
        max_speed = 6;
        break;
    default:
        ASSERT(false);
    }

    faxer_config_set_setting(fax_service_handle, FAXER_SETTING_INT_MAX_SPEED, &max_speed);

    return  true;
}

bool ui_sys_fax_settings_allow_ECM_get(bool * const allow_ecm)
{
    RETURN_FALSE_IF_FSM_NOT_READY

    uint32_t temp;

    faxer_config_get_setting(fax_service_handle, FAXER_SETTING_SW_ECM_MODE, &temp, sizeof(temp));

    *allow_ecm = temp != 0 ? true : false;

    return  true;
}

bool ui_sys_fax_settings_allow_ECM_set(const bool allow_ecm)
{
    RETURN_FALSE_IF_FSM_NOT_READY

    uint32_t temp = (allow_ecm == true ? 1 : 0);

    faxer_config_set_setting(fax_service_handle, FAXER_SETTING_SW_ECM_MODE, &temp);

    return  true;
}








