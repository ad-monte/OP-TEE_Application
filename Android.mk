###################### optee-hello-world ######################
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_CFLAGS += -DANDROID_BUILD
LOCAL_CFLAGS += -Wall

LOCAL_SRC_FILES += host/main.c

LOCAL_C_INCLUDES := $(LOCAL_PATH)/ta/include

LOCAL_SHARED_LIBRARIES := libteec
LOCAL_MODULE := my_ta
LOCAL_VENDOR_MODULE := true
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)


# Second client application
include $(CLEAR_VARS)
LOCAL_CFLAGS += -DANDROID_BUILD
LOCAL_CFLAGS += -Wall
LOCAL_SRC_FILES += host/test_suite.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/ta/include
LOCAL_SHARED_LIBRARIES := libteec
LOCAL_MODULE := test_suite
LOCAL_VENDOR_MODULE := true
LOCAL_MODULE_TAGS := optional

# include always the TA (to have it built before the client app)
include $(LOCAL_PATH)/ta/Android.mk
