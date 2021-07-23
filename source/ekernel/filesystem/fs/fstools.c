/*
*********************************************************************************************************
*                                                    MELIS
*                                    the Easy Portable/Player Develop Kits
*                                                  File System
*
*                                    (c) Copyright 2011-2014, Sunny China
*                                             All Rights Reserved
*
* File    : fstools.c
* By      : Sunny
* Version : v1.0
* Date    : 2011-1-15
* Descript: vfs tools, code is extracted from linux.
* Update  : date                auther      ver     notes
*           2011-3-15 15:25:05  Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#include "sys_fsys_i.h"
#include "err.h"
#include "fsys_debug.h"
#include "nls.h"
#include <string.h>

extern struct file_system_type *pFSRoot;

/*
**********************************************************************************************************************
*
*             esFSYS_format
*
*  Description:
*  format one partiton
*
*  Parameters:
*   ppartName   - like 'c' or 'd' etc.
*   fstype      - wanna filesystem type
*
*  Return value:
*   <0          - Unable to find the device.
*   >=0         - Index of the device in the device information table.
**********************************************************************************************************************
*/
__s32 esFSYS_format(const char *partname, const char *fstype, __hdle fmtpara)
{
    __u8   err;
    int    res, semap_on = 0;
    __hdle pPart = NULL;
    struct file_system_type *pFS;

    fs_log_trace0("fmt:%s,%s", partname, fstype);

    if (esFSYS_vfslock())
    {
        fs_log_error("format error when enter vfs mutex\n");
        return EPDK_FAIL;
    }

    semap_on = 1;

    /* ʶ���ļ�ϵͳ                 */
    for (pFS = pFSRoot; pFS; pFS = pFS->next)
    {
        if (!strcmp(pFS->name, fstype))
        {
            if (pFS->format)
            {
                break;
            }
            else
            {
                fs_log_info("format failed!\n%s filesystem has no format function!\n", fstype);
                res = EPDK_FAIL;
                goto out;
            }
        }
    }
    if (!pFS)
    {
        fs_log_info("format failed!\n%s filesystem not found!maybe you need to install the filesystem first.\n", fstype);
        res = EPDK_FAIL;
        goto out;
    }
    esFSYS_vfsunlock();
    semap_on = 0;

    fs_log_info("open part \"%s\"\n", partname);
    pPart = esFSYS_popen(partname, "r+");
    if (!pPart)
    {
        fs_log_info("format failed!\npart %s can not be open!\n", partname);
        res = EPDK_FAIL;
        goto out;
    }

    if (esFSYS_vfslock())
    {
        fs_log_error("format error when enter vfs mutex\n");
        return EPDK_FAIL;
    }
    semap_on = 1;

    res = pFS->format((__hdle)pPart, fmtpara);
    if (res)
    {
        fs_log_info("format failed!\n");
        res = EPDK_FAIL;
        goto out;
    }

    fs_log_info("complete.\n\n");
    res = EPDK_OK;

out:
    if (semap_on)
    {
        esFSYS_vfsunlock();
    }
    if (pPart)
    {
        esFSYS_pclose(pPart);
    }

    fs_log_trace0("O\n");
    return res;
}

__s32 esFSYS_statfs(const char *path, __hdle buf, __u32 flags)
{
    char               *s;
    int                 res;
    struct super_block *sb;
    __u8                err;
    struct kstatfs     *stat = (struct kstatfs *)buf;

    fs_log_trace0("stafs:%s,", path);

    if (esFSYS_vfslock())
    {
        fs_log_error("statfs error when enter vfs mutex\n");
        return EPDK_FAIL;
    }

    /* ��ȡ��pFileName·�����е��̷�ָ���ķ���ָ�룬����ȡ�̷������·���� */
    sb = path_to_sb(path, NULL);
    if (!sb)
    {
        fs_log_warning("part \"%c\" not accessable!\n", *path);
        res = EPDK_FAIL;
        fs_err = -ENOENT;
        goto out;
    }

    if (sb->s_op->statfs)
    {
        res = sb->s_op->statfs(sb, stat, flags);
    }
    else
    {
        res = EPDK_FAIL;
        fs_err = -EPERM;
        fs_log_warning("no statfs operation!\n");
    }

out:
    esFSYS_vfsunlock();
    fs_log_trace0("O\n");
    return res;
}

/*
**********************************************************************************************************************
*
*             esFSYS_partfslock
*
*  Description:
*               - lock one disk from been fs-unmounted.
*
*  Parameters:
*   diskname    - disk letter follow up with ":\", such as "c:\" , 'd:\' etc.
*
*  Return value:
*   EPDK_OK     - ok
*   EPDK_FAIL   - some thing error
**********************************************************************************************************************
*/
__s32 esFSYS_partfslck(char *partname)
{
    __u8                err;
    __s32               res;
    struct super_block *sb;
    struct inode       *ino;

    fs_log_trace0("pfslk:%s", partname);

    if (esFSYS_vfslock())
    {
        fs_log_error("partfslck error when enter vfs mutex\n");
        return EPDK_FAIL;
    }

    sb = path_to_sb(partname, NULL);
    if (!sb)
    {
        fs_log_warning("part \"%c\" can not be accessed!\n", *partname);
        res = EPDK_FAIL;
        fs_err = -ENOENT;
        goto out;
    }

    ino = igrab(sb->s_root->d_inode);
    if (ino)
    {
        res = EPDK_OK;
    }
    else
    {
        res = EPDK_FAIL;
    }

out:
    esFSYS_vfsunlock();

    fs_log_trace0("%d\n", res);
    return res;
}

/*
**********************************************************************************************************************
*
*             esFSYS_partfsunlock
*
*  Description:
*               - give up hold the fs of one disk. the disk then can be unmount from fs
*
*  Parameters:
*   diskname    - disk letter follow up with ":\", such as "c:\" , 'd:\' etc.
*
*  Return value:
*   EPDK_OK     - ok
*   EPDK_FAIL   - some thing error.
**********************************************************************************************************************
*/
__s32 esFSYS_partfsunlck(char *partname)
{
    __u8                err;
    __s32               res;
    struct super_block *sb;

    fs_log_trace0("pfsulk:%s", partname);

    if (esFSYS_vfslock())
    {
        fs_log_error("partfsunlck error when enter vfs mutex\n");
        return EPDK_FAIL;
    }

    sb = path_to_sb(partname, NULL);
    if (!sb)
    {
        fs_log_warning("part \"%c\" can not be accessed!\n", *partname);
        res = EPDK_FAIL;
        fs_err = -ENOENT;
        goto out;
    }

    if (atomic_read(&sb->s_root->d_inode->i_count) > 1)
    {
        iput(sb->s_root->d_inode);
    }

    res = EPDK_OK;

out:
    esFSYS_vfsunlock();

    fs_log_trace0("O%d\n", res);
    return res;
}

__s32 esFSYS_setfs(char *partname, __u32 cmd, __s32 aux, char *para)
{
    struct nls_table   *new_nls;

    fs_log_trace0("setfs: path(%s), cmd(%d), para(%s)", partname, cmd, para);

    if (esFSYS_vfslock())
    {
        fs_log_error("setfs error when enter vfs mutex\n");
        return EPDK_FAIL;
    }

    new_nls = load_nls(aux);
    if (new_nls)
    {
        fs_log_info("Set nls to [%d]\n", aux);
        unload_nls(nls);
        nls = new_nls;
    }

    esFSYS_vfsunlock();
    fs_log_trace0("O\n");
    return EPDK_OK;
}

__s32 esFSYS_getfscharset(const char *partname, __s32 *pCharset)
{
    __s32               ret = EPDK_OK;
    __u8                err = 0;

    fs_log_trace0("getfscharset: path(%s)", partname);

    if (esFSYS_vfslock())
    {
        fs_log_error("getfscharset error when enter vfs mutex\n");
        return EPDK_FAIL;
    }

    /* ��ȡ�ļ�ϵͳ��ǰ�ı��� */
    if (pCharset)
    {
        *pCharset = get_current_charset();
    }
    else
    {
        ret = EPDK_FAIL;
    }
    esFSYS_vfsunlock();
    fs_log_trace0("O\n");
    return ret;
}
