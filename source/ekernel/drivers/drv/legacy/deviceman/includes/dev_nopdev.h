/*
*********************************************************************************************************
*                                                   ePOS
*                               the Easy Portable/Player Operation System
*                                          module sub-system
*
*                               (c) Copyright 2006-2007, Steven.ZGJ China
*                                           All Rights Reserved
*
* File   : dev_ramdisk.h
* Version: V1.0
* By     : steven.zgj
*********************************************************************************************************
*/
#ifndef _DEV_NOPDEV_H_
#define _DEV_NOPDEV_H_

extern __dev_devop_t nopdev_ops;

__s32 nopdev_init(void);
__s32 nopdev_exit(void);

#endif //#ifndef _DEV_NOPDEV_H_

