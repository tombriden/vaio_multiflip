LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := vaio_multiflip.c
LOCAL_SHARED_LIBS := libc
LOCAL_LDLIBS := -llog
 
LOCAL_CFLAGS := -g

LOCAL_MODULE := vaio_multiflip
LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_EXECUTABLES)
LOCAL_MODULE_TAGS := eng
include $(BUILD_EXECUTABLE)
