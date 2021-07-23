// PlugInCenter.cpp: implementation of the CPlugInCenter class.
//
//////////////////////////////////////////////////////////////////////
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "PlugInCenter.h"

/*
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
*/
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


#ifndef SCOTT_DEBUG
#define SCOTT_DEBUG
#endif //SCOTT_DEBUG

#include "error.h"                  //

#include "PlugInWrapper/PlugInWrapper.h"        //�����װ��

#include "plugin.h"
#include "plugin.cpp"


//------------------------------------------------------------------------------------------------------------
//
// ����˵��
//
//
// ����˵��
//
//
// ����ֵ
//
//
// ����
//    ��
//
//------------------------------------------------------------------------------------------------------------

CPlugInCenter::CPlugInCenter()
{
    m_PlugList = NULL;
}

//------------------------------------------------------------------------------------------------------------
//
// ����˵��
//
//
// ����˵��
//
//
// ����ֵ
//
//
// ����
//    ��
//
//------------------------------------------------------------------------------------------------------------
CPlugInCenter::~CPlugInCenter()
{
    DeleteList();
}


//------------------------------------------------------------------------------------------------------------
//
// ����˵��
//
//
// ����˵��
//
//
// ����ֵ
//
//
// ����
//    ��
//
//------------------------------------------------------------------------------------------------------------
void *CPlugInCenter::LoadInterface(const char *plug_guid)
{
    u32 ret     = OK;
    int i       = 0;
    int index   = -1;
    char szModuleDir[MAX_PATH];
    char szPlugPath[MAX_PATH];
    CPlugInWrapper *pWrapper = NULL;
    void            *plug_if  = NULL;
    PLUG_IF_ITEM_t *item     = NULL;

    //----------------------------------------------------------
    //���ԶԲ������һ����׺.plg ����Ŀ¼�µ�plg�ļ����أ�
    //����gettype��ýӿ�id��Ȼ�����queryinterface��ѯ�ӿ�
    //������getinterface��ýӿ�.�����ı��Ϳ����˻�����
    //----------------------------------------------------------

    //----------------------------------------------------------
    //
    //----------------------------------------------------------
    for (i = 0; i < sizeof(plug_info) / sizeof(PLUG_INFO_t); i++)
    {
        if (memcmp(plug_guid, plug_info[i].plug_guid, GUID_LEN) == 0)
        {
            index = i;
            break;
        }
    }

    if (index == -1)
    {
        Err("::LoadInterface", __FILE__, __LINE__, "Not support plug %s", plug_guid);
        return NULL;
    }

    item = new PLUG_IF_ITEM_t;
    if (NULL == item)
    {
        Err("::LoadInterface", __FILE__, __LINE__, "!item");
        return NULL;
    }

    pWrapper = new CPlugInWrapper;
    if (NULL == pWrapper)
    {
        Err("::LoadInterface", __FILE__, __LINE__, "!pWrapper");
        delete item;
        item = NULL;

        return NULL;
    }
    //Msg("malloc : %08x %08x", item, pWrapper);

//  ret = GetModuleDirectory(szModuleDir);
    getcwd(szModuleDir, MAX_PATH);
    memset(szPlugPath, 0, MAX_PATH);
    sprintf(szPlugPath, "%s/%s", szModuleDir, plug_info[index].plug_name);
    pWrapper->SetPlugInPath(szPlugPath);

    ret = pWrapper->LoadPlugIn();
    if (OK != ret)
    {
        Err("LoadInterface", __FILE__, __LINE__, "pWrapper->LoadPlugIn %d", ret);
        delete pWrapper;
        pWrapper = NULL;
        delete item;
        item = NULL;

        return NULL;
    }
    ret = pWrapper->QueryInterface((const char *)plug_guid);
    if (OK != ret)
    {
        Err("LoadInterface", __FILE__, __LINE__, "QueryInterface(%s) %d", plug_guid, ret);
        delete pWrapper;
        pWrapper = NULL;
        delete item;
        item = NULL;

        return NULL;
    }
    plug_if = pWrapper->GetInterface((const char *)plug_guid);
    if (NULL == plug_if)
    {
        Err("LoadInterface", __FILE__, __LINE__, "GetInterface");
        delete pWrapper;
        pWrapper = NULL;

        delete item;
        item = NULL;

        return NULL;
    }

    memcpy(item->GUID, plug_guid, GUID_LEN);
    item->Wrapper = pWrapper;
    item->_if = plug_if;
    item->next = NULL;

    AddPlugItem(item);

    return item->_if;
}

//------------------------------------------------------------------------------------------------------------
//
// ����˵��
//
//
// ����˵��
//
//
// ����ֵ
//
//
// ����
//    ��
//
//------------------------------------------------------------------------------------------------------------
u32 CPlugInCenter::AddPlugItem(PLUG_IF_ITEM_t *item)
{
    PLUG_IF_ITEM_t *current = NULL;
    if (NULL == m_PlugList)
    {
        m_PlugList = item;
    }
    else
    {
        current = m_PlugList;
        while (NULL != current)
        {
            if (NULL != current->next)
            {
                current = current->next;
            }
            else
            {
                current->next = item;
                return OK;
            }
        }
    }

    return __LINE__;
}
//------------------------------------------------------------------------------------------------------------
//
// ����˵��
//
//
// ����˵��
//
//
// ����ֵ
//
//
// ����
//    ��
//
//------------------------------------------------------------------------------------------------------------
void CPlugInCenter::DeleteList()
{
    PLUG_IF_ITEM_t *current = NULL;
    PLUG_IF_ITEM_t *item = NULL;


    if (NULL == m_PlugList)
    {
        Msg("DeleteList:%d\n", __LINE__);
        return;
    }
    else
    {
        current = m_PlugList;
        while (current)
        {
            item = current;
            current = current->next;

            item->Wrapper->UnLoadPlugIn();
            delete item->Wrapper;
            item->Wrapper = NULL;

            item->_if = NULL;

            delete item;
            item = NULL;
        }
        m_PlugList = NULL;
    }
}

//------------------------------------------------------------------------------------------------------------
//
// ����˵��
//
//
// ����˵��
//
//
// ����ֵ
//
//
// ����
//    ��
//
//------------------------------------------------------------------------------------------------------------
void *CPlugInCenter::GetInterfaceFromList(const char *plug_guid)
{
    PLUG_IF_ITEM_t *current = NULL;
    if (NULL == m_PlugList)
    {
        return NULL;
    }
    else
    {
        current = m_PlugList;
        while (NULL != current)
        {
            if (0 == memcmp(plug_guid, current->GUID, GUID_LEN))
            {
                return current->_if;
            }
            current = current->next;
        }
    }

    return NULL;
}

//------------------------------------------------------------------------------------------------------------
//
// ����˵��
//
//
// ����˵��
//
//
// ����ֵ
//
//
// ����
//    ��
//
//------------------------------------------------------------------------------------------------------------
void *CPlugInCenter::GetInterface(const char *plug_guid)
{
    void *plug_if = NULL;

    plug_if = GetInterfaceFromList(plug_guid);
    if (NULL != plug_if)
    {
        return plug_if;
    }
    else
    {
        plug_if = LoadInterface(plug_guid);
        return plug_if;
    }
}



//------------------------------------------------------------------------------------------------------------
//
// ����˵��
//
//
// ����˵��
//
//
// ����ֵ
//
//
// ����
//    ��
//
//------------------------------------------------------------------------------------------------------------
void CPlugInCenter::Release()
{
    DeleteList();
}
