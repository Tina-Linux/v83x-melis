/*
********************************************************************************
*                                    eMOD
*                   the Easy Portable/Player Develop Kits
*                               mod_herb sub-system
*                          (module name, e.g.mpeg4 decoder plug-in) module
*
*          (c) Copyright 2010-2012, Allwinner Microelectronic Co., Ltd.
*                              All Rights Reserved
*
* File   : herb_fs_buf.c
* Version: V1.0
* By     : Eric_wang
* Date   : 2011-3-5
* Modify : zengzhijin 2020-04-24
* Description:
********************************************************************************
*/

#include "cache_fs_s.h"
#include "herb_fs_cache.h"
#include <string.h>
#include <kapi.h>
#include <log.h>
#include <stdlib.h>
#include <time.h>

extern __u32   module_id;
#define SYS_TIMEDELAY_UNIT (1000 / CONFIG_HZ)

/* ���ܴ�ӡ���������abs,vbs����գ�muxerдflash�ٶȲ���ʱ����ʹ�� */
#define SYS_LOAD_PRINT __msg

/*******************************************************************************
Function name: HERB_TimeDelay
Description:
    1.ʱ���ӳ�. unit:ms
    2.��װ����ϵͳ���ӳٺ�����
    3.��ΪĿǰ��ϵͳ��delay��λʱ����10ms����������ֵ������10�ı���������
      ���ᾫȷdelay.delayʱ������>=Ԥ��ʱ��.
Parameters:

Return:

Time: 2009/12/25
*******************************************************************************/
static void HERB_TimeDelay(__s32 nDelay)
{
    if (nDelay)
    {
        esKRNL_TimeDly((nDelay + SYS_TIMEDELAY_UNIT - 1) / SYS_TIMEDELAY_UNIT);
    }

    return;
}

/*******************************************************************************
Function name: HERB_fopen
Description:
    1.���ļ���ͬʱ����һ���̣߳�����дflash.
Parameters:
    1.Ҫ��ȡ�ĳ�Ա����:cur_ptr����һ���̻߳��޸ģ����Ծ����ȡ��
    2.Ҫ�޸ĵĳ�Ա����:head_ptr,file_pst.��Щ����������һ���߳��������޸ĵġ�
Return:

Time: 2010/6/23
*******************************************************************************/
static void write_filesys_maintsk(void *p_arg)
{
    HerbFSCache *pHerbFSCache = (HerbFSCache *)p_arg;
    __herb_file_ptr_t  *pHerbFile = &pHerbFSCache->HerbFilePtr;
    __u8    *pCur;
    __u8    *pTmpHeadPtr = NULL;
    __s32   nDataSize;      // �ܵ���Ч�����ݳ���
    __s32   nDataSize1;     // ������ݻ�ͷ����һ�����ݳ���
    __s32   nDataSize2;
    __s32   nTmpDataSize;   //����д��file system�����ݳ���,һ����HERB_CHUNK_SIZE
    __s32   nDelay = 0;
    __u32   nWtDataSize = 0;
    __s32   timeout_flag = -1;
    struct timespec abs_timeout;

    memset(&abs_timeout, 0, sizeof(struct timespec));

    while (1)
    {
        //check if need stop main task
        if (pHerbFile->stopflag)
        {
            //buffer�ڵ�����ȫ��д��file system.
            //HERB_fflush((ES_FILE*)pHerbFile);
            pHerbFSCache->fflush(pHerbFSCache);
            goto _exit_write_filesys_task;
        }

        //check if any request to delete main task
        if (esKRNL_TDelReq(EXEC_prioself) == OS_TASK_DEL_REQ)
        {
            //buffer�ڵ�����ȫ��д��file system.
            //HERB_fflush((ES_FILE*)pHerbFile);
            pHerbFSCache->fflush(pHerbFSCache);
            goto _exit_write_filesys_task;
        }

        //��������ΪHERB_fflush()������һ���̵߳���дfilesystem,��������Ҫ����
        pHerbFile->wrt_filesys_lock.lock(&pHerbFile->wrt_filesys_lock);
        pCur = pHerbFile->cur_ptr;

        if (pHerbFile->head_ptr <= pCur)
        {
            nDataSize1 = pCur - pHerbFile->head_ptr;
            nDataSize2 = 0;
        }
        else
        {
            nDataSize1 = pHerbFile->cache + pHerbFile->buf_size - pHerbFile->head_ptr;
            nDataSize2 = pHerbFile->cur_ptr - pHerbFile->cache;
        }

        nDataSize = nDataSize1 + nDataSize2;

        if (nDataSize < HERB_CHUNK_SIZE)
        {
            nDelay = WRT_ES_FILESYS_INTERVAL;
            goto _wrt_chunk_done;
        }
        else
        {
            nDelay = 0;
        }

        nTmpDataSize = HERB_CHUNK_SIZE;

        if (nDataSize1 >= nTmpDataSize) //ֱ��д
        {
            nWtDataSize = esFSYS_fwrite(pHerbFile->head_ptr, 1, nTmpDataSize, pHerbFile->fp);
            //֮������������δ��룬����ΪҪ��֤pHerbFile->head_ptrһ�θ�ֵ����һ���߳���Ҫ��ȡpHerbFile->head_ptr����ֵ�ģ�
            //����pHerbFile->head_ptr��ֵ���ģ�����ԭ�Ӳ�����
            pTmpHeadPtr = pHerbFile->head_ptr;
            // pTmpHeadPtr += nTmpDataSize;
            pTmpHeadPtr += nWtDataSize;//nTmpDataSize;

            if (pTmpHeadPtr == pHerbFile->cache + pHerbFile->buf_size)
            {
                pTmpHeadPtr = pHerbFile->cache;
            }
            else if (pTmpHeadPtr > pHerbFile->cache + pHerbFile->buf_size)
            {
                __wrn("fatal error! exceed cache buffer\n");
            }

            pHerbFile->head_ptr = pTmpHeadPtr;
            //pHerbFile->file_pst += nTmpDataSize;
            pHerbFile->file_pst += nWtDataSize;//nTmpDataSize;

            if (nWtDataSize != nTmpDataSize)
            {
                __wrn("flash full! [%d]<[%d]\n", nWtDataSize, nTmpDataSize);
                pHerbFile->disk_full_flg = 1;
                //ȥ��,д��file system��,
                pHerbFile->wrt_filesys_lock.unlock(&pHerbFile->wrt_filesys_lock);
                goto _exit_write_filesys_task;
            }
        }
        else    //buffer��Ҫ��ͷ,д��file system.
        {
            //��д��nDataSize1
            nWtDataSize = esFSYS_fwrite(pHerbFile->head_ptr, 1, nDataSize1, pHerbFile->fp);
            pHerbFile->head_ptr = pHerbFile->cache;
            pHerbFile->file_pst += nDataSize1;
            nTmpDataSize -= nDataSize1;

            if (nWtDataSize != nDataSize1)
            {
                __wrn("flash full2! [%d]<[%d]\n", nWtDataSize, nDataSize1);
                pHerbFile->disk_full_flg = 1;
                //ȥ��,д��file system��,
                pHerbFile->wrt_filesys_lock.unlock(&pHerbFile->wrt_filesys_lock);
                goto _exit_write_filesys_task;
            }

            //��дnDataSize2��һ����
            nWtDataSize = esFSYS_fwrite(pHerbFile->head_ptr, 1, nTmpDataSize, pHerbFile->fp);
            pHerbFile->head_ptr += nTmpDataSize;
            pHerbFile->file_pst += nTmpDataSize;

            if (nWtDataSize != nTmpDataSize)
            {
                __wrn("flash full3! [%d]<[%d]\n", nWtDataSize, nTmpDataSize);
                pHerbFile->disk_full_flg = 1;
                //ȥ��,д��file system��,
                pHerbFile->wrt_filesys_lock.unlock(&pHerbFile->wrt_filesys_lock);
                goto _exit_write_filesys_task;
            }
        }

_wrt_chunk_done:
        //ȥ��,д��file system��,
        pHerbFile->wrt_filesys_lock.unlock(&pHerbFile->wrt_filesys_lock);
        /* HERB_TimeDelay(nDelay); */

        if (timeout_flag == 0)
        {
            sem_post(&pHerbFile->fwrite_sem);
        }

        int clock_gettime(clockid_t clockid, struct timespec * tp);
        if (nDelay != 0)
        {
            if (clock_gettime(CLOCK_REALTIME, &abs_timeout) == 0)
            {
#define TICKS_TO_NS(x)  (x * 1000 * 1000 * (1000 / CONFIG_HZ))
                long tv_nsec = abs_timeout.tv_nsec + TICKS_TO_NS(nDelay);

                abs_timeout.tv_sec += tv_nsec / (1000 * 1000 * 1000L);
                abs_timeout.tv_nsec = tv_nsec % (1000 * 1000 * 1000L);

                timeout_flag = sem_timedwait(&pHerbFile->write_task_sem, &abs_timeout) == 0 ? 0 : 1;
            }
            else
            {
                sem_wait(&pHerbFile->write_task_sem);
                timeout_flag = 0;
            }
        }
        else
        {
            timeout_flag = 1;
        }
    }

_exit_write_filesys_task:
    __inf("--------------------------- write_filesys_maintsk exit!\n");
    sem_post(&pHerbFile->fwrite_sem);
    esKRNL_TDel(EXEC_prioself);
}

static ES_FILE *HerbFSCache_fopen(CacheFS *thiz, const __s8 *path, const __s8 *mode)
{
    HerbFSCache *pHerbFSCache = (HerbFSCache *)thiz;
    __herb_file_ptr_t  *tmpHerbFile = &pHerbFSCache->HerbFilePtr;

    if (!path || !mode)
    {
        __wrn("Parameter for open herb file is invalid!\n");
        goto _err0_herb_fopen;
    }

    tmpHerbFile->fp = esFSYS_fopen(path, mode);
    if (!tmpHerbFile->fp)
    {
        __wrn("Open herb file failed!\n");
        goto _err1_herb_fopen;
    }

    /* request memory for cache file data. */
    tmpHerbFile->cache = PHY_MALLOC((tmpHerbFile->buf_size + 1023), 1024);

    if (!tmpHerbFile->cache)
    {
        __wrn("Request memory for cache fs cache failed!\n");
        goto _err2_herb_fopen;
    }

    tmpHerbFile->head_ptr = tmpHerbFile->cache;
    tmpHerbFile->cur_ptr   = tmpHerbFile->cache;
    tmpHerbFile->file_pst  = eLIBs_flltell(tmpHerbFile->fp);
    tmpHerbFile->cache_pst = tmpHerbFile->file_pst;
    tmpHerbFile->write_total_size = 0;
    cache_lock_init(&pHerbFSCache->HerbFilePtr.wrt_filesys_lock);
    sem_init(&pHerbFSCache->HerbFilePtr.fwrite_sem, 0, 0);
    sem_init(&pHerbFSCache->HerbFilePtr.write_task_sem, 0, 0);

    /* ���߳� */
    tmpHerbFile->write_filesys_tskprio = esKRNL_TCreate(write_filesys_maintsk,
                                         pHerbFSCache,
                                         0x2000,
                                         (module_id << 8) | WRITE_ES_FILESYS_TASK_PRIO);

    if (0 == tmpHerbFile->write_filesys_tskprio)
    {
        __wrn("herb_fopen create task fail,open file fail!\n");
        goto _err3_herb_fopen;
    }

    return (ES_FILE *)tmpHerbFile;

_err3_herb_fopen:
    cache_lock_destroy(&pHerbFSCache->HerbFilePtr.wrt_filesys_lock);
    sem_destroy(&pHerbFSCache->HerbFilePtr.fwrite_sem);
    sem_destroy(&pHerbFSCache->HerbFilePtr.write_task_sem);

    if (tmpHerbFile->cache)
    {
        PHY_FREE(tmpHerbFile->cache);
        tmpHerbFile->cache = NULL;
    }

_err2_herb_fopen:
    esFSYS_fclose(tmpHerbFile->fp);
_err1_herb_fopen:
_err0_herb_fopen:
    return 0;
}
/*******************************************************************************
Function name: HERB_fclose
Description:
    1.�ر��ļ���Ҫ��������:
      (1) �����̡߳�
      (2) �ͷ�cache.
      (3) �ر��ļ�.

Parameters:

Return:
     -1:ʧ��
    0:�ɹ�
Time: 2010/6/24
*******************************************************************************/
static __s32 HerbFSCache_fclose(CacheFS *thiz)
{
    HerbFSCache *pHerbFSCache = (HerbFSCache *)thiz;
    __herb_file_ptr_t  *tmpHerbFile = &pHerbFSCache->HerbFilePtr;

    if (!tmpHerbFile->fp)
    {
        __wrn("file handle is invalid when close herb file!\n");
        return -1;
    }

    /* �����߳� */
    tmpHerbFile->stopflag = 1;

    if (tmpHerbFile->write_filesys_tskprio)
    {
        /* wake up file parser main task */
        /* esKRNL_TimeDlyResume(tmpHerbFile->write_filesys_tskprio); */
        sem_post(&tmpHerbFile->write_task_sem);

        while (esKRNL_TDelReq(tmpHerbFile->write_filesys_tskprio) != OS_TASK_NOT_EXIST)
        {
            /* esKRNL_TimeDlyResume(tmpHerbFile->write_filesys_tskprio); */
            sem_post(&tmpHerbFile->write_task_sem);
            esKRNL_TimeDly(1);
        }

        tmpHerbFile->write_filesys_tskprio = 0;
    }

    cache_lock_destroy(&tmpHerbFile->wrt_filesys_lock);
    sem_destroy(&tmpHerbFile->fwrite_sem);
    sem_destroy(&tmpHerbFile->write_task_sem);

    /* release cache buffer */
    if (tmpHerbFile->cache)
    {
        PHY_FREE(tmpHerbFile->cache);
        tmpHerbFile->cache = 0;
    }

    /* close file handle */
    if (tmpHerbFile->fp)
    {
        esFSYS_fclose(tmpHerbFile->fp);
        tmpHerbFile->fp = NULL;
    }

    /* reset herb file handle */
    memset(tmpHerbFile, 0, sizeof(__herb_file_ptr_t));

    return 0;
}

__s32 HerbFSCache_fread(CacheFS *thiz, void *buf, __s32 size, __s32 count)
{
    __wrn("HerbCache mode don't support fread!\n");
    return 0;
}

/*******************************************************************************
Function name: HERB_fwrite
Description:
    1.��cache buffer��֮�������ǵȣ���buffer�пռ���д!
      ��Ϊ������ֱ��д�ļ�ϵͳ��������д��buffer�С�
    2.Ҫ��ȡ�ĳ�Ա����:head_ptr
      Ҫ�޸ĵĳ�Ա����:cur_ptr,cache_pst,file_size.��Щ����������һ���߳��������޸ĵġ�

Parameters:
    1.size:һ��unit�Ĵ�С��
    2.count:unit��������
Return:
    1. EPDK_FAIL; fatal error
    2. count written: ����д������ݿ������
Time: 2010/6/25
*******************************************************************************/
static __s32 HerbFSCache_fwrite(CacheFS *thiz, void *buf, __s32 size, __s32 count)   //д�ļ�
{
    HerbFSCache *pHerbFSCache = (HerbFSCache *)thiz;
    __herb_file_ptr_t  *tmpHerbFile = &pHerbFSCache->HerbFilePtr;
    __s32               tmpDataSize;    //Ҫд��buffer�����ݳ��ȣ��ֽ�
    __s32               nWtDataSize = 0;    //��д��buffer���ֽڳ���
    __u8                *tmpUsrPtr; //��д���ݵ���ʼ��ַ
    __u8    *phead;     //��ǰ��Ч���ݵ���ʼ��ַ
    __s32   nLeftLen;   //cache bufferʣ��Ŀռ�
    __s32   nLeftLen1;   //cache bufferʣ��ĵ�һ�οռ䣬��Ϊ��ringbuffer�����Ի��ͷ
    __s32   nLeftLen2;   //cache bufferʣ��ĵڶ��οռ䣬

    if (!tmpHerbFile || !size || !count)
    {
        __wrn("File handle or parameter is invalid when read file!\n");
        return 0;
    }

    tmpUsrPtr = (__u8 *)buf;
_write_once:

    tmpHerbFile->wrt_filesys_lock.lock(&tmpHerbFile->wrt_filesys_lock);

    if (tmpHerbFile->disk_full_flg)
    {
        __wrn("HERB FS DISK FULL!\n");
        return nWtDataSize / size;
    }

    phead = tmpHerbFile->head_ptr;

    if (tmpHerbFile->cur_ptr >= phead)
    {
        if (phead == tmpHerbFile->cache)
        {
            nLeftLen1 = tmpHerbFile->cache + tmpHerbFile->buf_size - tmpHerbFile->cur_ptr - 1;
            nLeftLen2 = 0;
        }
        else
        {
            nLeftLen1 = tmpHerbFile->cache + tmpHerbFile->buf_size - tmpHerbFile->cur_ptr;
            nLeftLen2 = phead - tmpHerbFile->cache - 1;
        }
    }
    else
    {
        nLeftLen1 = phead - tmpHerbFile->cur_ptr - 1;
        nLeftLen2 = 0;
    }

    nLeftLen = nLeftLen1 + nLeftLen2;
    tmpDataSize = size * count - nWtDataSize;

    //д��buffer
    if (tmpDataSize <= nLeftLen) //Ҫд�뻺��������������ʣ��ռ��٣�
    {
        if (tmpDataSize <= nLeftLen1) //��һ���������д��
        {
            memcpy(tmpHerbFile->cur_ptr, tmpUsrPtr, tmpDataSize);

            //MEM_CPY(tmpHerbFile->cur_ptr, tmpUsrPtr, tmpDataSize);
            //tmpHerbFile->cur_ptr��ֵ����һ�θ���!��Ϊ����һ���߳�Ҫ��ȡ������������ֵ���ܸ���.
            if (tmpHerbFile->cur_ptr + tmpDataSize == tmpHerbFile->cache + tmpHerbFile->buf_size)
            {
                tmpHerbFile->cur_ptr = tmpHerbFile->cache;
            }
            else if (tmpHerbFile->cur_ptr + tmpDataSize > tmpHerbFile->cache + tmpHerbFile->buf_size)
            {
                __wrn("HERB FS fatal error!\n");
                tmpHerbFile->cur_ptr = tmpHerbFile->cache;
                tmpHerbFile->wrt_filesys_lock.unlock(&tmpHerbFile->wrt_filesys_lock);
                return EPDK_FAIL;
            }
            else
            {
                tmpHerbFile->cur_ptr += tmpDataSize;
            }

            tmpHerbFile->cache_pst += tmpDataSize;
            tmpHerbFile->write_total_size += tmpDataSize;
            tmpHerbFile->wrt_filesys_lock.unlock(&tmpHerbFile->wrt_filesys_lock);
            return count;
        }
        else //��һ������д���꣬��Ҫ��д�ڶ�������
        {
            //MEM_CPY(tmpHerbFile->cur_ptr, tmpUsrPtr, nLeftLen1);
            memcpy(tmpHerbFile->cur_ptr, tmpUsrPtr, nLeftLen1);
            tmpUsrPtr += nLeftLen1;
            tmpDataSize -= nLeftLen1;
            //MEM_CPY(tmpHerbFile->cache, tmpUsrPtr, tmpDataSize);
            memcpy(tmpHerbFile->cache, tmpUsrPtr, tmpDataSize);
            tmpHerbFile->cur_ptr = tmpHerbFile->cache + tmpDataSize;
            tmpHerbFile->cache_pst += (nLeftLen1 + tmpDataSize);
            tmpHerbFile->write_total_size += (nLeftLen1 + tmpDataSize);
            tmpHerbFile->wrt_filesys_lock.unlock(&tmpHerbFile->wrt_filesys_lock);
            return count;
        }
    }
    else //���������д�������ݣ���д�����ٵȡ�
    {
        if (nLeftLen1)
        {
            //MEM_CPY(tmpHerbFile->cur_ptr, tmpUsrPtr, nLeftLen1);
            memcpy(tmpHerbFile->cur_ptr, tmpUsrPtr, nLeftLen1);
            tmpUsrPtr += nLeftLen1;
            tmpDataSize -= nLeftLen1;

            if (tmpHerbFile->cur_ptr + nLeftLen1 == tmpHerbFile->cache + tmpHerbFile->buf_size)
            {
                tmpHerbFile->cur_ptr = tmpHerbFile->cache;
            }
            else if (tmpHerbFile->cur_ptr + nLeftLen1 > tmpHerbFile->cache + tmpHerbFile->buf_size)
            {
                __wrn("HERB FS write data fatal error!\n");
                tmpHerbFile->wrt_filesys_lock.unlock(&tmpHerbFile->wrt_filesys_lock);
                return EPDK_FAIL;
            }
            else
            {
                tmpHerbFile->cur_ptr += nLeftLen1;
            }

            tmpHerbFile->cache_pst += nLeftLen1;
            tmpHerbFile->write_total_size += nLeftLen1;
        }

        if (nLeftLen2)
        {
            memcpy(tmpHerbFile->cur_ptr, tmpUsrPtr, nLeftLen2);
            tmpUsrPtr += nLeftLen2;
            tmpDataSize -= nLeftLen2;
            tmpHerbFile->cur_ptr += nLeftLen2;
            tmpHerbFile->cache_pst += nLeftLen2;
            tmpHerbFile->write_total_size += nLeftLen2;
        }

        nWtDataSize += nLeftLen;
        tmpHerbFile->wrt_filesys_lock.unlock(&tmpHerbFile->wrt_filesys_lock);
        /* HERB_TimeDelay(WRT_CACHE_WAIT_DLY); */
        sem_post(&tmpHerbFile->write_task_sem);
        sem_wait(&tmpHerbFile->fwrite_sem);
        SYS_LOAD_PRINT("[LOADTEST]herb fs write wait [%d]ms\n", WRT_CACHE_WAIT_DLY);
        goto _write_once; //��delay�����У���һ���̻߳�ı�tmpHerbFile->head_ptr.
    }
}
/*******************************************************************************
Function name: HerbFSCache_fseek
Description:
    1. fseek��������:�ȵ���fflush()���ѻ�����������д�꣬Ȼ����fseek��ָ���ĵط�.
    2. fseek��fread���ܲ�������.���ɵ����߱�֤.
Parameters:

Return:

Time: 2011/3/6
*******************************************************************************/
static __s32 HerbFSCache_fseek(CacheFS *thiz, __s64 offset, __s32 origin)   //fseek
{
    __s32   ret;
    HerbFSCache *pHerbFSCache = (HerbFSCache *)thiz;
    __herb_file_ptr_t *pHerbFile = &pHerbFSCache->HerbFilePtr;
    ret = pHerbFSCache->fflush(pHerbFSCache);

    if (EPDK_OK != ret)
    {
        __wrn("fseek,fflush() fail\n");
        return EPDK_FAIL;
    }

    ret = eLIBs_fllseek(pHerbFile->fp, offset, origin);
    pHerbFile->file_pst = pHerbFile->cache_pst = eLIBs_flltell(pHerbFile->fp);

    if (0 != ret)
    {
        __wrn("fatal error! fseek fail\n");
    }

    return ret;
}

static __s32 HerbFSCache_ftruncate(CacheFS *thiz, off_t length)
{
    __s32   ret;
    HerbFSCache *pHerbFSCache = (HerbFSCache *)thiz;
    __herb_file_ptr_t *pHerbFile = &pHerbFSCache->HerbFilePtr;

    ret = pHerbFSCache->fflush(pHerbFSCache);
    if (EPDK_OK != ret)
    {
        __wrn("HerbFSCache_ftruncate, fflush() fail\n");
        return EPDK_FAIL;
    }

    ret = esFSYS_ftruncate(pHerbFile->fp, length);
    if (EPDK_OK == ret)
    {
        pHerbFile->file_pst = pHerbFile->cache_pst = eLIBs_flltell(pHerbFile->fp);
    }
    else
    {
        __wrn("fatal error! ftruncate fail\n");
    }

    return ret;
}

static __s32 HerbFSCache_fsync(CacheFS *thiz)
{
    __s32   ret;
    HerbFSCache *pHerbFSCache = (HerbFSCache *)thiz;
    __herb_file_ptr_t *pHerbFile = &pHerbFSCache->HerbFilePtr;

    ret = pHerbFSCache->fflush(pHerbFSCache);
    if (EPDK_OK != ret)
    {
        __wrn("HerbFSCache_ftruncate, fflush() fail\n");
        return EPDK_FAIL;
    }

    ret = esFSYS_fsync(pHerbFile->fp);
    if (EPDK_OK == ret)
    {
        pHerbFile->file_pst = pHerbFile->cache_pst = eLIBs_flltell(pHerbFile->fp);
    }
    else
    {
        __wrn("fatal error! fsync fail\n");
    }

    return ret;
}

static __s32 HerbFSCache_fstat(CacheFS *thiz, void *buf)
{
    __s32 ret = 0;
    __ES_FSTAT *stat_buf = (__ES_FSTAT *)buf;
    HerbFSCache *pHerbFSCache = (HerbFSCache *)thiz;
    __herb_file_ptr_t *pHerbFile = &pHerbFSCache->HerbFilePtr;

    stat_buf->pos = pHerbFile->cache_pst;
    stat_buf->size = pHerbFile->cache_pst;

    return ret;
}

static __s64 HerbFSCache_ftell(CacheFS *thiz)
{
    HerbFSCache *pHerbFSCache = (HerbFSCache *)thiz;
    return pHerbFSCache->HerbFilePtr.cache_pst;
}

/* ������mode = HERB_FS_WORK_MODE_CACHE����buffer��ģʽ */
static __s32 HerbFSCache_SetCacheSize(CacheFS *thiz, __s32 cache_size)
{
    HerbFSCache *pHerbFSCache = (HerbFSCache *)thiz;
    pHerbFSCache->HerbFilePtr.buf_size = cache_size;
    return EPDK_OK;
}

static ES_FILE *HerbFSCache_GetFp(CacheFS *thiz)
{
    HerbFSCache *pHerbFSCache = (HerbFSCache *)thiz;
    return pHerbFSCache->HerbFilePtr.fp;
}

/*******************************************************************************
Function name: HERB_fflush
Description:
    1.��cache buffer�е�����ȫ��д��file system��ȥ��
Parameters:
    1.Ҫ��ȡ�ĳ�Ա����:cur_ptr�������л����ź�������.
    2.Ҫ�޸ĵĳ�Ա����:head_ptr,file_pst.��Щ����������һ���߳��������޸ĵġ�
Return:
    1. EPDK_OK,
    2.EPDK_FAIL;
Time: 2010/6/28
*******************************************************************************/
static __s32 Impl_HerbFSCache_fflush(HerbFSCache *thiz)     //��cache�е�����ȫ��д���ļ�ϵͳ
{
    HerbFSCache *pHerbFSCache = (HerbFSCache *)thiz;
    __herb_file_ptr_t  *pHerbFile = &pHerbFSCache->HerbFilePtr;
    __s32   ret = EPDK_OK;
    __u8    *pCur;  //ָ��ǰ����Ч���ݵ���һ�ֽڣ�����Ҫд���buf��λ��
    __s32   nDataSize1;
    __s32   nDataSize2;

    if (pHerbFile->disk_full_flg)
    {
        __wrn("fflush detect disk full\n");
        return EPDK_FAIL;
    }

    pHerbFile->wrt_filesys_lock.lock(&pHerbFile->wrt_filesys_lock);
    pCur = pHerbFile->cur_ptr;

    if (pHerbFile->head_ptr <= pCur)
    {
        nDataSize1 = pCur - pHerbFile->head_ptr;
        nDataSize2 = 0;
    }
    else
    {
        nDataSize1 = pHerbFile->cache + pHerbFile->buf_size - pHerbFile->head_ptr;
        nDataSize2 = pHerbFile->cur_ptr - pHerbFile->cache;
    }

    //tmpDataSize = nDataSize1 + nDataSize2;

    //��ʼдfile system
    if (nDataSize1)
    {
        esFSYS_fwrite(pHerbFile->head_ptr, 1, nDataSize1, pHerbFile->fp);

        if (pHerbFile->head_ptr + nDataSize1 == pHerbFile->cache + pHerbFile->buf_size)
        {
            pHerbFile->head_ptr = pHerbFile->cache;
            pHerbFile->file_pst += nDataSize1;
        }
        else if (pHerbFile->head_ptr + nDataSize1 < pHerbFile->cache + pHerbFile->buf_size)
        {
            pHerbFile->head_ptr += nDataSize1;
            pHerbFile->file_pst += nDataSize1;
        }
        else
        {
            __wrn("HERB_fflush() fatal error!\n");
            ret = EPDK_FAIL;
            goto _quit_flush;
        }
    }

    if (nDataSize2)
    {
        if (0 == nDataSize1)
        {
            __wrn("fatal error, nDataSize1=%d, nDataSize2=%d\n", nDataSize1, nDataSize2);
            ret = EPDK_FAIL;
            goto _quit_flush;
        }

        esFSYS_fwrite(pHerbFile->head_ptr, 1, nDataSize2, pHerbFile->fp);
        pHerbFile->head_ptr += nDataSize2;
        pHerbFile->file_pst += nDataSize2;
    }

    ret = EPDK_OK;
_quit_flush:
    pHerbFile->wrt_filesys_lock.unlock(&pHerbFile->wrt_filesys_lock);
    return ret;
}

HerbFSCache *newHerbFSCache()
{
    HerbFSCache *pHerbFSCache = (HerbFSCache *)malloc(sizeof(HerbFSCache));

    if (NULL == pHerbFSCache)
    {
        __wrn("NewHerbFSCache() malloc fail\n");
        return NULL;
    }

    memset(pHerbFSCache, 0, sizeof(HerbFSCache));
    CacheFS_Init(&pHerbFSCache->base);

    pHerbFSCache->base.cache_fs_work_mode = CACHE_FS_WORK_MODE_HERBCACHE;
    pHerbFSCache->base.cachefs_fopen = HerbFSCache_fopen;
    pHerbFSCache->base.cachefs_fclose = HerbFSCache_fclose;
    pHerbFSCache->base.cachefs_fread = HerbFSCache_fread;
    pHerbFSCache->base.cachefs_fwrite = HerbFSCache_fwrite;
    pHerbFSCache->base.cachefs_fseek = HerbFSCache_fseek;
    pHerbFSCache->base.cachefs_ftell = HerbFSCache_ftell;
    pHerbFSCache->base.cachefs_fsync = HerbFSCache_fsync;
    pHerbFSCache->base.cachefs_fstat = HerbFSCache_fstat;
    pHerbFSCache->base.cachefs_ftruncate = HerbFSCache_ftruncate;
    pHerbFSCache->base.SetCacheSize = HerbFSCache_SetCacheSize;
    pHerbFSCache->base.GetFp = HerbFSCache_GetFp;

    memset(&pHerbFSCache->HerbFilePtr, 0, sizeof(__herb_file_ptr_t));
    pHerbFSCache->HerbFilePtr.buf_size = HERB_FS_CACHE_SIZE;
    pHerbFSCache->fflush = Impl_HerbFSCache_fflush;
    return pHerbFSCache;
}

void deleteHerbFSCache(HerbFSCache *pHerbFSCache)
{
    free(pHerbFSCache);
}
