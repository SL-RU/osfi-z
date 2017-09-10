#             __________               __   ___.
#   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
#   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
#   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
#   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
#                     \/            \/     \/    \/            \/
# $Id$
#

# libwma
WMALIB := $(CODECDIR)/libwma.a
WMALIB_SRC := $(call preprocess, $(RBCODECLIB_DIR)/codecs/libwma/SOURCES)
WMALIB_OBJ := $(call c2obj, $(WMALIB_SRC))
OTHER_SRC += $(WMALIB_SRC)

$(WMALIB): $(WMALIB_OBJ)
	$(SILENT)$(shell rm -f $@)
	$(call PRINTS,AR $(@F))$(AR) rcs $@ $^ >/dev/null
