LOCAL_PATH := $(call my-dir)

#
#### libsmartar.so ####
#
include $(CLEAR_VARS)
LOCAL_MODULE := smartar
LOCAL_MODULE_FILENAME := libsmartar
LOCAL_SRC_FILES := ./smartar/libs/armeabi-v7a/libsmartar.so
include $(PREBUILT_SHARED_LIBRARY)

# IMPORTANT!!!
# to avoid implicit "unsigned char" in GCC ARM
__LOCAL_CFLAGS +=  -fsigned-char

#
#### libsample_api.so ####
#
include $(CLEAR_VARS)

LOCAL_MODULE := sample_api
LOCAL_CPP_EXTENSION := .cc
LOCAL_ARM_MODE := arm
LOCAL_CFLAGS += $(__LOCAL_CFLAGS)

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/src \
    $(LOCAL_PATH)/../../../mobile_common \
	$(LOCAL_PATH)/smartar/include

LOCAL_SRC_FILES += \
	../../../mobile_common/sample_common/sample_common.cc \
	../../../mobile_common/sample_common/sample_common_util.cc \
	../../../mobile_common/sample_full/sample_full.cc \
	src/sampleJni.cc

LOCAL_LDLIBS := \
	-llog \
	-lgcc\
	-lz \
	-lGLESv1_CM \
	-lGLESv2

LOCAL_SHARED_LIBRARIES := libsmartar

include $(BUILD_SHARED_LIBRARY)
