/*
*********************************************************************************************************
*                                                    MELIS
*                                    the Easy Portable/Player Develop Kits
*                                                  File System
*
*                                    (c) Copyright 2011-2014, Sunny China
*                                             All Rights Reserved
*
* File    : dospart.h
* By      : Sunny
* Version : v1.0
* Date    : 2011-3-15
* Descript: dos partition system.
* Update  : date                auther      ver     notes
*           2011-3-15 15:04:00  Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#ifndef __DOSPART_H__
#define __DOSPART_H__

#include "part.h"
typedef struct __FSYS_DOSPART_P
{
    __u32               partaddr;        /* �������豸�еĵ�ַ   */
    __u32               partseccnt;      /* ������С             */
    __u32               partsecsize;     /* ����������С         */
} __fsys_dospart_p_t;


struct parsed_partitions *check_partition(__hdle bdev);

#endif  /* __DOSPART_H__ */

