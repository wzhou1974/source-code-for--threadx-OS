# ============================================================================
# Copyright (c) 2013 Marvell Semiconductor, Inc. All Rights Reserved
#                      
#                         Marvell Confidential
# ============================================================================


#Include master environment
include $(BUILD_ROOT)/project.mk

INCFLAGS = -I./src -I./include -I$(BUILD_ROOT)/$(GLOBAL_INCLUDE)

INSTALL_SRC_DIR  = include
INSTALL_DEST_DIR = $(BUILD_ROOT)/$(GLOBAL_INCLUDE)
INSTALL_SRC      = ui_system_interface_api.h

SOURCE          =  ui_system_interface_status.c
SOURCE          += ui_system_interface_utils.c
ifdef HAVE_UI_SYSTEM_INTERFACE_TEST
    MACROS      += HAVE_UI_SYSTEM_INTERFACE_TEST
    SOURCE      += ui_system_interface_test.c
endif

ifdef HAVE_PRINT_SUPPORT
INSTALL_SRC     += ui_system_interface_print.h
SOURCE          += ui_system_interface_print.c
endif

ifdef HAVE_COPY_SUPPORT
INSTALL_SRC     += ui_system_interface_copy.h
SOURCE 		    += ui_system_interface_copy.c
endif

ifdef HAVE_PLATFORM
INSTALL_SRC     += ui_system_interface_platform.h
SOURCE          += ui_system_interface_platform.c
endif

ifdef HAVE_AUDIO
INSTALL_SRC     += ui_system_interface_audio.h
SOURCE 	        += ui_system_interface_audio.c
endif

ifdef HAVE_FAX_SUPPORT
INSTALL_SRC     += ui_system_interface_fax.h
SOURCE          += ui_system_interface_fax.c
endif

ifdef HAVE_POWER_MGR
INSTALL_SRC     += ui_system_interface_powermgr.h
SOURCE          += ui_system_interface_powermgr.c
endif


ifdef HAVE_COMMON_CONSUMABLES_API
INSTALL_SRC     += ui_system_interface_consumables.h	
SOURCE          += ui_system_interface_consumables.c
endif

ifdef HAVE_SCAN_SUPPORT
INSTALL_SRC     += ui_system_interface_scan.h
SOURCE 	        += ui_system_interface_scan.c
endif

ifdef HAVE_NETWORK
INSTALL_SRC     += ui_system_interface_network.h
SOURCE 	        += ui_system_interface_network.c	
endif

ifdef HAVE_WIRELESS
SOURCE          += ui_system_interface_wireless.c
endif

ifdef HAVE_ETHERNET
SOURCE 	        += ui_system_interface_wirednetwork.c
endif

ifdef HAVE_INTERNAL_PAGE
INSTALL_SRC     += ui_system_interface_ipage.h
SOURCE 	        += ui_system_interface_ipage.c
endif

MODULE_LIBPATH := $(foreach LIB, $(LIBPATH), $(BUILD_ROOT)/$(LIB))

include $(BUILD_ROOT)/stdtargets.mk

