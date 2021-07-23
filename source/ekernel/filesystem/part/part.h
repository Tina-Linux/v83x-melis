/*
*********************************************************************************************************
*                                                    MELIS
*                                    the Easy Portable/Player Develop Kits
*                                                  File System
*
*                                    (c) Copyright 2011-2014, Sunny China
*                                             All Rights Reserved
*
* File    : part.h
* By      : Sunny
* Version : v1.0
* Date    : 2011-3-15
* Descript: partition management of file system.
* Update  : date                auther      ver     notes
*           2011-3-15 14:58:06  Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#ifndef __PART_H__
#define __PART_H__
#include <sys_fsys.h>
#include <sys_device.h>
#include "sys_fsys_i.h"

extern __hdle pPartSem;

typedef struct __FSYS_PART              __fsys_part_t;      /* ����         */
typedef struct fix_part_s               __fix_part_t;
typedef struct __FSYS_PD                __fsys_pd_t;        /* ����������� */
typedef struct __FSYS_PDOPS             __fsys_pdops_t;     /* ��������     */

#define     FSYS_PARTSTATUS_UNUSED      (1<<0)      /* �ļ�ϵͳʹ��    */
#define     FSYS_PARTSTATUS_FSUSED      (1<<1)      /* �ļ�ϵͳʹ��    */
#define     FSYS_PARTSTATUS_RAWUSED     (1<<2)      /* ֱ�ӷ���ʹ��    */
#define     FSYS_PARTSTATUS_DEAD        (1<<3)      /* �˷����Ѿ���ж��*/
#define     FSYS_PARTSTATUS_INVALID     (FSYS_PARTSTATUS_UNUSED | FSYS_PARTSTATUS_DEAD)

#define     FSYS_PARTATTR_OPENRWM       0x03
#define     FSYS_PARTATTR_OPENR         (1<<0)      /* �����Կɶ��� */
#define     FSYS_PARTATTR_OPENW         (1<<1)      /* �����Կɶ��� */
#define     FSYS_PARTATTR_DEVRWM        0x0c
#define     FSYS_PARTATTR_DEVR          (1<<2)      /* �����Դ��Ŀɶ����� */
#define     FSYS_PARTATTR_DEVW          (1<<3)      /* �����Դ��Ŀ�д���� */
#define     FSYS_PARTATTR_FS            (1<<4)      /* �������п�ʶ����ļ�ϵͳ */

#define     PD_STAT_INACTIVE            0x00
#define     PD_STAT_ACTIVE              0x01

/*
**********************************************************************************************************************
* struct: part
*/
struct __FSYS_PDOPS
{
    __u32(*read)(void *pData, __u32 Sector, __u32 N, __fsys_part_t *pPart);
    __u32(*write)(const void *pData, __u32 Sector, __u32 N, __fsys_part_t *pPart);
    __s32(*ioctl)(__fsys_part_t *pPart, __u32 Cmd, __s32 Aux, void *pBuffer);

    __s32(*mount)(__fsys_part_t *pPart);
    __s32(*unmount)(__fsys_part_t *pPart, int force);
    __u32(*identify)(__hdle hDev);
    __s32(*fdisk)(__hdle hDev);
};

struct __FSYS_PD
{
    char                   *name;
    __u32                   nUsr;
    __u32                   status;     /* �ڵ�״̬��0��ʾ������0xff��ʾΪ���ڵ㣨������������Ѿ�ж�أ�*/
    __fsys_pd_t            *next;

    __fsys_pdops_t          Ops;        /* �������������� */
    //__u32                   last_lun;   //�Ƿ�u�̻򿨵����һ���̷�
};

struct __FSYS_PART
{
    char                    dname[MAX_PART_NAME_LEN];
    /* ������ */
    char                    letter;     /* �̷� */

    __u32                   status;     /* ����������FSYS_PARTSTATUS_XXX */
    __s32                   error;      /* ������������ļ�¼ */
    __s32                   attr;       /* access mode */

    __fsys_pd_t            *pPD;        /* �������� */
    __hdle                  hPDPrivate; /* ��������˽�����ݿ��� */
    __hdle                  hFSPrivate; /* �ļ�ϵͳ˽�����ݿ��� */

    __hdle                  hNode;      /* �豸�ڵ��� */
    __hdle                  hDev;       /* �豸�����Ĳ������ */
    __u32                   Unit;       /* �豸�ڷ����� */
    __bool                  updateflag; /* �������±�� */
    __u32                   last_lun;   //�Ƿ�u�̻򿨵����һ���̷�
};

struct fix_part_s
{
    char                letter;
    struct __FSYS_PART *part;
};

extern int deactivate_part(__fsys_part_t *part, int force);

#endif  /* __PART_H__ */
