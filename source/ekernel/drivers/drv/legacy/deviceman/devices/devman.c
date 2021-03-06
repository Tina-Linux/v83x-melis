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
* update  : kevin.z.m, 2010-9-7 17:08, clear code.
**********************************************************************************************************************
*/
#include "dev.h"
#include <kapi.h>
#include <string.h>
#include <port.h>
#include <log.h>

extern __hdle    pDevSem;            /* for lock parts table */

__dev_classnode_t devclass_nop = {.name = DEV_CLASS_NOP, .nodetype = NODETYPE_CLASS,};

/*
**********************************************************************************************************************
*                                     SetDevAttr
*
* Description: set dev node attribute.
*
* Arguments  : class name
*              dev name
*              devnode
*
* Returns    : always EPDK_OK
**********************************************************************************************************************
*/
__s32 SetDevAttr(char *classname, char *devname, __dev_node_t *devnode)
{
    devnode->attrib = 0;

    if (!strcmp(classname, DEV_CLASS_DMS))
    {
        devnode->attrib |= DEV_NODE_ATTR_CTL
                           | DEV_NODE_ATTR_PART
                           | DEV_NODE_ATTR_FS
                           | DEV_NODE_ATTR_SYNMNT;
        devnode->pletter = PART_LETTER_DMS;
    }
    else if (!strcmp(classname, DEV_CLASS_DISK))
    {
        devnode->attrib |= DEV_NODE_ATTR_RD
                           | DEV_NODE_ATTR_WR
                           | DEV_NODE_ATTR_BLK
                           | DEV_NODE_ATTR_PART
                           | DEV_NODE_ATTR_FS;
        devnode->pletter = PART_LETTER_FREE;

        if (!strcmp(devname, DEV_NAME_RAMDISK))
        {
            devnode->pletter = PART_LETTER_RAMDISK;
            devnode->attrib |= DEV_NODE_ATTR_SYNMNT;
        }
        else if (!strcmp(devname, DEV_NAME_ROOTFS))
        {
            devnode->pletter = PART_LETTER_ROOTFS;
            devnode->attrib |= DEV_NODE_ATTR_SYNMNT;
        }
        else if (!strcmp(devname, DEV_NAME_SYSDATAFS))
        {
            devnode->pletter = PART_LETTER_SYSDATA;
            devnode->attrib |= DEV_NODE_ATTR_SYNMNT;
        }
        else if (!strcmp(devname, DEV_NAME_SYSBOOTFS))
        {
            devnode->pletter = PART_LETTER_SYSBOOT;
            devnode->attrib |= DEV_NODE_ATTR_SYNMNT;
        }
        else if (!strcmp(devname, DEV_NAME_BOOTFS))
        {
            devnode->pletter = PART_LETTER_BOOTFS;
            devnode->attrib |= DEV_NODE_ATTR_SYNMNT;
        }
        else if (!strncmp(devname, DEV_NAME_USERDISK, strlen(DEV_NAME_USERDISK)))
        {

            /* ????????????????????????????????"USERDISKxx",
             * ????xx????00~99????????????????????????????.
             * ????????????????????W~U??????????????????????(W~U)????.
             */
            devnode->pletter = PART_LETTER_USER;
            devnode->attrib |= DEV_NODE_ATTR_SYNMNT;
        }
        else if (!strcmp(devname, DEV_NAME_SDCARD0) ||
                 !strcmp(devname, DEV_NAME_SDCARD1))
        {
            devnode->attrib |= DEV_NODE_ATTR_MOVABLE
                               | DEV_NODE_ATTR_USBSLOT
                               | DEV_NODE_ATTR_SYNMNT;
        }
        else if (!strcmp(devname, DEV_NAME_SCSI_DISK_00))
        {
            devnode->attrib |= DEV_NODE_ATTR_MOVABLE
                               | DEV_NODE_ATTR_USBSLOT;
        }
    }
    else
    {
        devnode->attrib  |= DEV_NODE_ATTR_RD
                            | DEV_NODE_ATTR_WR
                            | DEV_NODE_ATTR_CTL
                            | DEV_NODE_ATTR_SYNMNT;
    }

    return EPDK_OK;
}

/*
**********************************************************************************************************************
*                                     esDEV_DevReg
*
* Description: module init function, this function will be called by system module management: MInstall,
*               user will never call it.
* Arguments  : void
*
* Returns    : if success return EPDK_OK
*              esle return EPDK_FAIL
**********************************************************************************************************************
*/
__hdle esDEV_DevReg(const char *classname, const char *name, const __dev_devop_t *pDevOp, void *pOpenArg)
{
    __u8               newclassflag = 0;
    __hdle             res;
    __dev_node_t      *devnode;
    __dev_node_t      *predevnode;
    __u8               err;

    __dev_classnode_t *classnode    = &devclass_nop;
    __dev_classnode_t *preclassnode = &devclass_nop;

    esKRNL_SemPend(pDevSem, 0, &err);
    if (err)
    {
        __err("pend semphore faiure.");
        return NULL;
    }

    /* step 1: ????class??????????????????????????????????????                     */
    while (classnode)
    {
        if (strcmp(classname, classnode->name) == 0)
        {
            break;
        }
        preclassnode = classnode;
        classnode = classnode->next;
    }
    if (classnode == 0)
    {
        /*????????                              */
        classnode = k_malloc(sizeof(__dev_classnode_t) + strlen(classname) + 1/*'/0'*/);
        if (classnode == 0)
        {
            __wrn("k_malloc fail!");
            res = NULL;
            goto out;
        }
        /* ??classnode??????list??              */
        preclassnode->next = classnode;
        classnode->next    = 0;

        /* copy name into classnode             */
        classnode->nodetype = NODETYPE_CLASS;
        classnode->name = (char *)((__u32)classnode + sizeof(__dev_classnode_t));
        strcpy(classnode->name, classname);

        /* nodelist is empty                    */
        classnode->nodelist = 0;

        /* new class is created                 */
        newclassflag = 1;
    }
    /* step 2: ????????list????????????????????????????????????????                                                 */
    devnode = classnode->nodelist;
    predevnode = 0;

    while (devnode)
    {
        if (strcmp(devnode->name, name) == 0)
        {
            break;
        }
        predevnode = devnode;
        devnode = devnode->next;
    }
    if (devnode != 0)
    {
        __wrn("dev is already registered, or name conflict!");
        res = NULL;
        goto out;
    }

    /* step 3: ????sizeofdrv????????????                                                                            */
    devnode = k_malloc(sizeof(__dev_node_t) + strlen(name) + 1/*'/0'*/);
    if (devnode == 0)
    {
        __wrn("k_malloc fail!");
        if (newclassflag)       /* ????class????????????????????class list */
        {
            k_free(classnode);
            preclassnode->next = 0;
        }
        res = NULL;
        goto out;
    }

    //??????????
    devnode->name = (char *)((__u32)devnode + sizeof(__dev_node_t));
    strcpy(devnode->name, name);
    //??????????
    devnode->next = 0;
    //????class
    devnode->classnode = classnode;
    //??????????0
    devnode->opentimes = 0;
    //??????
    devnode->status = DEV_STAT_ACTIVE;
    //????device????
    devnode->DevOp = *pDevOp;
    /* save open arg                                            */
    devnode->pOpenArg = pOpenArg;

    devnode->hDev = 0;

    devnode->sem = esKRNL_SemCreate(1);
    if (devnode->sem == NULL)
    {
        __err("create sem. failure");
        res = NULL;
        goto out;
    }

    /* ??????list??(link to rear of the list                    */
    if (predevnode == 0)
    {
        classnode->nodelist = devnode;
    }
    else
    {
        predevnode->next = devnode;
    }

    __inf("device \"%s\\%s\" is setup.", classname, name);

    /* step 4: ?????????????????? */
    SetDevAttr((char *)classname, (char *)name, devnode);

    /* step 5: ??????????????????????                           */
    if (devnode->attrib & DEV_NODE_ATTR_PART)
    {
        esFSYS_mntparts((__hdle)devnode);
    }

    res = (__hdle)devnode;

out:
    esKRNL_SemPost(pDevSem);
    return res;
}

/*
**********************************************************************************************************************
*                                     esDEV_DevUnreg
*
* Description: module init function, this function will be called by system module management: MInstall,
*               user will never call it.
* Arguments  : void
*
* Returns    : if success return EPDK_OK
*              esle return EPDK_FAIL
**********************************************************************************************************************
*/

__s32  esDEV_DevUnreg(__hdle hNode)
{
    __s32  res;
    __dev_node_t  *pNode = (__dev_node_t *)hNode;
    __u8           err;

    esKRNL_SemPend(pDevSem, 0, &err);
    if (err)
    {
        __err("create semphore faile");
        return EPDK_FAIL;
    }

    /* ????????????                                                                 */
    esFSYS_umntparts(hNode, 1);

    if (pNode->opentimes == 0)                      /* ????node????????????????????????node??????????               */
    {
        __dev_node_t *p, **pp = &(pNode->classnode->nodelist);

        for (p = *pp; p && (pNode != p); pp = &(p->next), p = p->next);

        if (!p)
        {
            __wrn("BUG when unres dev: try to destroy a devnode not exsit in node list!");
            res = EPDK_FAIL;
            goto out;
        }
        *pp = pNode->next;

        esKRNL_SemDel(pNode->sem, 0, &err);
        k_free((void *)pNode); /* ????node????                                                 */
        res = EPDK_OK;
        goto out;
    }
    else                                            /* ????????????node??????????????node                           */
    {
        pNode->DevOp.Close(pNode->hDev);            /* ????????????                                                 */
        pNode->status = DEV_STAT_INACTIVE;          /* ??????????????????                                           */
    }

    res = EPDK_OK;

out:
    esKRNL_SemPost(pDevSem);
    return res;
}

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
__s32 esDEV_Lock(__hdle hNode)
{
    esKRNL_SemPend(((__dev_node_t *)hNode)->sem, 0, NULL);
    return EPDK_OK;
}
/*
**********************************************************************************************************************
*                                     esDEV_Close
*
* Description: ????????
*
* Arguments  : void
*
* Returns    : if success return EPDK_OK
*              esle return EPDK_FAIL
**********************************************************************************************************************
*/
__s32  esDEV_Unlock(__hdle hNode)
{
    esKRNL_SemPost(((__dev_node_t *)hNode)->sem);
    return EPDK_OK;
}

