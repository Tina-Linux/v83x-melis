/*
*********************************************************************************************************
*                                                    MELIS
*                                    the Easy Portable/Player Develop Kits
*                                                  File System
*
*                                    (c) Copyright 2011-2014, Sunny China
*                                             All Rights Reserved
*
* File    : fsman.c
* By      : Sunny
* Version : v1.0
* Date    : 2011-1-15
* Descript: vfs manager, code is extracted from linux.
* Update  : date                auther      ver     notes
*           2011-3-15 15:22:55  Sunny       1.0     Create this file.
*********************************************************************************************************
*/
#include "fsys_debug.h"
#include "err.h"
#include "nls.h"
#include <mod_charset.h>
#include <kapi.h>
#include <arch.h>
#include <log.h>
#include <debug.h>

static struct fsys_mutex  vfs_mutex = { NULL, NULL, 0 };
struct file_system_type  *pFSRoot = NULL;

/* Charset used for input and display */
struct nls_table     *nls = NULL;
__s32                 fs_err = 0;

int buffer_init(void);
int kmem_cache_init(void);
int kmem_cache_sizes_init(void);

/*
**********************************************************************************************************************
*
*             FS_Init
*
*  Description:
*  API function. Start the file system.
*
*  Parameters:
*  None.
*
*  Return value:
*  ==0         - File system has been started.
*  !=0         - An error has occured.
**********************************************************************************************************************
*/

__s32 fsys_vfs_init(void)
{
    int   err = 0;

    vfs_mutex.sem   = esKRNL_SemCreate(1);
    if (vfs_mutex.sem == NULL)
    {
        __err("create sem failure.");
        return EPDK_FAIL;
    }
    vfs_mutex.nref  = 0;    /* No enterance any now */
    vfs_mutex.owner = NULL;  /* No any owner now     */

    err |= !(vfs_mutex.sem);
    err |= page_pool_ini();
    err |= kmem_cache_init();
    err |= kmem_cache_sizes_init();
    err |= vfs_caches_init(0);
    err |= buffer_init();

    /* filesystem default charset type is UTF8 */
    nls = load_nls(EPDK_CHARSET_ENM_UTF8);

    if (err)
    {
        return EPDK_FAIL;
    }
    return EPDK_OK;
}

/*
**********************************************************************************************************************
*
*             FS_Exit
*
*  Description:
*  API function. Stop the file system.
*
*  Parameters:
*  None.
*
*  Return value:
*  ==0         - File system has been stopped.
*  !=0         - An error has occured.
**********************************************************************************************************************
*/

__s32 fsys_vfs_exit(void)
{
    return EPDK_FAIL;
}

/*
**********************************************************************************************************************
*
*             fsys_regist_part
*
*  Description:
*   ע����豸���ļ�ϵͳ��
*
*  Parameters:
*   pFullName   - Fully qualified name.
*   pFilename   - Address of a pointer, which is modified to point to
*                the file name part of pFullName.
*
*  Return value:
*   <0          - Unable to find the device.
*   >=0         - Index of the device in the device information table.
**********************************************************************************************************************
*/
__s32 esFSYS_fsreg(__hdle hFS)
{
    __s32  res = EPDK_OK;
    __u32  cpu_sr;
    struct file_system_type *p, *fs = (struct file_system_type *)hFS;

    fs_log_trace0("fsreg:");

    if (esFSYS_vfslock())
    {
        __log("fsreg error when enter vfs mutex");
        return EPDK_FAIL;
    }

    cpu_sr = awos_arch_lock_irq();

    /* �ж��ļ�ϵͳ�Ƿ��Ѿ���ע�� */
    for (p = pFSRoot; p; p = p->next)
    {
        if (strcmp(p->name, fs->name) == 0)
        {
            __log("fs already registered!");
            awos_arch_unlock_irq(cpu_sr);
            fs_err = -EEXIST;
            res = EPDK_FAIL;
            goto out;
        }
    }

    /* ��ʼ���������� */
    INIT_LIST_HEAD(&fs->fs_supers);
    /* �ļ�ϵͳ�û�Ϊ0                          */
    fs->use_cnt = 0;
    /* ��������Ϊ��Ч����                       */
    fs->status = FS_STAT_ACTIVE;
    /* �������ҽӵ�pFSRoot��                    */
    fs->next   = pFSRoot;
    pFSRoot    = fs;
    awos_arch_unlock_irq(cpu_sr);

out:
    esFSYS_vfsunlock();

    fs_log_trace0("O%d", res);
    return res;
}
/*
**********************************************************************************************************************
*
*             fsys_regist_part
*
*  Description:
*   �γ����豸
*
*  Parameters:
*   pFullName   - Fully qualified name.
*   pFilename   - Address of a pointer, which is modified to point to
*                the file name part of pFullName.
*
*  Return value:
*   <0          - Unable to find the device.
*   >=0         - Index of the device in the device information table.
**********************************************************************************************************************
*/
__s32 esFSYS_fsunreg(__hdle hFS)
{
    __u32   cpu_sr;
    __s32   res = EPDK_OK;
    struct file_system_type *fs = (struct file_system_type *)hFS;
    struct file_system_type *p, **pp = &pFSRoot;

    fs_log_trace0("fsureg:");

    if (esFSYS_vfslock())
    {
        __log("fsunreg error when enter vfs mutex");
        return EPDK_FAIL;
    }

    cpu_sr = awos_arch_lock_irq();
    if (fs->use_cnt)
    {
        __log("file system busy!");
        awos_arch_unlock_irq(cpu_sr);
        fs_err = -EBUSY;
        res = EPDK_FAIL;
        goto out;
    }

    for (p = *pp; p && (fs != p); pp = &(p->next), p = p->next);
    if (!p)
    {
        awos_arch_unlock_irq(cpu_sr);
        __log("try to destroy a fs not exsit in fs list!");
        fs_err = -ENXIO;
        res = EPDK_FAIL;
        goto out;
    }

    fs->status = FS_STAT_INACTIVE;
    *pp = fs->next;

    awos_arch_unlock_irq(cpu_sr);

out:
    esFSYS_vfsunlock();

    fs_log_trace0("O%d", res);
    return res;
}

__s32 esFSYS_mntfs(__hdle part)
{
    __u32 cpu_sr;
    __u32 last_lun;
    __s32 res;
    char *partName;
    struct file_system_type *pFS;

    if (esFSYS_vfslock())
    {
        __err("mntfs error when enter vfs mutex");
        return EPDK_FAIL;
    }

    last_lun = 0;

    esFSYS_pioctrl(part, PART_IOC_SYS_GETLASTPART, 0, &last_lun);

    __inf("last_lun=%d", last_lun);

    esFSYS_pioctrl(part, PART_IOC_SYS_GETNAME, 0, &partName);

    pFS = pFSRoot;
    while (pFS)
    {
        if (pFS->identify && (pFS->status == FS_STAT_ACTIVE))
        {
            cpu_sr = awos_arch_lock_irq();
            pFS->use_cnt++;
            awos_arch_unlock_irq(cpu_sr);

            if (pFS->identify(part) == EPDK_TRUE)
            {
                /* �������Ա�ʶ��   */
                if (pFS->mount(part) == EPDK_OK)
                {
                    /* �������Ա��ҽ�   */
                    cpu_sr = awos_arch_lock_irq();
                    pFS->use_cnt--;
                    awos_arch_unlock_irq(cpu_sr);

#ifdef CONFIG_MELIS_LAYERFS
                    unsigned long letter = 0;
                    char path[32];
                    memset(&path, 0, sizeof(path));
                    memcpy(path, CONFIG_MELIS_LAYERFS_DIR_PATH, strlen(CONFIG_MELIS_LAYERFS_DIR_PATH));

                    esFSYS_pioctrl(part, PART_IOC_SYS_GETLETTER, 0, &letter);
                    path[strlen(CONFIG_MELIS_LAYERFS_DIR_PATH)] = (char)letter;

                    int mount(const char *device_name, const char *path,
                              const char *filesystemtype,
                              unsigned long rwflag,
                              const void *data);

                    mount(NULL, path, pFS->name, 0, (const void *)letter);
#endif
                    break;
                }
            }
            cpu_sr = awos_arch_lock_irq();
            pFS->use_cnt--;
            awos_arch_unlock_irq(cpu_sr);
        }
        pFS = pFS->next;
    }
    if (!pFS)
    {
        __err("identify the filesystem failure.");
        fs_err = -ENOEXEC;
        res = EPDK_FAIL;
    }
    else
    {
        __epos_kmsg_t *msg;
        char           letter;

        /* ���͹㲥��Ϣ����֪ϵͳ�ϲ��ļ�ϵͳ�Ѿ���ж�� */
        msg = (__epos_kmsg_t *)malloc(sizeof(__epos_kmsg_t));
        if (!msg)
        {
            __err("msg struct balloc fail!");
        }
        else
        {
            esFSYS_pioctrl(part, PART_IOC_SYS_GETLETTER, 0, &letter);

            __log("%s,mount on %c.", pFS->name, letter);

            msg->target       = KMSG_TGT_BROADCST;
            msg->message      = KMSG_VAL_FS_PLUGIN;
            msg->prio         = KMSG_PRIO_LOW;
            msg->l.para       = ((__u8)letter) | (((__u8)last_lun) << 8);
            esKSRV_SendMsgEx((void *)msg);
        }
        res =   EPDK_OK;
    }

    esFSYS_vfsunlock();

    fs_log_trace0("O%d", res);
    return res;
}

__s32 esFSYS_umntfs(__hdle part, __s32 force)
{
    __s32  res;
    struct file_system_type *pFS;
    struct super_block *sb;

    fs_log_trace0("umntfs:");

    if (esFSYS_vfslock())
    {
        __log("umntfs error when enter vfs mutex");
        return EPDK_FAIL;
    }

    /* ���ļ�ϵͳ��unmount����  */
    esFSYS_pioctrl(part, PART_IOC_SYS_GETFSPRIV, 0, &sb);
    if (sb)
    {
        pFS = sb->s_type;
        if (pFS && pFS->unmount)
        {
            if (pFS->unmount(sb, force) == EPDK_FAIL)
            {
                if (force)
                {
                    __log("force umount: unexpect error meet.");
                }
                else
                {
                    __log("non-force umount: %c is busy.", sb->s_letter);
                }
                fs_err = -EBUSY;
                res = EPDK_FAIL;
                goto out;
            }
            else
            {
                __epos_kmsg_t         *msg;
                char *partname;
                char  letter;

                /* suppber block�����Ѿ����������٣���Ҫ�ӷ�����
                   �ݽṹ�л�ȡ������                           */
                esFSYS_pioctrl(part, PART_IOC_SYS_GETNAME, 0, &partname);
                esFSYS_pioctrl(part, PART_IOC_SYS_GETLETTER, 0, &letter);
                __log("part \"%c:[%s]\" is unmounted from"
                      "\"%s\" fs.", letter, partname, pFS->name);

#ifdef CONFIG_MELIS_LAYERFS
                char path[32];
                memset(&path, 0, sizeof(path));
                memcpy(path, CONFIG_MELIS_LAYERFS_DIR_PATH, strlen(CONFIG_MELIS_LAYERFS_DIR_PATH));

                path[strlen(CONFIG_MELIS_LAYERFS_DIR_PATH)] = letter;
                int umount(const char *specialfile);
                umount(path);
#endif
                /* ���͹㲥��Ϣ����֪ϵͳ�ϲ��ļ�ϵͳ�Ѿ���ж�� */
                msg = (__epos_kmsg_t *)malloc(sizeof(__epos_kmsg_t));
                if (!msg)
                {
                    __log("msg struct balloc fail!");
                }
                else
                {
                    msg->target       = KMSG_TGT_BROADCST;
                    msg->message      = KMSG_VAL_FS_PLUGOUT;
                    msg->prio         = KMSG_PRIO_LOW;
                    msg->l.para       = letter;
                    esKSRV_SendMsgEx((void *)msg);
                }
            }
        }
    }
    esFSYS_pioctrl(part, PART_IOC_SYS_SETFSPRIV, 0, NULL);
    res = EPDK_OK;

out:
    esFSYS_vfsunlock();

    fs_log_trace0("O%d", res);
    return res;
}

int esFSYS_vfslock(void)
{
    __u8 err = 0;

    /* if the same thread enter multiple times,
     * you can't pend sem again to aviod block itself,
     * just simple increase vfs_mutex.nref and return.
     */
    if ((vfs_mutex.nref > 0) && (vfs_mutex.owner == awos_task_self()))
    {
        vfs_mutex.nref++;
        return 0;
    }

    esKRNL_SemPend(vfs_mutex.sem, 0, &err);
    if (err)
    {
        __log("Pend sem error, error number:%d", err);
        return err;
    }
    vfs_mutex.owner = awos_task_self();
    vfs_mutex.nref++;

    return 0;
}

int esFSYS_vfsunlock(void)
{
    if ((vfs_mutex.owner != awos_task_self()) || (vfs_mutex.nref < 1))
    {
        __log("vfs mutex misuse");
        return -EINVAL;
    }

    /* if the same thread leave multiple times,
     * you can't post sem multiple times,
     * just simple decrease vfs_mutex.nref and return.
     */
    if (vfs_mutex.nref > 1)
    {
        vfs_mutex.nref--;
        return 0;
    }
    vfs_mutex.nref--;
    esKRNL_SemPost(vfs_mutex.sem);

    return 0;
}

__s32 test_and_freeze_partfs(__hdle h_sb)
{
    __u32           cpu_sr;
    struct super_block *sb = (struct super_block *)h_sb;

    if (!sb)
    {
        return 1;
    }

    shrink_dcache_for_umount(sb, 0);
    if (!invalidate_inodes(sb))
    {
        cpu_sr = awos_arch_lock_irq();
        sb->s_frozen++;
        awos_arch_unlock_irq(cpu_sr);
        sb->s_flags &= ~MS_ACTIVE;
        return 1;
    }

    return 0;
}

void unfreezepart(__hdle h_sb)
{
    __u32           cpu_sr;
    struct super_block *sb = (struct super_block *)h_sb;

    if (!sb)
    {
        return;
    }

    cpu_sr = awos_arch_lock_irq();
    if (sb->s_frozen)
    {
        sb->s_frozen--;
        if (!sb->s_frozen)
        {
            sb->s_flags |= MS_ACTIVE;
        }
    }
    awos_arch_unlock_irq(cpu_sr);
}
