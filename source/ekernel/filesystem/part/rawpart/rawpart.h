/*
*********************************************************************************************************
*                                                    MELIS
*                                    the Easy Portable/Player Develop Kits
*                                                  File System
*
*                                    (c) Copyright 2011-2014, Sunny China
*                                             All Rights Reserved
*
* File    : rawpart.h
* By      : Sunny
* Version : v1.0
* Date    : 2011-3-15
* Descript: raw partition system.
* Update  : date                auther      ver     notes
*           2011-3-15 15:02:30  Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#ifndef __RAWPART_H__
#define __RAWPART_H__

#include "part.h"

typedef struct __FSYS_RAWPART_P
{
    __u32                partaddr;      /* �������豸�еĵ�ַ   */
    __u32                partseccnt;    /* ����������Ŀ��С     */
    __u32                partsecsize;   /* ����������С         */
} __fsys_rawpart_p_t;

#endif  /* __RAWPART_H__ */
