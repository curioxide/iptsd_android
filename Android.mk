LOCAL_PATH := $(call my-dir)

IPTSD_CPPFLAGS :=  \
	-std=c++17 -Wall -Wextra -Wpedantic \
	-O3 -Wmissing-include-dirs \
	-Winit-self -Wimplicit-fallthrough -Wdouble-promotion \
	-Wconversion \
	-fexceptions -DSPDLOG_FMT_EXTERNAL

ifeq ($(PLATFORM_SDK_VERSION),30)
IPTSD_CPPFLAGS += -march=core-avx2
else
IPTSD_CPPFLAGS += -march=x86-64-v3
endif

IPTSD_SHARED_LIBRARIES := libinih-cpp libspdlog
IPTSD_STATIC_LIBRARIES := libc++fs fmtlib9
IPTSD_HEADER_LIBRARIES := libeigen inih_headers cli11 fmtlib9_headers\
						microsoft-gsl libinih-cpp_headers

#build iptsd-calibrate
include $(CLEAR_VARS)
LOCAL_CPPFLAGS := $(IPTSD_CPPFLAGS)
LOCAL_SRC_FILES := $(call all-cpp-files-under, src/apps/calibrate)
LOCAL_MODULE := iptsd-calibrate
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := $(IPTSD_SHARED_LIBRARIES)
LOCAL_STATIC_LIBRARIES := $(IPTSD_STATIC_LIBRARIES)
LOCAL_HEADER_LIBRARIES := $(IPTSD_HEADER_LIBRARIES)
LOCAL_C_INCLUDES:= $(LOCAL_PATH)/src
include $(BUILD_EXECUTABLE)

#build iptsd-check-device
include $(CLEAR_VARS)
LOCAL_CPPFLAGS := $(IPTSD_CPPFLAGS)
LOCAL_SRC_FILES := $(call all-cpp-files-under, src/apps/check-device)
LOCAL_MODULE := iptsd-check-device
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := $(IPTSD_SHARED_LIBRARIES)
LOCAL_STATIC_LIBRARIES := $(IPTSD_STATIC_LIBRARIES)
LOCAL_HEADER_LIBRARIES := $(IPTSD_HEADER_LIBRARIES)
LOCAL_C_INCLUDES:= $(LOCAL_PATH)/src
include $(BUILD_EXECUTABLE)

#build iptsd-dump
include $(CLEAR_VARS)
LOCAL_CPPFLAGS := $(IPTSD_CPPFLAGS)
LOCAL_SRC_FILES := $(call all-cpp-files-under, src/apps/dump)
LOCAL_MODULE := iptsd-dump
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := $(IPTSD_SHARED_LIBRARIES)
LOCAL_STATIC_LIBRARIES := $(IPTSD_STATIC_LIBRARIES)
LOCAL_HEADER_LIBRARIES := $(IPTSD_HEADER_LIBRARIES)
LOCAL_C_INCLUDES:= $(LOCAL_PATH)/src
include $(BUILD_EXECUTABLE)

#build iptsd-perf
include $(CLEAR_VARS)
LOCAL_CPPFLAGS := $(IPTSD_CPPFLAGS)
LOCAL_SRC_FILES := $(call all-cpp-files-under, src/apps/perf)
LOCAL_MODULE := iptsd-perf
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := $(IPTSD_SHARED_LIBRARIES)
LOCAL_STATIC_LIBRARIES := $(IPTSD_STATIC_LIBRARIES)
LOCAL_HEADER_LIBRARIES := $(IPTSD_HEADER_LIBRARIES)
LOCAL_C_INCLUDES:= $(LOCAL_PATH)/src
include $(BUILD_EXECUTABLE)

#build iptsd
include $(CLEAR_VARS)
LOCAL_CPPFLAGS := $(IPTSD_CPPFLAGS)
LOCAL_SRC_FILES := $(call all-cpp-files-under, src/apps/daemon)
LOCAL_MODULE := iptsd
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := $(IPTSD_SHARED_LIBRARIES)
LOCAL_STATIC_LIBRARIES := $(IPTSD_STATIC_LIBRARIES)
LOCAL_HEADER_LIBRARIES := $(IPTSD_HEADER_LIBRARIES)
LOCAL_C_INCLUDES:= $(LOCAL_PATH)/src
LOCAL_POST_INSTALL_CMD := $(hide) mkdir -p $(TARGET_OUT_VENDOR)/etc/ipts; \
						  rsync -av -l $(LOCAL_PATH)/etc/iptsd.conf $(TARGET_OUT_VENDOR)/etc; \
						  rsync -av -l $(LOCAL_PATH)/etc/presets/* $(TARGET_OUT_VENDOR)/etc/ipts

include $(BUILD_EXECUTABLE)

#copy iptsd-find-hidraw
include $(CLEAR_VARS)
LOCAL_MODULE := iptsd-find-hidraw
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES

LOCAL_SRC_FILES := etc/iptsd-find-hidraw
include $(BUILD_PREBUILT)
