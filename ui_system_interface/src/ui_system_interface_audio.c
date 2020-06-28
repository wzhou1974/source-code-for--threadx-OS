/*
 * ============================================================================
 * Copyright (c) 2013   Marvell International, Ltd. All Rights Reserved
 *
 *                         Marvell Confidential
 * ============================================================================
 */

/** 
 * \file ui_system_interface_audio.c
 * 
 * \brief Implementations of audio part of ui_system_interface
 *        
 * 
 **/



#include "ui_system_interface_audio.h"
#include "audio_mgr_api.h"


bool ui_sys_audio_createtone(const char *name, uint16_t frequency, uint16_t duration_in_ms)
{
	error_type_t result;
    result = audio_mgr_create_tone(name, frequency, duration_in_ms);
	return result == OK ? true : false;	
}

bool ui_sys_audio_playtone(const char *name, uint8_t volume_percent, bool discard_if_busy)
{	
    tone_instance result;

    result = audio_mgr_play_sound(name, volume_percent, discard_if_busy);
	return result != NULL ? true : false; 	
}

