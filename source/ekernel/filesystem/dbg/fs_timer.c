/*
*********************************************************************************************************
*                                                    MELIS
*                                    the Easy Portable/Player Develop Kits
*                                                File system module
*
*                                    (c) Copyright 2011-2014, Sunny China
*                                             All Rights Reserved
*
* File    : fsys_timer.c
* By      : Sunny
* Version : v1.0
* Date    : 2011-3-8
* Descript: timer for file system debug.
* Update  : date                auther      ver     notes
*           2011-3-8 14:42:59   Sunny       1.0     Create this file.
*********************************************************************************************************
*/
#include "fsys_debug.h"
#include "fs_timer.h"

#if     FSYS_DEBUG_ON

__hdle timer_hdle = NULL;

/*--------------------------- debug ------------------------*/
static struct timer_cfg_s timer_cfg = {0};
static timers_t     timers =
{
    12,
    {
        {diodevicetime,    0, 0, "dio time"},
        {bufdevicetime,    0, 0, "bio time"},
        {getblktime,       0, 0, "get buf time"},
        {hashtime,         0, 0, "dio hash time"},
        {memcpytime,       0, 0, "mem copy time"},
        {bmaptime,         0, 0, "bmap time"},
        {fatcachetime,     0, 0, "fat cache time"},
        {scanfattime,      0, 0, "fat table time"},
        {oneaccesstime,    0, 0, "one access time"},
        {shrinkcachetime,  0, 0, "shrink cache time"},
        {partiotime,       0, 0, "part io time"},
        {invalidtime,      0, 0, "unused item"},
    }
};

void debug_timerclr(ttype_t target)
{
    int i;

    if (!timer_cfg.on || !timer_hdle)
    {
        return ;
    }

    esTIME_PauseCntr(timer_hdle);
    if (target == alltime)
    {
        for (i = 0; i < timers.count; i++)
        {
            timers.item[i].value = 0;
        }
    }
    else
    {
        for (i = 0; i < timers.count; i++)
        {
            if (timers.item[i].type == target)
            {
                timers.item[i].value = 0;
                break;
            }
        }
        if (i == timers.count)
        {
            __inf("invalid fs debug timer: %d, please check it.", target);
        }
    }
    esTIME_ContiCntr(timer_hdle);
}

void debug_timerstart(ttype_t target)
{
    int i;
    __u32 temptime;

    if (!timer_cfg.on || !timer_hdle)
    {
        return ;
    }

    temptime = esTIME_QuerryCntr(timer_hdle) << 1;
    esTIME_PauseCntr(timer_hdle);
    for (i = 0; i < timers.count && target != alltime; i++)
    {
        if (timers.item[i].type == target)
        {
            timers.item[i].start = temptime;
            break;
        }
    }
    if (i == timers.count || target == alltime)
    {
        __inf("invalid fs debug timer: %d, please check it.", target);
    }
    esTIME_ContiCntr(timer_hdle);
}

__u32 debug_timerend(ttype_t target)
{
    int i;
    __u32 temptime;
    __u32 ret = 0;

    if (!timer_cfg.on || !timer_hdle)
    {
        return 0;
    }

    temptime = esTIME_QuerryCntr(timer_hdle) << 1;
    esTIME_PauseCntr(timer_hdle);

    for (i = 0; i < timers.count && target != alltime; i++)
    {
        if (timers.item[i].type == target)
        {
            timers.item[i].value += (temptime - timers.item[i].start);
            ret = timers.item[i].value;
            break;
        }
    }
    if (i == timers.count || target == alltime)
    {
        __inf("invalid fs debug timer: %d, please check it.", target);
    }
    esTIME_ContiCntr(timer_hdle);

    return ret;
}

void debug_timershow(ttype_t target)
{
    int i;

    if (!timer_cfg.on || !timer_hdle)
    {
        return;
    }

    esTIME_PauseCntr(timer_hdle);
    if (target == alltime)
    {
        for (i = 0; i < timers.count; i++)
        {
            __inf("[%s", timers.item[i].info);
            __inf(": %d]", timers.item[i].value);
        }
    }
    else
    {
        for (i = 0; i < timers.count; i++)
        {
            if (timers.item[i].type == target)
            {
                __inf("[%s", timers.item[i].info);
                __inf(": %d]", timers.item[i].value);
                break;
            }
        }
        if (i == timers.count)
        {
            __inf("invalid fs debug timer: %d, please check it.", target);
        }
    }
    esTIME_ContiCntr(timer_hdle);
}

static char printbuf[128];
void debug_timerprint(char *fmt, ...)
{
    va_list args;

    if (!timer_cfg.on || !timer_hdle)
    {
        return;
    }

    esTIME_PauseCntr(timer_hdle);

    va_start(args, fmt);
    vsprintf(printbuf, fmt, args);
    va_end(args);
    __inf("%s", printbuf);

    esTIME_ContiCntr(timer_hdle);
}

BOOL fs_timer_parse_option(const char *option)
{
    if (strcmp(option, "request") == 0 || strcmp(option, "REQUEST") == 0)
    {
        timer_hdle = esTIME_RequestCntr(NULL, "FS");
        if (!timer_hdle)
        {
            __inf("Request counter fail");
            return FALSE;
        }
        /* set timer prescale, refrence timer frequency. */
        esTIME_SetCntrPrescale(timer_hdle, 135);
        esTIME_SetCntrValue(timer_hdle, 0);
        esTIME_StartCntr(timer_hdle);
        return TRUE;
    }
    else if (strncmp(option, "0x", 2) == 0 || strncmp(option, "0X", 2) == 0)
    {
        timer_hdle = __strtol(option, NULL, 0);
        __inf("timer handle: %x", timer_hdle);
        return TRUE;
    }
    else if (strcmp(option, "release") == 0 || strcmp(option, "RELEASE") == 0)
    {
        if (timer_hdle)
        {
            esTIME_ReleaseCntr(timer_hdle);
            timer_hdle = NULL;
            return TRUE;
        }
        __inf("no timer to release!");
        return FALSE;
    }
    else if (strcmp(option, "on") == 0 || strcmp(option, "ON") == 0)
    {
        timer_cfg.on = 1;
        return TRUE;
    }
    else if (strcmp(option, "off") == 0 || strcmp(option, "OFF") == 0)
    {
        timer_cfg.on = 0;
        return TRUE;
    }
    __inf("Unknown fs timer option '%s'", option);
    return FALSE;
}

#endif  /* FSYS_DEBUG_ON */
