// PlugInWrapper.cpp: implementation of the CPlugInWrapper class.
//
//////////////////////////////////////////////////////////////////////


#include "PlugInWrapper.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#include "plug_shell.h"
#include "plug_shell.cpp"



#ifndef SCOTT_DEBUG
#define SCOTT_DEBUG
#include "error.h"
#endif //SCOTT_DEBUG


//------------------------------------------------------------------------------------------------------------
//
// 函数说明
//
//
// 参数说明
//
//
// 返回值
//
//
// 其他
//    无
//
//------------------------------------------------------------------------------------------------------------
CPlugInWrapper::CPlugInWrapper()
{
    memset(m_plug_path, 0, MAX_PATH);

    m_hPlug = NULL;             //
}


//------------------------------------------------------------------------------------------------------------
//
// 函数说明
//
//
// 参数说明
//
//
// 返回值
//
//
// 其他
//    无
//
//------------------------------------------------------------------------------------------------------------
CPlugInWrapper::CPlugInWrapper(const char *plug_path)
{
    SetPlugInPath(plug_path);

    m_hPlug = NULL;             //
}


//------------------------------------------------------------------------------------------------------------
//
// 函数说明
//
//
// 参数说明
//
//
// 返回值
//
//
// 其他
//    无
//
//------------------------------------------------------------------------------------------------------------
CPlugInWrapper::~CPlugInWrapper()
{
    if (NULL != m_hPlug)
    {
        UnLoadPlugIn();
        m_hPlug = NULL;
    }
}


//------------------------------------------------------------------------------------------------------------
//
// 函数说明
//
//
// 参数说明
//
//
// 返回值
//
//
// 其他
//    无
//
//------------------------------------------------------------------------------------------------------------
u32 CPlugInWrapper::SetPlugInPath(const char *plug_path)
{
    memset(m_plug_path, 0, MAX_PATH);
    strcpy(m_plug_path, plug_path);

    return OK;
}

//------------------------------------------------------------------------------------------------------------
//
// 函数说明
//
//
// 参数说明
//
//
// 返回值
//
//
// 其他
//    无
//
//------------------------------------------------------------------------------------------------------------
char *CPlugInWrapper::GetPlugInPath()
{
    return m_plug_path;
}

//------------------------------------------------------------------------------------------------------------
//
// 函数说明
//
//
// 参数说明
//
//
// 返回值
//
//
// 其他
//    无
//
//------------------------------------------------------------------------------------------------------------
u32 CPlugInWrapper::LoadPlugIn()
{
    u32 ret = OK;

    if (NULL != m_hPlug)
    {
        ret = UnLoadPlugIn();
        m_hPlug = NULL;
        if (OK != ret)
        {
            return __LINE__;
        }
    }

    if (0 == strlen(m_plug_path))
    {
        return __LINE__;
    }

    m_hPlug = ::_LoadPlugIn(m_plug_path);
    if (NULL == m_hPlug)
        return __LINE__;

    return ret;
}


//------------------------------------------------------------------------------------------------------------
//
// 函数说明
//
//
// 参数说明
//
//
// 返回值
//
//
// 其他
//    无
//
//------------------------------------------------------------------------------------------------------------
u32 CPlugInWrapper::UnLoadPlugIn()
{
    u32 ret = OK;

    if (NULL == m_hPlug)
        return __LINE__;

    ret = ::_UnLoadPlugIn(m_hPlug);
    m_hPlug = NULL;
    memset(m_plug_path, 0, MAX_PATH);
    if (OK != ret)
        return __LINE__;

    return OK;
}


//------------------------------------------------------------------------------------------------------------
//
// 函数说明
//
//
// 参数说明
//
//
// 返回值
//
//
// 其他
//    无
//
//------------------------------------------------------------------------------------------------------------
Common_t *CPlugInWrapper::GetCommonInterface()
{
    if (NULL == m_hPlug)
        return NULL;

    return ::_GetCommonInterface(m_hPlug);
}


//------------------------------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------------------------------
u32 CPlugInWrapper::QueryInterface(const char *plug_guid)
{
    if (NULL == plug_guid)
    {
        Err("CPlugInWrapper::QueryInterface", __FILE__, __LINE__, "!plug_guid %s", plug_guid);
        return __LINE__;
    }

    if (NULL == m_hPlug)
    {
        Err("CPlugInWrapper::QueryInterface", __FILE__, __LINE__, "!m_hPlug %s", plug_guid);
        return __LINE__;
    }

    return ::_QueryInterface(m_hPlug, plug_guid);
}


//------------------------------------------------------------------------------------------------------------
//
// 函数说明
//
//
// 参数说明
//
//
// 返回值
//
//
// 其他
//    无
//
//------------------------------------------------------------------------------------------------------------
void *CPlugInWrapper::GetInterface(const char *plug_guid)
{
    if (NULL == plug_guid)
        return NULL;

    if (NULL == m_hPlug)
        return NULL;

    return ::_GetInterface(m_hPlug, plug_guid);
}


//------------------------------------------------------------------------------------------------------------
//
// 函数说明
//
//
// 参数说明
//
//
// 返回值
//
//
// 其他
//    无
//
//------------------------------------------------------------------------------------------------------------
HPLUG CPlugInWrapper::GetPlugInHandle()
{
    return m_hPlug;
}

