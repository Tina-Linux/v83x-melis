
/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : log.h
* Description :
* History :
*   Author  : xyliu <xyliu@allwinnertech.com>
*   Date    : 2015/04/13
*   Comment :
*
*
*/
#ifndef _CDC_LOG_H_
#define _CDC_LOG_H_

#include <cdc_config.h>

#ifndef LOG_TAG
#define LOG_TAG "cedarc"
#endif

#ifdef __ANDROID__
    #ifdef CONF_OREO_AND_NEWER
    #include <log/log.h>
    #else
    #include <cutils/log.h>
    #endif

    #define LOG_LEVEL_ERROR     ANDROID_LOG_ERROR
    #define LOG_LEVEL_WARNING   ANDROID_LOG_WARN
    #define LOG_LEVEL_INFO      ANDROID_LOG_INFO
    #define LOG_LEVEL_VERBOSE   ANDROID_LOG_VERBOSE
    #define LOG_LEVEL_DEBUG     ANDROID_LOG_DEBUG

    #define AWLOG(level, fmt, arg...)  \
        LOG_PRI(level, LOG_TAG, "<%s:%u>: " fmt, __FUNCTION__, __LINE__, ##arg)

#define CC_LOG_ASSERT(e, fmt, arg...)                               \
        LOG_ALWAYS_FATAL_IF(                                        \
                !(e),                                               \
                "<%s:%d>check (%s) failed:" fmt,                    \
                __FUNCTION__, __LINE__, #e, ##arg)                  \

#else

#include <stdio.h>
#include <string.h>

#define LOG_LEVEL_ERROR     "error  "
#define LOG_LEVEL_WARNING   "warning"
#define LOG_LEVEL_INFO      "info   "
#define LOG_LEVEL_VERBOSE   "verbose"
#define LOG_LEVEL_DEBUG     "debug  "

#ifdef CONFIG_MOD_CEDAR_ENABLE
#include <libc.h>

#define AWLOG(level, fmt, arg...)  \
    eLIBs_printf("%s: %s <%s:%u>: "fmt"\n\r", level, LOG_TAG, __FUNCTION__, __LINE__, ##arg)

#define CC_LOG_ASSERT(e, fmt, arg...)                                       \
                do {                                                        \
                    if (!(e))                                               \
                    {                                                       \
                        loge("check (%s) failed:"fmt, #e, ##arg);           \
                    }                                                       \
                } while (0)
#else
#define AWLOG(level, fmt, arg...)  \
    printf("%s: %s <%s:%u>: "fmt"\n", level, LOG_TAG, __FUNCTION__, __LINE__, ##arg)

#define CC_LOG_ASSERT(e, fmt, arg...)                                       \
                do {                                                        \
                    if (!(e))                                               \
                    {                                                       \
                        loge("check (%s) failed:"fmt, #e, ##arg);           \
                        assert(0);                                          \
                    }                                                       \
                } while (0)
#endif

#endif

#if CONFIG_LOG_LEVEL == OPTION_LOG_LEVEL_CLOSE

#define loge(fmt, arg...)
#define logw(fmt, arg...)
#define logi(fmt, arg...)
#define logd(fmt, arg...)
#define logv(fmt, arg...)

#elif CONFIG_LOG_LEVEL == OPTION_LOG_LEVEL_ERROR

#define loge(fmt, arg...) AWLOG(LOG_LEVEL_ERROR, "\033[40;31m" fmt "\033[0m", ##arg)
#define logw(fmt, arg...)
#define logi(fmt, arg...)
#define logd(fmt, arg...)
#define logv(fmt, arg...)

#elif CONFIG_LOG_LEVEL == OPTION_LOG_LEVEL_WARNING

#define loge(fmt, arg...) AWLOG(LOG_LEVEL_ERROR, "\033[40;31m" fmt "\033[0m", ##arg)
#define logw(fmt, arg...) AWLOG(LOG_LEVEL_WARNING, fmt, ##arg)
#define logi(fmt, arg...)
#define logd(fmt, arg...)
#define logv(fmt, arg...)

#elif CONFIG_LOG_LEVEL == OPTION_LOG_LEVEL_DEFAULT

#define loge(fmt, arg...) AWLOG(LOG_LEVEL_ERROR, "\033[40;31m" fmt "\033[0m", ##arg)
#define logw(fmt, arg...) AWLOG(LOG_LEVEL_WARNING, fmt, ##arg)
#define logi(fmt, arg...) AWLOG(LOG_LEVEL_INFO, fmt, ##arg)
#define logd(fmt, arg...) AWLOG(LOG_LEVEL_DEBUG, fmt, ##arg)
#define logv(fmt, arg...)

#elif CONFIG_LOG_LEVEL == OPTION_LOG_LEVEL_DETAIL

#define loge(fmt, arg...) AWLOG(LOG_LEVEL_ERROR, "\033[40;31m" fmt "\033[0m", ##arg)
#define logw(fmt, arg...) AWLOG(LOG_LEVEL_WARNING, fmt, ##arg)
#define logi(fmt, arg...) AWLOG(LOG_LEVEL_INFO, fmt, ##arg)
#define logd(fmt, arg...) AWLOG(LOG_LEVEL_DEBUG, fmt, ##arg)
#define logv(fmt, arg...) AWLOG(LOG_LEVEL_VERBOSE, fmt, ##arg)

#else
    #error "invalid configuration of debug level."
#endif

#define CEDARC_UNUSE(param) (void)param  //just for remove compile warning

//#define CEDARC_DEBUG (0)

#endif

