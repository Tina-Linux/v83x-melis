/*
*********************************************************************************************************
*                                                    eMOD
*                                   the Easy Portable/Player Operation System
*                                              mod_mmp sub-system
*
*                                    (c) Copyright 2008-2009, Kevin.Z China
*                                              All Rights Reserved
*
* File   : cache_fs.h
* Version: V1.0
* By     : kevin.z
* Date   : 2009-2-2 8:37
    cache��ϵ���ļ�������API.
    cache�ڲ�����ģ�鶼�������API�����ļ�������ֱ�ӵ���ϵͳ��fopen�Ⱥ�����

    ǿ�ƹ涨:
    �����ʹ��cache_fs.h�ṩ�ĺ�����������ģ���ʱ�ȵ���
    CACHE_InitHeap();
    CACHE_InitPhyHeap();
    ��ʼ��cache_common���ڴ������.��Ϊcache_fs�ĺ���Ĭ��ʹ��cache_common�ṩ��
    �ڴ��������.
* Modify : zengzhijin 2020-04-24
*********************************************************************************************************
*/
#ifndef _CACHE_FS_H_
#define _CACHE_FS_H_

#include <sys/types.h>
#include <libc.h>

typedef enum tag_CACHE_FS_WORK_MODE
{
    CACHE_FS_WORK_MODE_BYTEALIGN    = 0,   // read align by byte
    CACHE_FS_WORK_MODE_SECTALIGN    = 1,   // read align by sector
    /* user defined read,�û��Լ����������ȡ�ļ�.ĿǰĬ��no cache.*/
    /* �����������������Ӳ���ϵ��ļ������ͣ�������ڴ��ȡ�ȣ����������ļ��������ļ������������ļ�,��Ҫ��fopen()�Ⱥ��� */
    CACHE_FS_WORK_MODE_USER_DEFINED = 2,
    CACHE_FS_WORK_MODE_NO_CACHE     = 3,   // use fread of fs��no cache
    CACHE_FS_WORK_MODE_HERBCACHE    = 4,   //�ڲ����߳�,��Ҫ����¼��д.
} CacheFSWorkMode;

extern ES_FILE     *CACHE_fopen(const char *path, const char *mode, __u32 flags);
extern __s32    CACHE_fstat(ES_FILE *fp, void *buf);
extern __s32    CACHE_fsync(ES_FILE *fp);
extern __s32    CACHE_ftruncate(ES_FILE *fp, off_t length);
extern int      CACHE_fclose(ES_FILE *fp);
extern int      CACHE_fread(void *buf, int size, int count, ES_FILE *fp);
extern int      CACHE_fwrite(void *buf, int size, int count, ES_FILE *fp);
extern int      CACHE_fseek(ES_FILE *fp, __s64 offset, int origin);
extern __s64    CACHE_ftell(ES_FILE *fp);
extern int      CACHE_fioctrl(ES_FILE *fp, __s32 Cmd, __s32 Aux, void *pBuffer);

/* ����filesize�ĺ��� */
extern __u32    CACHE_fsize(ES_FILE *fp);
/* ���º�������cachefs�����ԣ�������newCacheFs֮ǰ(������CACHE_fopen֮ǰ)ȫ�����úá�����CacheFS֮���ٵ�����3�������������� */
extern __s32    CACHE_fs_set_workmod(__s32 mode);
extern __s32    CACHE_fs_get_workmod(void);

/* ������mode = CACHE_FS_WORK_MODE_SECTALIGN����cache��ģʽ */
extern int      CACHE_fs_SetCacheSize(int cache_size, int mode);

/* ��Ϊfs buffer modeҪ���̣߳� epdkϵͳ�£����̵߳ø�һ��module id. */
extern void     CACHE_fs_set_mid(__u32 mid);

/* �����USRDEFģʽ��������__cache_usr_file_op_t */
/* ������CACHE_FS_WORK_MODE_USER_DEFINED����ģʽ�����úñ�Ҫ�����ݽṹfop = __cache_usr_file_op_t g_cache_usr_fop;�ڴ��ļ�ʱʹ�� */
extern void     CACHE_ufinit(void *fop);

#endif //_CACHE_FS_H_

