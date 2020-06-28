/*
 * ============================================================================
 * Copyright (c) 2011-2013   Marvell International, Ltd. All Rights Reserved
 *
 *                         Marvell Confidential
 * ============================================================================
 */

/** 
 * \file ui_system_interface_audio.h
 * 
 * \brief 
 * 
 * 
 **/

#ifndef __UI_SYSTEM_INTERFACE_AUDIO_H__
#define __UI_SYSTEM_INTERFACE_AUDIO_H__

#include <stdint.h>
#include <stdbool.h>

/** 
 *  @brief Registers a fixed frequency tone of given length.
 *         The tone can later be played by referring to it by
 *         the name provided.
 * 
 *  @param name Name of the tone provided by the caller. Later
 *      requests to play tones will use this name.
 *  
 *  @param frequency Frequency of the tone to play (hz).
 *  
 *  @param duration_in_ms How long the tone should last (millisecond).
 *  
 *  
 *  @return false if name is already in use.  true otherwise.
 * 
 */
bool ui_sys_audio_createtone(const char *name, uint16_t frequency, uint16_t duration_in_ms);

/** 
 *  @brief Request that a tone be played.
 * 
 *  @param name Name of the tone to be played.  This
 *      tone should have been registered previously with the
 *      system using one of the audio_mgr_create calls.
 *  
 *  @param volume_percent How loud to play the tone (0-100).
 *  
 *  @param discard_if_busy If another tone is playing, new tones
 *                         will be queued.  If you tone won't
 *                         make sense when played later
 *                         (keybeeps, for example), set this to
 *                         true.  Instead of being queued, the
 *                         request will simply be ignored.
 *  
 *  @return false if name is already in use.  true otherwise.
 *         
 * 
 */
bool ui_sys_audio_playtone(const char *name, uint8_t volume_percent, bool discard_if_busy);


#endif // ifndef __UI_SYSTEM_INTERFACE_AUDIO_H__

