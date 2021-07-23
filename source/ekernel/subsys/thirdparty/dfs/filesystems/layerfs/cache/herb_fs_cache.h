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
* File   : herb_fs_cache.h
* Version: V1.0
* By     : Eric_wang
* Date   : 2011-3-6
* Modify : zengzhijin 2020-04-24
* Description:
    ��������Ƶ�herbfs,д��buffer�У����ɵ������̸߳����bufferд��file system.
    �������д,�������Լ����û���,���಻���𲢷�д�Ļ���.
********************************************************************************
*/
#ifndef _HERB_FS_CACHE_H_
#define _HERB_FS_CACHE_H_
#include "cache_fs_base.h"
#include "cache_lock.h"
#include <semaphore.h>

/* ����д���ļ�ϵͳ��������������ȼ����Ա�֤������ɵ������ܼ�ʱд�룬������ʹABS, VBS��������� */
#define WRITE_ES_FILESYS_TASK_PRIO     KRNL_priolevel1

#define HERB_FS_CACHE_SIZE (6*1024*1024)    //cache size��Ĭ��ֵ.
#define HERB_CHUNK_SIZE (256*1024)  //һ��д��flash��������
#define WRT_CACHE_WAIT_DLY (50)  //unit:ms, ���cache buffer������ѯ��ʱ����.
//#define WRT_ES_FILESYS_WAIT_DLY (50) //unit:ms, ������������߳�Ҳ��д�ļ�ϵͳ,delay��ʱ����
#define WRT_ES_FILESYS_INTERVAL (100) //unit:ms,write�̳߳������ݲ����������,�ȴ���һ��д��ʱ����.


typedef struct tag_HerbFSCache HerbFSCache;

/* ��cache�е�����ȫ��д���ļ�ϵͳ */
typedef __s32(*HerbFSCache_fflush)(HerbFSCache *thiz);

typedef struct __HERB_ES_FILE_PTR
{
    ES_FILE     *fp;        //file pointer for read file

    __u8        *cache;     //buffer for cache file data, should be palloc,
    __u32       buf_size;   //size of cache buffer, user define, HERB_FS_CACHE_SIZE, ������Ч�����ݳ�����buf_size-1���Ա���cur_ptr��head_ptr���غ����������

    __u8        *head_ptr;  //����������Ч���ݵĿ�ʼ��ַ��
    __u8        *cur_ptr;   //ָ����Ч���ݵ�ַ����һ���ֽ�(���̻�ͷ). current access pointer in the cache buffer
    __s64       file_pst;   //truely file offset accessed by cache fs, data_ptr map file_pst.д�ļ�ʱ�����ļ�ϵͳ������д���ĵط�
    __s64       cache_pst;  //current access pointer map the file offset,����cache,�û���Ϊд���ļ�ϵͳ�ĵط�
    __s64       write_total_size;  //current file writing size,(include cache's valid data)������fseek���ܺ��Ѿ���׼�ˣ���������ͳ���ܹ�д�˶�������, �ظ�д��Ҳͳ�������ˡ�û��̫��������.

    __u8        stopflag;//��ֹ�̵߳ı�Ǳ���
    __u32       write_filesys_tskprio;
    cache_lock_t wrt_filesys_lock;
    sem_t       fwrite_sem;
    sem_t       write_task_sem;

    __u8        disk_full_flg;  //0:not full; 1:disk full
} __herb_file_ptr_t;

typedef struct tag_HerbFSCache  //��¼herbfs����ģʽ����ز�����struct
{
    CacheFS base;
    __herb_file_ptr_t HerbFilePtr;
    HerbFSCache_fflush   fflush;
} HerbFSCache;

extern HerbFSCache *newHerbFSCache(void);
extern void deleteHerbFSCache(HerbFSCache *pHerbFSCache);

#endif  /* _HERB_FS_CACHE_H_ */

