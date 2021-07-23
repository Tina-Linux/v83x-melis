/*
*********************************************************************************************************
*                                                    MELIS
*                                    the Easy Portable/Player Develop Kits
*                                                execute sub-system
*
*                                    (c) Copyright 2011-2014, Sunny China
*                                             All Rights Reserved
*
* File    : exec_i.h
* By      : Sunny
* Version : v1.0
* Date    : 2011-4-1
* Descript: execute sub-system.
* Update  : date                auther      ver     notes
*           2011-4-1 18:54:41   Sunny       1.0     Create this file.
*********************************************************************************************************
*/
#ifndef __EXEC_I_H__
#define __EXEC_I_H__
#include <typedef.h>
#include <kernel/kapi.h>
#include <common/romldr.h>
#include "loader.h"

#define OS_EVENT_TBL_SIZE       16

/*wait process main function to return  */
#define EXEC_CREATE_WAIT_RET    (1<<0)

typedef struct __KRNL_XCB
{
    __u8            id;                                 /* ��ϵͳΨ???��id???                                         */
    __u16           tcbtbl[OS_EVENT_TBL_SIZE];          /* ��¼��ǰ�Ľ��������???                                   */
    void           *heap;                               /* scb�ľֲ���                                              */
    char           *xfile;                          /* ��ǰ�ļ����ļ�·�����ļ�???                               */
    __resm_rsb_t   *them;                           /* ���Ӧ�ĵ�ǰ��Դ�ļ�                                     */
    __resm_rsb_t   *lang;                           /* ���Ӧ�ĵ�ǰ��Դ�ļ�                                     */
} __krnl_xcb_t;

typedef struct __EXEC_PCB       //���̿��ƿ�
{
    __krnl_xcb_t     xcb;
    __s32            ret;
    rt_sem_t         retsem;
} __exec_pcb_t;

typedef struct
{
    __s32(*main)(void *p_arg);
    void           *p_arg;
    __u8            id;
} __exec_mainthread_para_t;

#endif //__EXEC_I_H__
