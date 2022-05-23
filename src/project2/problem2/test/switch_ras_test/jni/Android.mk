LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_SRC_FILES := switch_ras_test.c
# your source code
LOCAL_MODULE := switch_ras_test
# output file name
LOCAL_CFLAGS += -pie -fPIE
# These two line cannot be
LOCAL_LDFLAGS += -pie -fPIE
# changed.
LOCAL_FORCE_STATIC_EXECUTABLE := true
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := loop.c
# your source code
LOCAL_MODULE := loop
# output file name
LOCAL_CFLAGS += -pie -fPIE
# These two line cannot be
LOCAL_LDFLAGS += -pie -fPIE
# changed.
LOCAL_FORCE_STATIC_EXECUTABLE := true
include $(BUILD_EXECUTABLE)