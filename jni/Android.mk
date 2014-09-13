LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := deepsleep
LOCAL_SRC_FILES := deepsleep.c
LOCAL_CPPFLAGS := -std=gnu++0x -Wall -O2        # whatever g++ flags you like
LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog   # whatever ld flags you like
LOCAL_LDLIBS += -ldl 


include $(BUILD_EXECUTABLE)    # <-- Use this to build an executable.
