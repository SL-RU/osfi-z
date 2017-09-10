#             __________               __   ___.
#   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
#   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
#   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
#   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
#                     \/            \/     \/    \/            \/
# $Id$
#

# liba52
A52LIB := $(CODECDIR)/liba52.a
A52LIB_SRC := $(call preprocess, $(RBCODECLIB_DIR)/codecs/liba52/SOURCES)
A52LIB_OBJ := $(call c2obj, $(A52LIB_SRC))
OTHER_SRC += $(A52LIB_SRC)

$(A52LIB): $(A52LIB_OBJ)
	$(SILENT)$(shell rm -f $@)
	$(call PRINTS,AR $(@F))$(AR) rcs $@ $^ >/dev/null
