#
# 1. Set the path and clear environment
# 	TARGET_PATH := $(call my-dir)
# 	include $(ENV_CLEAR)
#
# 2. Set the source files and headers files
#	TARGET_SRC := xxx_1.c xxx_2.c
#	TARGET_INc := xxx_1.h xxx_2.h
#
# 3. Set the output target
#	TARGET_MODULE := xxx
#
# 4. Include the main makefile
#	include $(BUILD_BIN)
#
# Before include the build makefile, you can set the compilaion
# flags, e.g. TARGET_ASFLAGS TARGET_CFLAGS TARGET_CPPFLAGS
#

TARGET_PATH:= $(call my-dir)

########################################
include $(ENV_CLEAR)

TARGET_SRC := sample_g2d.c

TARGET_INC := \
    $(TARGET_TOP)/system/include \
    $(TARGET_TOP)/system/public/include \
    $(TARGET_TOP)/middleware/include/utils \
    $(TARGET_TOP)/middleware/include/media \
    $(TARGET_TOP)/middleware/include \
    $(TARGET_TOP)/middleware/media/include/utils \
    $(TARGET_TOP)/middleware/media/LIBRARY/libcedarx/libcore/common/iniparser \
    $(TARGET_TOP)/middleware/sample/configfileparser \

TARGET_SHARED_LIB := \
    libsample_confparser \
	librt \
    libpthread \
    liblog \
    libcdx_common \

TARGET_STATIC_LIB := \
    libaw_mpp \
    libmedia_utils \
    libmuxers \
    libmp4_muxer \
    libffavutil \
    libFsWriter \
    libmp3_muxer \
    libaac_muxer \
    libraw_muxer \
    libmpeg2ts_muxer \
    libcedarxdemuxer \

TARGET_CFLAGS += -fPIC -Wall -Wno-unused-but-set-variable -ldl

#TARGET_LDFLAGS += -static

TARGET_MODULE := sample_g2d

include $(BUILD_BIN)

