// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "LiveFace.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define FACE_GUIDE  "LiveFaceGuide.pdf"

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
    //{{AFX_MSG_MAP(CMainFrame)
    ON_WM_CREATE()
    ON_COMMAND(ID_VIEW_PREVIEW_BAR_TEMP, OnViewPreviewBarTemp)
    ON_UPDATE_COMMAND_UI(ID_VIEW_PREVIEW_BAR_TEMP, OnUpdateViewPreviewBarTemp)
    ON_COMMAND(ID_VIEW_PREVIEW_BAR_USER, OnViewPreviewBarUser)
    ON_UPDATE_COMMAND_UI(ID_VIEW_PREVIEW_BAR_USER, OnUpdateViewPreviewBarUser)
    ON_COMMAND(ID_UI_CHINESE, OnUiChinese)
    ON_UPDATE_COMMAND_UI(ID_UI_CHINESE, OnUpdateUiChinese)
    ON_COMMAND(ID_UI_ENGLISH, OnUiEnglish)
    ON_UPDATE_COMMAND_UI(ID_UI_ENGLISH, OnUpdateUiEnglish)
    ON_COMMAND(ID_FACE_GUIDE, OnFaceGuide)
    ON_WM_DROPFILES()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/*
static UINT indicators[] =
{
    ID_SEPARATOR,           // status line indicator
    ID_INDICATOR_CAPS,
    ID_INDICATOR_NUM,
    ID_INDICATOR_SCRL,
};
*/

static UINT indicators[] =
{
//  ID_SEPARATOR,           // status line indicator
    ID_INDICATOR_TEMPLATE_SIZE,
    ID_INDICATOR_USERRES_SIZE,
    ID_INDICATOR_CAPS,
//  ID_INDICATOR_NUM,
//  ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

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

CMainFrame::CMainFrame() : m_PreViewTemplate(TEMPLATE_RES), m_PreViewUser(USER_RES)
{
    m_bShowTemplate = TRUE;
    m_bShowUser     = TRUE;

    m_bChina = (CHINA_ACP == GetACP()) ? TRUE : FALSE;

    m_hinstanceChinese = NULL;
    m_hinstanceEnglish = NULL;
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
CMainFrame::~CMainFrame()
{

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
int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    //-------------------------------------------------------------------------
    //
    //-------------------------------------------------------------------------
    if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
                               | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
            !m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
    {
        TRACE0("Failed to create toolbar\n");
        return -1;      // fail to create
    }

    //-------------------------------------------------------------------------
    //���ݲ���ϵͳ�����������ù������ַ�,��������ť���˵������
    //-------------------------------------------------------------------------
    int index = 0;
    m_wndToolBar.SetButtonText(index++, m_bChina ? "��"   : "Open");
    m_wndToolBar.SetButtonText(index++, m_bChina ? "����"   : "Save");
    m_wndToolBar.SetButtonText(index++, m_bChina ? "���Ϊ" : "Saveas");

    m_wndToolBar.SetButtonText(index++, m_bChina ? "ģ��"   : "Template");
    m_wndToolBar.SetButtonText(index++, m_bChina ? "�û�"   : "User");
    m_wndToolBar.SetButtonText(index++, m_bChina ? "����"   : "Compile");
    m_wndToolBar.SetButtonText(index++, m_bChina ? "����"   : "Help");
    m_wndToolBar.SetButtonText(index++, m_bChina ? "����"   : "About");

    //������������С
    CRect rc(0, 0, 0, 0);
    CSize sizeMax(0, 0);
    CToolBarCtrl &bar = m_wndToolBar.GetToolBarCtrl();
    for (int nIndex = bar.GetButtonCount() - 1; nIndex >= 0; nIndex--)
    {
        bar.GetItemRect(nIndex, rc);
        rc.NormalizeRect();
        sizeMax.cx = __max(rc.Size().cx, sizeMax.cx);
        sizeMax.cy = __max(rc.Size().cy, sizeMax.cy);
    }
    m_wndToolBar.SetSizes(sizeMax, CSize(16, 15));

    //-------------------------------------------------------------------------
    //
    //-------------------------------------------------------------------------
    if (!m_wndStatusBar.Create(this) ||
            !m_wndStatusBar.SetIndicators(indicators,
                                          sizeof(indicators) / sizeof(UINT)))
    {
        TRACE0("Failed to create status bar\n");
        return -1;      // fail to create
    }

    int StatusPaneWidth[] =
    {
        //50,
        200,
        200,
        50,
        50,
        50,
    };
    for (int i = 0 ; i < 3 /*sizeof(indicators)/sizeof(UINT)*/; i++)
    {
        m_wndStatusBar.SetPaneInfo(i, indicators[i], SBPS_POPOUT, StatusPaneWidth[i]);
    }
    //-------------------------------------------------------------------------
    //
    //-------------------------------------------------------------------------
    m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
    EnableDocking(CBRS_ALIGN_ANY);
    DockControlBar(&m_wndToolBar);


    //-------------------------------------------------------------------------
    //
    //-------------------------------------------------------------------------
    if (!m_PreViewTemplate.Create(_T(TITLE_STANDARD), this, CSize(256, 500),
                                  TRUE , AFX_IDW_CONTROLBAR_FIRST + 33))
    {
        Msg("%s %d", __FILE__, __LINE__);
        return -1;
        // fail to create
    }

    m_PreViewTemplate.SetBarStyle(m_PreViewTemplate.GetBarStyle() | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);


    m_PreViewTemplate.EnableDocking(CBRS_ALIGN_LEFT | CBRS_ALIGN_RIGHT);


    EnableDocking(CBRS_ALIGN_ANY);

#ifdef _SCB_REPLACE_MINIFRAME
    m_pFloatingFrameClass = RUNTIME_CLASS(CSCBMiniDockFrameWnd);
#endif


    //-------------------------------------------------------------------------
    //
    //-------------------------------------------------------------------------
    if (!m_PreViewUser.Create(_T(TITLE_USERRES), this, CSize(256, 500),
                              TRUE , AFX_IDW_CONTROLBAR_FIRST + 33))
    {
        return -1;
        // fail to create
    }
    m_PreViewUser.SetBarStyle(m_PreViewUser.GetBarStyle() | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

    m_PreViewUser.EnableDocking(CBRS_ALIGN_LEFT | CBRS_ALIGN_RIGHT);

    DockControlBar(&m_PreViewTemplate, AFX_IDW_DOCKBAR_LEFT);

    //-------------------------------------------------------------------------
    //
    //-------------------------------------------------------------------------
    RecalcLayout();
    CRect rBar;
    m_PreViewTemplate.GetWindowRect(rBar);
    rBar.OffsetRect(0, 1);

    DockControlBar(&m_PreViewUser, AFX_IDW_DOCKBAR_LEFT, rBar);


    m_bShowTemplate = TRUE;
    m_bShowUser     = TRUE;

    //-------------------------------------------------------------------------
    //scott
    //-------------------------------------------------------------------------
    CenterWindow(GetDesktopWindow());

    //Msg("%s %d", __FILE__, __LINE__);

    m_hinstanceChinese = AfxGetResourceHandle();//::AfxGetInstanceHandle();
    m_hinstanceEnglish = ::LoadLibrary("English.dll");

    return 0;
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

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT &cs)
{
    if (!CMDIFrameWnd::PreCreateWindow(cs))
        return FALSE;

    // TODO: Modify the Window class or styles here by modifying
    //  the CREATESTRUCT cs

    //cs.style |=WS_POPUP;            //ʹ�����ڲ��ɼ�
    //cs.dwExStyle |=WS_EX_TOOLWINDOW;//����ʾ����ť

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
    CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext &dc) const
{
    CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers


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

void CMainFrame::UpdatePreViewBar()
{
    m_PreViewTemplate.SendMessage(WM_SIZE);

    m_PreViewUser.SendMessage(WM_SIZE);
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
void CMainFrame::OnViewPreviewBarTemp()
{

    if (m_bShowTemplate)
    {
        ::ShowWindow(m_PreViewTemplate.m_hWnd, SW_HIDE);
        DockControlBar(&m_PreViewTemplate, AFX_IDW_DOCKBAR_LEFT);
        m_bShowTemplate = FALSE;

        RecalcLayout(TRUE);
    }
    else
    {
        ::ShowWindow(m_PreViewTemplate.m_hWnd, SW_SHOW);
        DockControlBar(&m_PreViewTemplate, AFX_IDW_DOCKBAR_LEFT);
        RecalcLayout(TRUE);
        m_bShowTemplate = TRUE;
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
void CMainFrame::OnUpdateViewPreviewBarTemp(CCmdUI *pCmdUI)
{
    pCmdUI->SetCheck(m_bShowTemplate);

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
void CMainFrame::OnViewPreviewBarUser()
{
    if (m_bShowUser)
    {
        ::ShowWindow(m_PreViewUser.m_hWnd, SW_HIDE);
        DockControlBar(&m_PreViewUser, AFX_IDW_DOCKBAR_LEFT);
        m_bShowUser = FALSE;
        RecalcLayout(TRUE);
    }
    else
    {
        ::ShowWindow(m_PreViewUser.m_hWnd, SW_SHOW);
        DockControlBar(&m_PreViewUser, AFX_IDW_DOCKBAR_LEFT);
        RecalcLayout(TRUE);
        m_bShowUser = TRUE;
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
void CMainFrame::OnUpdateViewPreviewBarUser(CCmdUI *pCmdUI)
{
    pCmdUI->SetCheck(m_bShowUser);
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

void CMainFrame::OnUiChinese()
{
    m_bChina = TRUE;

    HINSTANCE hinst = m_hinstanceChinese;
    if (hinst)
    {
        ::AfxSetResourceHandle(hinst);
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
void CMainFrame::OnUpdateUiChinese(CCmdUI *pCmdUI)
{
    pCmdUI->SetCheck(m_bChina);

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
void CMainFrame::OnUiEnglish()
{
    m_bChina = FALSE;
    Msg("OnUiEnglish");

    if (m_hinstanceEnglish)
    {
        Msg("set hinst");
        ::AfxSetResourceHandle(m_hinstanceEnglish);
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
void CMainFrame::OnUpdateUiEnglish(CCmdUI *pCmdUI)
{
    pCmdUI->SetCheck(!m_bChina);

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

void CMainFrame::OnFaceGuide()
{
    char szHelpFile[MAX_PATH];
    memset(szHelpFile, 0, MAX_PATH);

    char szDir[MAX_PATH];
    GetModuleDirectory(szDir);

    //----------------------------------------------------------
    //�򿪰����ĵ�
    //----------------------------------------------------------
    sprintf(szHelpFile, "%s\\%s",  szDir, FACE_GUIDE);
    ::ShellExecute(NULL, "open", (LPCTSTR)szHelpFile, "", NULL, SW_SHOWDEFAULT);

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

BOOL CMainFrame::DestroyWindow()
{
    ::FreeLibrary(m_hinstanceEnglish);

    m_hinstanceEnglish = NULL;

    return CMDIFrameWnd::DestroyWindow();
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
void CMainFrame::SetPaneText(int index, const char *szText)
{
    m_wndStatusBar.SetPaneText(index, szText);
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

void CMainFrame::OnDropFiles(HDROP hDropInfo)
{
    char           szFilePath[MAX_PATH];
    UINT           cFiles = 0;
    UINT           u    = 0;
    CLiveFaceApp *pApp = NULL;
    CDocument     *pDoc = NULL;
    CString csFaceTemplate;

    cFiles = DragQueryFile(hDropInfo, (UINT) - 1, NULL, 0);
    if (1 != cFiles)
    {
        ::AfxMessageBox("һ��ֻ�ܴ���һ���ļ�");
        goto OnDropFiles_end;
    }

    memset(szFilePath, 0, sizeof(szFilePath));
    DragQueryFile(hDropInfo, u, szFilePath, sizeof(szFilePath));
    csFaceTemplate.Format("%s", szFilePath);
    //Msg("OnDropFiles:%s", csFaceTemplate);
    pApp = (CLiveFaceApp *)::AfxGetApp();
    if (NULL == pApp)
    {
        Msg("AfxGetApp failed !");
        goto OnDropFiles_end;
    }

    pDoc = pApp->OpenDocumentFile((LPCTSTR)csFaceTemplate);
    if (NULL == pDoc)
    {
        Msg("���ĵ�(%s)ʧ��", csFaceTemplate);
        goto OnDropFiles_end;
    }

    /*
    for (u = 0; u < cFiles; u++)
    {
       //  get the next file name and try to open it--if not a valid
       //  file, then skip to the next one (if there is one).
       memset(szFilePath, 0, sizeof(szFilePath));
       DragQueryFile(hDropInfo, u, szFilePath, sizeof(szFilePath));
       m_csFilePath.Empty();
       m_csFilePath.Format("[%d]=%s", u, szFilePath);
       this->UpdateData(FALSE);
       //Sleep(2000);
    }
    */

    CMDIFrameWnd::OnDropFiles(hDropInfo);
    return;

OnDropFiles_end:
    Msg("OnDropFiles_end");
    DragFinish(hDropInfo);

    CMDIFrameWnd::OnDropFiles(hDropInfo);
}
