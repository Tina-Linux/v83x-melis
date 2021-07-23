/*
*********************************************************************************************************
*                                                    MELIS
*                                    the Easy Portable/Player Develop Kits
*                                                  File System
*
*                                    (c) Copyright 2011-2014, Sunny China
*                                             All Rights Reserved
*
* File    : dir.c
* By      : Sunny
* Version : v1.0
* Date    : 2011-1-15
* Descript: directory operations of vfs, code is extracted from linux.
* Update  : date                auther      ver     notes
*           2011-3-15 15:10:18  Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#include "file.h"
#include "fs.h"
#include "err.h"
#include "slab.h"
#include "fs_dirent.h"
#include "fsys_debug.h"
#include "support.h"
#include <string.h>

extern int nr_files;

extern struct file *do_sys_open(const char *filename, int flags, int mode, struct super_block *sb);
extern __s32 filp_close(struct file *pfile);
extern int sys_mkdirat(const char *pathname, struct super_block *sb, int mode);
extern int do_rmdir(const char *pathname, struct super_block *sb);

struct __dirstream
{
    size_t allocation;  /* Space allocated for the block.  */
    size_t size;        /* Total valid data in the block.  */
    size_t offset;      /* Current offset into the block.  */
    char   data[0];     /* Directory block.  */
};

struct dents_buf
{
    __fsys_dirent_t out;
    struct __dirstream dstream;
};

__hdle esFSYS_opendir(const char *pDirName)
{
    struct super_block  *sb;
    struct file *retval;
    const char *realname;
    int cpu_sr, error = 0, temp_open_nr;

    fs_log_trace0("dopn:%s,", pDirName);
    if (esFSYS_vfslock())
    {
        fs_log_warning("opendir err when enter vfs mutex\n");
        return NULL;
    }

    /* �򿪵��ļ������Ƿ��Ѿ��ﵽ���� */
    cpu_sr = awos_arch_lock_irq();
    temp_open_nr = ++nr_files;
    awos_arch_unlock_irq(cpu_sr);

    if (temp_open_nr >= NR_MAXOPEN)
    {
        fs_log_info("max file or dir handle reached!\n");
        error = -EMFILE;
        goto dec_out;
    }

    /* ��ȡ��pFileName·�����е��̷�ָ���ķ���ָ�룬����ȡ�̷������·���� */
    sb = path_to_sb(pDirName, &realname);
    if (!sb)
    {
        fs_log_info("part \"%c\" not exist!\n", *pDirName);
        error = -ENOENT;
        goto dec_out;
    }

    /* �����ļ�ϵͳ�򿪲����ڲ����� */
    retval = do_sys_open(realname, O_DIRECTORY, 0, sb);
    if (IS_ERR(PTR_ERR(retval)))
    {
        error = PTR_ERR(retval);
        goto dec_out;
    }

    if (retval)
    {
        fs_log_trace0("fd:%d,", retval->f_fd);
    }

    /* ����dirent�ռ� */
    retval->private_data = (void *)__get_free_page(0); // kmalloc(sizeof(__fsys_dirent_t), 0);
    if (retval->private_data == NULL)
    {
        fs_log_error("opendir, no space for dirent!\n");
        fput(retval);
        error = ENOMEM;
        goto dec_out;
    }
    else
    {
        struct dents_buf *de_buf = (struct dents_buf *)retval->private_data;

        memset(retval->private_data, 0x00, 1 << PAGE_SHIFT);
        de_buf->dstream.allocation = PAGE_SIZE -
                                     (sizeof(__fsys_dirent_t) + offsetof(struct __dirstream, data));
        de_buf->dstream.offset = de_buf->dstream.size = 0;
    }

    unmount_check(sb);

    /* �ɹ���Ŀ¼ */
    goto out;

dec_out:
    nr_files--;
out:
    if (error)
    {
        fs_log_warning("error(%d) meet in opendir: %s\n", error, pDirName);
        fs_err = error;
        retval = NULL;
    }
    esFSYS_vfsunlock();

    fs_log_trace0("O%d\n", error);
    return (__hdle)retval;
}

__s32 esFSYS_closedir(__hdle hDir)
{
    int retval = EPDK_FAIL;

    if (!hDir)
    {
        fs_err = EBADF;
        return EPDK_FAIL;
    }

    fs_log_trace0("clsd:%d,", ((struct file *)hDir)->f_fd);
    if (((struct file *)hDir)->f_dentry)
    {
        fs_log_objname("%s,", ((struct file *)hDir)->f_dentry->d_name.name);
    }

    if (esFSYS_vfslock())
    {
        fs_log_warning("close file err when enter vfs mutex\n");
        return EPDK_FAIL;
    }

    /* ���� dirent �ռ� */
    if (((struct file *)hDir)->private_data)
    {
        free_page(((struct file *)hDir)->private_data);
    }

    /* �رն��� */
    retval = filp_close((struct file *)hDir);
    unmount_check(NULL);

    /* �ļ�����һ */
    nr_files--;

    if (retval && retval != EPDK_FAIL)
    {
        fs_err = retval;
        retval = EPDK_FAIL;
    }
    esFSYS_vfsunlock();

    fs_log_trace0("O%d\n", retval);

    return retval;
}

__s32 esFSYS_mkdir(const char *pDirName)
{
    struct super_block  *sb;
    const char *realname;
    int error = 0;

    fs_log_trace0("mkd:%s,", pDirName);

    if (esFSYS_vfslock())
    {
        fs_log_warning("mkdir err when enter vfs mutex\n");
        return EPDK_FAIL;
    }

    /* ��ȡ��pFileName·�����е��̷�ָ���ķ���ָ�룬����ȡ�̷������·���� */
    sb = path_to_sb(pDirName, &realname);
    if (!sb)
    {
        fs_log_info("part \"%c\" not accessable!\n", *pDirName);
        error = -ENOENT;
        goto out;
    }

    /* ����Ŀ¼ */
    error = sys_mkdirat(realname, sb, 0);
    unmount_check(sb);

out:
    if (error && error != EPDK_FAIL)
    {
        fs_log_warning("mkdir return value:%d\n", error);
        fs_err = error;
        error = EPDK_FAIL;
    }
    esFSYS_vfsunlock();

    fs_log_trace0("O%d\n", error);
    return error;
}

__s32 esFSYS_rmdir(const char *pDirName)
{
    struct super_block  *sb;
    const char *realname;
    int error = 0;

    fs_log_trace0("rmd:%s,", pDirName);

    if (esFSYS_vfslock())
    {
        fs_log_warning("rmdir err when enter vfs mutex\n");
        return EPDK_FAIL;
    }

    /* ��ȡ��pFileName·�����е��̷�ָ���ķ���ָ�룬����ȡ�̷������·���� */
    sb = path_to_sb(pDirName, &realname);
    if (!sb)
    {
        fs_log_info("part \"%c\" not accessable!\n", *pDirName);
        error = -ENOENT;
        goto out;
    }

    error = do_rmdir(realname, sb);
    unmount_check(sb);

out:
    if (error && error != EPDK_FAIL)
    {
        fs_log_warning("rmdir return value:%d\n", error);
        fs_err = error;
        error = EPDK_FAIL;
    }
    esFSYS_vfsunlock();

    fs_log_trace0("O%d\n", error);
    return error;
}

//static int fillonedir(void * __buf, const char * name, int namlen, unsigned int d_type)
//{
//  __fsys_dirent_t* dirent = __buf;
//
//    strncpy(dirent->d_name, name, FSYS_DIRNAME_MAX);
//    dirent->d_name[FSYS_DIRNAME_MAX -1] = '\0';
//    dirent->fatdirattr = d_type;
//
//  return 0;
//}

struct getdents_callback
{
    struct linux_dirent64 *current_dir;
    struct linux_dirent64 *previous;
    int count;
};

/**
  *return value:
  *    0: fill ok
  *  <>0: buffer not enough
  */
static int filldir(void *__buf, const char *name, int namlen, int nametype,
                   __s64 offset, __u64 ino, unsigned int d_type, __u64 d_size)
{
    struct linux_dirent64 *dirent = NULL;
    struct getdents_callback *buf = (struct getdents_callback *) __buf;
    int reclen = FSYS_ALIGN(NAME_OFFSET(dirent) + namlen + 1, sizeof(__u64));

    if (reclen > buf->count)
    {
        return -EINVAL;
    }

    dirent = buf->previous;
    if (dirent)
    {
        dirent->d_off = offset;
    }
    dirent = buf->current_dir;
    dirent->d_ino = ino;
    dirent->d_off = 0;
    dirent->d_reclen = reclen;
    dirent->d_type = d_type;

    dirent->d_size = d_size;
    strncpy(dirent->d_name, name, namlen);
    dirent->d_name[namlen] = '\0';
    buf->previous = dirent;
    dirent = (struct linux_dirent64 *)((char *)dirent + reclen);
    buf->current_dir = dirent;
    buf->count -= reclen;
    return 0;
}

/**
  *return value of res:
  *     1: dirent buffer end reached
  *     0: no more dir to read
  *    <0: ENOMEM error or other fatal error, need stop to go on scaning the directory
  *return value of outcnt:
  *     out bytes count
  */
static long __getdents(struct file *dir, struct linux_dirent64 *dirent,
                       unsigned int count, unsigned int *outcnt)
{
    struct linux_dirent64 *lastdirent;
    struct getdents_callback buf;
    int res;

    buf.current_dir = dirent;
    buf.previous = NULL;
    buf.count = count;

    *outcnt = 0;

    /* ��ȡĿ¼ */
    res = dir->f_op->readdir(dir, &buf, filldir);

    file_accessed(dir);
    lastdirent = buf.previous;
    if (lastdirent)
    {
        lastdirent->d_off = dir->f_pos;
        *outcnt = count - buf.count;
    }

    return res;
}

static __fsys_dirent_t *__getonedir(struct file *dir)
{

    struct __dirstream *dirbuf = &((struct dents_buf *)dir->private_data)->dstream;
    __fsys_dirent_t    *out    = &((struct dents_buf *)dir->private_data)->out;
    struct linux_dirent64 *dp;

    do
    {
        if (dirbuf->offset >= dirbuf->size)
        {
            unsigned int bytes;
            int res;

            res = __getdents(dir, (struct linux_dirent64 *)dirbuf->data, dirbuf->allocation, &bytes);
            if (res < 0)
                fs_log_warning("error met when read dir %s(%d)!\n",
                               dir->f_dentry->d_name.name, fs_err);
            if (bytes == 0)
            {
                out = NULL;
                goto out;
            }

            dirbuf->size = (size_t) bytes;
            dirbuf->offset = 0;
        }

        dp = (struct linux_dirent64 *) &dirbuf->data[dirbuf->offset];
        dirbuf->offset += dp->d_reclen;
    } while (dp->d_ino == 0);  /* Skip deleted files.  */

    out->d_ino = dp->d_ino;
    out->fatdirattr = dp->d_type;
    out->d_size = dp->d_size;
    strncpy(out->d_name, dp->d_name, FSYS_DIRNAME_MAX);

out:
    return out;
}

__hdle esFSYS_readdir(__hdle hDir)
{
    __hdle retval;

    fs_log_trace0("dr:%d,", ((struct file *)hDir)->f_fd);
    if (((struct file *)hDir)->f_dentry)
    {
        fs_log_objname("%s,", ((struct file *)hDir)->f_dentry->d_name.name);
    }

    if (esFSYS_vfslock())
    {
        fs_log_warning("readdir err when enter vfs mutex\n");
        return NULL;
    }

    if (!((struct file *)hDir)->f_dentry)
    {
        fs_err = -EDEADLK;
        fs_log_warning("dir is not accessable.\n");
        goto err_out;
    }

    /* Ŀ¼�����Ӧ�Ľڵ���fread���� */
    if (!((struct file *)hDir)->f_op || !((struct file *)hDir)->f_op->readdir)
    {
        fs_log_error("dir has no readdir method.\n");
        fs_err = -ENOTDIR;
        goto err_out;
    }

    /* �Ƿ�������Ŀ¼ */
    if (IS_DEADDIR(((struct file *)hDir)->f_dentry->d_inode))
    {
        fs_err = -ENOENT;
        fs_log_warning("%s is deleted.\n", ((struct file *)hDir)->f_dentry->d_name.name);
        goto err_out;
    }

    /* ��ȡĿ¼ */
    retval = __getonedir((struct file *)hDir);
    goto out;

err_out:
    retval = NULL;
    ((struct file *)hDir)->f_err = fs_err;

out:
    esFSYS_vfsunlock();
    if (retval)
    {
        fs_log_trace0("%s,O\n", ((__fsys_dirent_t *)retval)->d_name);
    }
    else
    {
        fs_log_trace0("O\n");
    }

    return retval;
}

void esFSYS_rewinddir(__hdle hDir)
{
    struct dents_buf *de_buf;

    fs_log_trace0("rwnd:%d,", ((struct file *)hDir)->f_fd);
    if (((struct file *)hDir)->f_dentry)
    {
        fs_log_objname("%s,", ((struct file *)hDir)->f_dentry->d_name.name);
    }

    if (esFSYS_vfslock())
    {
        fs_log_warning("rewinddir err when enter vfs mutex\n");
        return;
    }

    if (!((struct file *)hDir)->f_dentry)
    {
        fs_err = -EDEADLK;
        fs_log_warning("dir is not accessable.\n");
        goto out;
    }

    ((struct file *)hDir)->f_pos = 0;

    de_buf = (struct dents_buf *)(((struct file *)hDir)->private_data);

    de_buf->dstream.allocation = PAGE_SIZE -
                                 (sizeof(__fsys_dirent_t) + offsetof(struct __dirstream, data));
    de_buf->dstream.offset = de_buf->dstream.size = 0;

out:
    esFSYS_vfsunlock();
    fs_log_trace0("O\n");
}
