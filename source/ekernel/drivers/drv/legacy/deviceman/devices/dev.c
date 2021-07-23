/*
**********************************************************************************************************************
*                                                   ePOS
*                                  the Easy Portable/Player Operation System
*
*                                (c) Copyright 2007-2008, Steven.ZGJ.China
*                                           All Rights Reserved
*
* File    : devman.c
* By      : steven.ZGJ
* Version : V1.00
* update  : kevin.z.m 2010-9-7 17:05, clean code
**********************************************************************************************************************
*/
#include "dev.h"
#include <log.h>
#include <port.h>
#include <kapi.h>

/*
**********************************************************************************************************************
*                                     esDEV_Open
*
* Description: module init function, this function will be called by system module management: MInstall,
*               user will never call it.
* Arguments  : void
*
* Returns    : if success return EPDK_OK
*              esle return EPDK_FAIL
**********************************************************************************************************************
*/
__hdle esDEV_Open(__hdle hNode, __u32 Mode)
{
    __dev_dev_t      *pDev;
    __hdle            hDev;
    __dev_node_t     *pNode = (__dev_node_t *)hNode;

    if (pNode->status == DEV_STAT_INACTIVE)
    {
        __wrn("Device has been killed when unreg!");
        return NULL;
    }

    if (pNode->opentimes == 0)
    {
        hDev = pNode->DevOp.Open(pNode->pOpenArg, Mode);/* ���豸                                                     */
        if (hDev == NULL)
        {
            __wrn("dev cannot be open!");
            return NULL;
        }
        pNode->hDev = hDev;                         /* �����豸���                                                 */
    }                                               /* �����豸�������ռ�                                         */

    pDev = (__dev_dev_t *)k_malloc(sizeof(__dev_dev_t));
    if (pDev == NULL)
    {
        __err("alloc structure failure");
        return NULL;
    }

    pDev->devnode = pNode;                          /* ��¼nodeָ��                                                 */
    pDev->DevOp   = pNode->DevOp;                     /* �豸������ڣ�ֱ�Ӵ�__dev_node_t��copy���Է���ʹ��           */
    pDev->hDev    = pNode->hDev;                       /* �豸�����ֱ�Ӵ�__dev_node_t��copy���Է���ʹ��               */

    pNode->opentimes ++;                             /* �豸�ڵ�򿪴�����1                                          */

    return pDev;                            /* �����豸������                                             */
}
/*
**********************************************************************************************************************
*                                     esDEV_Close
*
* Description: �ر��豸
*
* Arguments  : void
*
* Returns    : if success return EPDK_OK
*              esle return EPDK_FAIL
**********************************************************************************************************************
*/
__s32  esDEV_Close(__hdle hDev)
{
    __dev_dev_t  *pDev  = (__dev_dev_t *)hDev;
    __dev_node_t *pNode = pDev->devnode;
    __u8          err;

    if (pNode->opentimes)
    {
        pNode->opentimes--;    /* �豸�ڵ�򿪴�����1                                          */
    }

    if (pNode->opentimes == 0)                      /* ����豸�ڵ��Ѿ�û������ʹ��                                 */
    {
        /* ����豸�ڵ�û���û�ʹ�ã�ͬʱ���������ڵ㣬����ע��Ĳ��� */
        if (pNode->status == DEV_STAT_INACTIVE)
        {
            __dev_node_t *p, **pp = &(pNode->classnode->nodelist);

            for (p = *pp; p && (pNode != p); pp = &(p->next), p = p->next);

            if (!p)
            {
                __wrn("BUG when close dev: try to destroy a devnode not exsit in node list!");
                return EPDK_FAIL;
            }

            *pp = pNode->next;

            /* �ͷ��豸�ڵ�ռ�õĿռ�               */
            esKRNL_SemDel(pNode->sem, 0, &err);
            k_free((void *)pNode);
        }
        else                                        /* �����Ѿ�û���û��򿪴��豸�ڵ㣬ͬʱ���豸�ڵ㲻�����ڵ�   */
        {
            if (pDev->DevOp.Close(pNode->hDev) == EPDK_FAIL) /* �ر��豸���                                         */
            {
                __err("close device[%s] fail!", pNode->name);
            }
        }
    }

    k_free((void *)pDev);              /* �ͷ��豸���ռ�õĿռ�                                       */

    return EPDK_OK;
}
/*
**********************************************************************************************************************
*                                     esDEV_Read
*
* Description: ��
*
* Arguments  :  pdata       ��Ҫ����������ָ��
*               size        ��Ĵ�С
*               n           ����
*               hDev        �豸���
*
* Returns    : ʵ�ʶ����Ŀ���
*
**********************************************************************************************************************
*/
__u32  esDEV_Read(void *pdata, __u32 size, __u32 n, __hdle hDev)
{
    __dev_node_t *pNode = ((__dev_dev_t *)hDev)->devnode;

    if (pNode->status == DEV_STAT_INACTIVE)
    {
        __wrn("Device has been killed when unreg!");
        return 0;
    }

    return (((__dev_dev_t *)hDev)->DevOp.Read)(pdata, size, n, ((__dev_dev_t *)hDev)->hDev);
}


/*
**********************************************************************************************************************
*                                     esDEV_Write
*
* Description: д
*
* Arguments  :  pdata       ��Ҫд�������ָ��
*               size        ��Ĵ�С
*               n           ����
*               hDev        �豸���
*
* Returns    : ʵ��д��Ŀ���
*
**********************************************************************************************************************
*/
__u32  esDEV_Write(const void *pdata, __u32 size, __u32 n, __hdle hDev)
{
    __dev_node_t *pNode = ((__dev_dev_t *)hDev)->devnode;


    if (pNode->status == DEV_STAT_INACTIVE)
    {
        __wrn("Device has been killed when unreg!");
        return 0;
    }

    return (((__dev_dev_t *)hDev)->DevOp.Write)(pdata, size, n, ((__dev_dev_t *)hDev)->hDev);
}


/*
**********************************************************************************************************************
*                                     esDEV_Ioctrl
*
* Description: �豸����
*
* Arguments  : hDev         �豸���
*              cmd          ����
*              aux          ����
*              pbuffer      ����buffer
*
* Returns    : device defined
*
**********************************************************************************************************************
*/
__s32  esDEV_Ioctl(__hdle hDev, __u32 cmd, __s32 aux, void *pbuffer)
{
    __dev_node_t *pNode = ((__dev_dev_t *)hDev)->devnode;

    if (pNode->status == DEV_STAT_INACTIVE)
    {
        __wrn("Device has been killed when unreg!");
        return EPDK_FAIL;
    }

    if (IS_DEVIOCSYS(cmd))
    {
        switch (cmd)
        {
            case DEV_IOC_SYS_GET_CLSNAME:
                *((int *)pbuffer) = (int)pNode->classnode->name;
                return EPDK_OK;

            case DEV_IOC_SYS_GET_DEVNAME:
                *((int *)pbuffer) = (int)pNode->name;
                return EPDK_OK;
            case DEV_IOC_SYS_GET_ATTRIB:
                *((int *)pbuffer) = pNode->attrib;
                return EPDK_OK;
            case DEV_IOC_SYS_GET_LETTER:
                *((char *)pbuffer) = pNode->pletter;
                return EPDK_OK;
            case DEV_IOC_SYS_GET_OPENARGS:
                *((int *)pbuffer) = (int)pNode->pOpenArg;
                return EPDK_OK;
            default:
                return EPDK_FAIL;
        }
    }
    else
        return (((__dev_dev_t *)hDev)->DevOp.Ioctl)
               (((__dev_dev_t *)hDev)->hDev, cmd, aux, pbuffer);
}

