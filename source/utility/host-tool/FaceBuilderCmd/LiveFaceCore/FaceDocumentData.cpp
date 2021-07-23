// FaceDocumentData.cpp: implementation of the FaceDocumentData class.
//
//////////////////////////////////////////////////////////////////////

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "error.h"
#include "basetypes.h"
#include "FaceDocumentData.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


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
FaceDocumentData::FaceDocumentData()
{
    memset(&m_system, 0, sizeof(SYSTEM_HEAD)) ;

    m_pItemArray = NULL;
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
FaceDocumentData::~FaceDocumentData()
{
    Release();
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
void FaceDocumentData::Release()
{
    if (NULL != m_pItemArray)
    {
        free(m_pItemArray);
        m_pItemArray = NULL;
    }

    memset(&m_system, 0, sizeof(SYSTEM_HEAD)) ;
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
u32 FaceDocumentData::AllocItemArray()
{
    if (NULL != m_pItemArray)
    {
        free(m_pItemArray);
        m_pItemArray = NULL;
    }

    m_pItemArray = (ITEM_INFO *)malloc(m_system.filecnt * sizeof(ITEM_INFO));
    if (NULL == m_pItemArray)
    {
        return __LINE__;
    }
    memset(m_pItemArray, 0, m_system.filecnt * sizeof(ITEM_INFO));

    return OK;
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

SYSTEM_HEAD *FaceDocumentData::GetSystemData()
{
    return &m_system;
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

ITEM_INFO *FaceDocumentData::GetItemData(u32 index)
{
    if (NULL == m_pItemArray)
    {
        return NULL;
    }

    if (index >= m_system.filecnt)
    {
        return NULL;
    }

    return &m_pItemArray[index];
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

void FaceDocumentData::Dump()
{
    u32 i = 0;

    DumpSystemData();

    for (i = 0 ; i < m_system.filecnt; i++)
    {
        DumpItem(i);
    }

    return ;
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
void FaceDocumentData::DumpSystemData()
{
    SYSTEM_HEAD *pSystemData = &m_system;

    Msg("version    = %x\n", pSystemData->version);
    Msg("pid	= %x\n", pSystemData->pid);
    Msg("vid	= %x\n", pSystemData->vid);
    Msg("hid	= %x\n", pSystemData->hid);
    Msg("fid	= %x\n", pSystemData->fid);
    Msg("image	= %s\n", pSystemData->image);
    Msg("filecnt    = %x\n", pSystemData->filecnt);
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

void FaceDocumentData::DumpItem(u32 index)
{
    ITEM_INFO *pItemInfo = NULL;

    if (NULL == m_pItemArray)
    {
        return ;
    }

    if (index >= m_system.filecnt)
    {
        return ;
    }

    pItemInfo = &m_pItemArray[index];

    Msg("main	= %s\n", pItemInfo->main);
    Msg("sub	= %s\n",  pItemInfo->sub);
    Msg("file	= %s\n", (char *)pItemInfo->filepath);

    return;
}
