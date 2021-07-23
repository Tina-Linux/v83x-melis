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
* File   : cache_fs_base.h
* Version: V1.0
* By     : Eric_wang
* Date   : 2011-2-18
* Description:
    ��Ϊ�ڲ����õ�ͷ�ļ�
    ��Ϊ����ʹ��
    �����г�ʼ�������ﲻ�ṩ��ʼ������
* Modify : zengzhijin 2020-04-24
********************************************************************************
*/
#ifndef _CACHE_FS_BASE_H_
#define _CACHE_FS_BASE_H_

typedef struct tag_CACHEFS CacheFS;

typedef ES_FILE *(*CacheFS_fopen)(CacheFS *thiz, const __s8 *path, const __s8 *mode);   //���ļ�,���ص����������ļ�ָ��
typedef __s32(*CacheFS_fclose)(CacheFS *thiz);      //�ر��ļ�
typedef __s32(*CacheFS_fread)(CacheFS *thiz, void *buf, __s32 size, __s32 count);      //���ļ�
typedef __s32(*CacheFS_fwrite)(CacheFS *thiz, void *buf, __s32 size, __s32 count);      //д�ļ�
typedef __s32(*CacheFS_fseek)(CacheFS *thiz, __s64 offset, __s32 origin);
typedef __s64(*CacheFS_ftell)(CacheFS *thiz);
typedef __s32(*CacheFS_fstat)(CacheFS *thiz, void *stat_buf);
typedef __s32(*CacheFS_fsync)(CacheFS *thiz);
typedef __s32(*CacheFS_ftruncate)(CacheFS *thiz, off_t offset);
typedef __s32(*CacheFS_SetCacheSize)(CacheFS *thiz, __s32 cache_size);      //������mode = CACHE_FS_WORK_MODE_SECTALIGN����cache��ģʽ
typedef ES_FILE   *(*CacheFS_GetFp)(CacheFS *thiz);   //�õ��������ļ�ָ��

/*******************************************************************************
Function name: tag_CACHEFS
Members:
    ����󲿷ֺ����ӿڶ������麯������Ҳ��Щcallback��������, ������Ҫ��ʼ������.
    ��Ϊ��ʼ������û��malloc�ڴ棬���Բ���Ҫexit����

*******************************************************************************/
typedef struct tag_CACHEFS  //��¼cachefs����ģʽ����ز�����struct
{
    __s32       cache_fs_work_mode; //CACHE_FS_WORK_MODE_SECTALIGN;����debug

    //CACHE_FS_WORK_MODE_SECTALIGN�ȴ������fs���ƣ���������malloc�ڴ�
    CacheFS_fopen           cachefs_fopen;
    CacheFS_fclose          cachefs_fclose;
    CacheFS_fread           cachefs_fread;
    CacheFS_fwrite          cachefs_fwrite;
    CacheFS_fseek           cachefs_fseek;
    CacheFS_ftell           cachefs_ftell;
    CacheFS_SetCacheSize    SetCacheSize;   //һ��Ҫ����fopen֮ǰ���ã�����������
    CacheFS_GetFp           GetFp;          //�õ��������ļ�ָ��.
    CacheFS_fstat           cachefs_fstat;
    CacheFS_fsync           cachefs_fsync;
    CacheFS_ftruncate       cachefs_ftruncate;
} CacheFS;

extern __s32 CacheFS_Init(CacheFS *thiz);

#endif  /* _CACHE_FS_BASE_H_ */
