#             __________               __   ___.
#   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
#   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
#   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
#   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
#                     \/            \/     \/    \/            \/
# $Id$
#

# libfaad
FAADLIB := $(CODECDIR)/libfaad.a
FAADLIB_SRC := $(call preprocess, $(RBCODECLIB_DIR)/codecs/libfaad/SOURCES)
FAADLIB_OBJ := $(call c2obj, $(FAADLIB_SRC))
OTHER_SRC += $(FAADLIB_SRC)
OTHER_INC += -I$(RBCODECLIB_DIR)/codecs/libfaad

$(FAADLIB): $(FAADLIB_OBJ)
	$(SILENT)$(shell rm -f $@)
	$(call PRINTS,AR $(@F))$(AR) rcs $@ $^ >/dev/null
