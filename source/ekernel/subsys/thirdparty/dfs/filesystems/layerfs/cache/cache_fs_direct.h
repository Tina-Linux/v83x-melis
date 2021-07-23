/*
********************************************************************************
*                                    eMOD
*                   the Easy Portable/Player Develop Kits
*                               mod_cache sub-system
*                          (module name, e.g.mpeg4 decoder plug-in) module
*
*          (c) Copyright 2010-2012, Allwinner Microelectronic Co., Ltd.
*                              All Rights Reserved
*
* File   : cache_fs_direct.h
* Version: V1.0
* By     : Eric_wang
* Date   : 2011-2-18
* Modify : zengzhijin 2020-04-24
* Description:
    ֱ��ʹ��ϵͳ�ṩ���ļ�������������
********************************************************************************
*/
#ifndef _CACHE_FS_DIRECT_H_
#define _CACHE_FS_DIRECT_H_

#include "cache_fs_base.h"

/* ��¼cachefs����ģʽ����ز�����struct, byte align,�ֽڶ��� */
typedef struct tag_CACHEFSDIRECT
{
    CacheFS CacheFSBase;
    ES_FILE    *fp;
} CacheFSDirect;

extern CacheFSDirect *newCacheFSDirect(void);
extern void deleteCacheFSDirect(CacheFSDirect *pCacheFSDirect);

#endif  /* _CACHE_FS_DIRECT_H_ */


