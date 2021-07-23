/*
*********************************************************************************************************
*                                                   ePOS
*                               the Easy Portable/Player Operation System
*                                              eMOD sub-system
*
*                               (c) Copyright 2006-2007, Steven.ZGJ China
*                                           All Rights Reserved
*
* File   : devman_private.h
* Version: V1.0
* By     : steven.zgj
*********************************************************************************************************
*/
#ifndef __DEV_H__
#define __DEV_H__
#include <typedef.h>
#include <sys_device.h>

#define  NODETYPE_CLASS         0x00000000
#define  NODETYPE_DEV           0x00000001

#define  DEV_STAT_INACTIVE      0x00
#define  DEV_STAT_ACTIVE        0x01

typedef struct __DEV_NODE       __dev_node_t;
typedef struct __DEV_CLASSNODE  __dev_classnode_t;

/* �豸��ڵ�   */
struct __DEV_CLASSNODE
{
    __dev_classnode_t        *next;
    __dev_node_t             *nodelist;

    int                       nodetype;
    __u8                      usedcnt;
    char                     *name;
};

/* �豸�ڵ������˾�����ڱ���һ���豸������Ϣ�����ݽṹ   */
struct  __DEV_NODE
{
    __dev_node_t             *next;
    __dev_classnode_t        *classnode;            /* ��ڵ�                                                       */

    int                       attrib;
    char                     *name;
    char                      pletter;

    __u32                     opentimes;            /* ����򿪵Ĵ���                                               */
    __u32                     status;               /* �ڵ�״̬��0��ʾ������0xff��ʾΪ���ڵ㣨������������Ѿ�ж�أ�*/

    __dev_devop_t             DevOp;                /* �����豸�������                                           */
    void                     *pOpenArg;

    __hdle                    sem;
    __hdle                    hDev;                 /* �豸���                                                     */

};

/*
******************************************************
* �豸�ڵ����������˾�������û���һ���豸�ڵ�ʱ��
* �صľ��������ͬ���豸����������豸�ڵ��ʵ��������
* ��������豸��ʵ����
******************************************************
*/
typedef struct __DEV_DEV
{
    __dev_node_t             *devnode;              /* ָ���豸�ڵ�                                                 */
    __hdle                    hDev;                 /* �豸�����ֱ�Ӵ�__dev_node_t��copy���Է���ʹ��               */
    __dev_devop_t             DevOp;                /* �豸������ڣ�ֱ�Ӵ�__dev_node_t��copy���Է���ʹ��           */
} __dev_dev_t;

#endif //#ifndef __DEV_H__

