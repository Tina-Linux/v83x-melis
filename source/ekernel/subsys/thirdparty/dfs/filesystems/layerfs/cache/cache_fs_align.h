/*
*********************************************************************************************************
*                                                    eMOD
*                                   the Easy Portable/Player Operation System
*                                              mod_mmp sub-system
*
*                                    (c) Copyright 2008-2009, Kevin.Z China
*                                              All Rights Reserved
*
* File   : cache_fs_align.h
* Version: V1.0
* By     : kevin.z
* Date   : 2009-2-2 8:37
* Modify : zengzhijin 2020-04-23
*********************************************************************************************************
*/
#ifndef _CACHE_FS_ALIGN_H_
#define _CACHE_FS_ALIGN_H_

#include "cache_fs_base.h"

typedef struct __CACHE_AES_FILE_PTR
{
    ES_FILE     *fp;            /* file pointer for read file */

    /* cache structure */
    __u8        *cache;         //buffer for cache file data, should be palloc
    __u32       cache_size;     //size of cache buffer, user define, һ����CACHE_FS_CACHE_SIZE bytes
    __u32       cache_bits;     //bits of cache buffer size, user define�� 2��cache_bits�η� = CACHE_FS_CACHE_SIZE
    __u32       cache_offset;   //cache offset position����δ�����ݵ���ʼλ����cache��ƫ��
    __u32       cache_left;     //cache left data size

    /* sector structure */
    __u32       sector_size;    //fs sector size
    __u32       sector_bits;    //fs sector bits

    __s64       file_pos;       //file offset accessed by user����ǰ���ļ�ϵͳ����ʵ������λ��
    __s64       file_size;      //file size

    /* cache statistical parameters for optimize */
    /* cache maybe dynamicly adjust size */
#ifdef CONFIG_LAYERFS_CACHE_DEBUG
    ES_FILE     *log_fp;            //file pointer for log file
    __u32       cache_hits;         //record cache hit times��cache�б�����Ҫ�������ݵĴ���
    __u32       cache_discard_cnt;  //redord cache discard times����Ϊfseek��ֱ�Ӷ���cache�Ĵ���
    __u32       direct_read_cnt;    //redord direct read times, cache no use��ֱ�Ӷ�ȡ������sector��buf�Ĵ���
    __u32       total_read_cnt;     //redord total read times������fread�Ĵ���
    __u32       total_read_size;    //redord total read size, �ۼƶ�ȡ���ֽ���
    __u32       total_seek_cnt;     //redord total seek times,�ۼƵ���fseek�Ĵ���
#endif
} __cache_afile_ptr_t;

/* ��¼cachefs����ģʽ����ز�����struct */
typedef struct tag_CACHEFSALIGN
{
    CacheFS CacheFSBase;
    __cache_afile_ptr_t AFilePtr;
} CacheFSAlign;
extern CacheFSAlign *newCacheFSAlign(void);
extern void deleteCacheFSAlign(CacheFSAlign *pCacheFSAlign);

#endif //_CACHE_FS_ALIGN_H_

