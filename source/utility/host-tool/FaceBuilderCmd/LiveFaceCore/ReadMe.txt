--------------------------------------------------------------------------------
								��������
--------------------------------------------------------------------------------

Msg("�д�����%s %d", __FILE__, __LINE__);

��Դģ���е��ļ���Ҫ���и�ʽ��ʶ�� 
�� bmp�� jpg�� png��

pngͼƬת��ΪbmpͼƬ

�����ĵ�������



--------------------------------------------------------------------------------
								2009-06-12 
--------------------------------------------------------------------------------

2009-06-12 17:40:24 scott
����״̬�� ��Դ�ļ���size 
����ͨ��


2009-06-12 17:01:53 scott
������ �ַ�����Ϣ ���� Ӣ��

2009-06-12 11:15:26 scott
Ӧ�ó���ʼ�����򴴽�faceĿ¼
Ӧ�ó����˳�ʱɾ��face��

2009-06-12 9:29:44 scott
�������ɾ��faceĿ¼�µ��ļ� 
DeleteFaceTempFile ���ε���


2009-06-12 9:03:23 scott
����ģ�棺��NULL����Դ���ļ��ֲ�����ʱ ��ʾ�û������Ϣ

--------------------------------------------------------------------------------
								2009-06-10
--------------------------------------------------------------------------------


2009-06-10 10:42:34 scott
����up��down������ʾbar home end


2009-06-10 8:56:18 scott
void CLiveFaceDoc::OnFaceBuild() 
	//--------------------------------------------------------------------------
	// ת��ǰ�ȱ���
	//--------------------------------------------------------------------------
	if (this->IsModified())
	{
		::AfxMessageBox("Please Save First !");
		return;
	}

--------------------------------------------------------------------------------
								2009-06-09
--------------------------------------------------------------------------------


2009-06-09 19:23:14 scott
void CLiveFaceDoc::OnFaceBuild() 
// ת��ǰ�ȱ���

2009-06-09 18:25:01 scott
Ψһʵ������


2009-06-09 17:26:49 scott
resaccess��ȡ��Դ�ļ�����bug
�Ѵ�������дһ�飬��oK

--------------------------------------------------------------------------------
								2009-06-08
--------------------------------------------------------------------------------

2009-06-08 15:52:32 scott
�������ĵ�����������


2009-06-08 15:52:07 scott
��ƻ����ļ���Toolbarͼ�꣬����ͨ��


2009-06-08 15:51:27 scott
UI������Ը��ݲ���ϵͳ������ȷ������;�����ж�̬�л�


2009-06-08 14:04:09 scott
�������ʱ���ݲ���ϵͳ������������UI��������Դ
AfxSetResourceHandle


2009-06-08 11:43:37 scott
��Ӱ����ĵ� ���ܣ� ����ͨ��


2009-06-08 11:03:58 scott
void CLiveFaceDoc::OnFaceBuild() 
	this->BeginWaitCursor();
	ret = m_FaceDocument.BuildFace((char *)(LPCTSTR)csFaceBin, 1);
	this->EndWaitCursor();
�򿪵ȴ����

2009-06-08 10:46:53 scott
faceĿ¼�µ��ļ�����livefaceһ��ʼ���о���Ҫɾ��
liveface�˳�ǰɾ��faceĿ¼�µ��ļ�


2009-06-08 10:30:55 scott
ѡ����Դ�ļ�֮�����item����ʾ��Ϣ


2009-06-08 9:27:27 scott
���previewbar����ʾ���ؿ��ƵĲ˵���
����ͨ��


2009-06-08 9:02:41 scott
������˫�� -1�����ֱ�ӷ��أ���������
����һ����Դ��Ŀѡ��Res�ļ����¼�


2009-06-08 9:02:25 scott
listview Font���ã�����ͨ��

--------------------------------------------------------------------------------
								2009-06-07
--------------------------------------------------------------------------------

2009-06-07 11:28:03 scott
�û�ѡ�����ԴͼƬҲ������ʾ������

2009-06-07 11:08:51 scott
��ʾ��Դģ���е�ͼƬ���Ѿ��ɹ�


--------------------------------------------------------------------------------
								2009-06-05
--------------------------------------------------------------------------------

2009-06-05 17:32:55 scott
getActiveDoc��Ҫ��鷵�ص�ָ���Ƿ�ΪNULL


2009-06-05 17:32:37 scott
�������preViewbar


2009-06-05 10:34:00 scott
void CLiveFaceDoc::OnUpdateFileSave(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(IsModified());	
}


2009-06-05 10:29:37 scott
void CLiveFaceApp::OnUpdateFileOpen(CCmdUI* pCmdUI) 
����ֻ��һ���ĵ�


2009-06-05 10:12:44 scott
void CLiveFaceApp::OnFileOpen() 
����ֻ��һ���ĵ�


2009-06-05 9:48:13 scott
script_if �ӿ�  �޸���
SetSysInfo SetResInfo ������д��ű��洢
�û��޸ĵĽ����Ч��

--------------------------------------------------------------------------------
								2009-06-04
--------------------------------------------------------------------------------

2009-06-04 17:29:28 scott
//��item����˫��
void CLiveFaceView::OnDblclk(NMHDR* pNMHDR, LRESULT* pResult) 


2009-06-04 17:29:23 scott
// ��item���е���
void CLiveFaceView::OnClick(NMHDR* pNMHDR, LRESULT* pResult) 


2009-06-04 17:06:06 scott
LRESULT  CLiveFaceView::OnLoadResInfo(WPARAM wParam, LPARAM lParam)
��Ϣ��������ʵ�ʴ�doc�л�ȡ���ݣ�Ȼ����ʾ����


2009-06-04 15:57:04 scott
void CLiveFaceView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	Msg("OnUpdate:%s %d", __FILE__, __LINE__);

	::PostMessage(this->GetSafeHwnd(), WM_LOAD_RESINFO, 0, 0);
}




2009-06-04 14:19:50 scott
void CLiveFaceView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 

2009-06-04 13:43:18 scott
//scott
	CenterWindow(GetDesktopWindow());
	

2009-06-04 10:11:30 scott
BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
//��ֹ�ڴ��ڱ������ϰ��ĵ���Ԥ�ó�Ӧ�ó�����
	cs.style &= ~FWS_PREFIXTITLE;
	

2009-06-04 9:55:10 scott
int CLiveFaceView::OnCreate(LPCREATESTRUCT lpCreateStruct) 

2009-06-04 9:40:20 scott
BOOL CLiveFaceDoc::OnOpenDocument(LPCTSTR lpszPathName) 


2009-06-04 9:39:26 scott
��ֹMDI��������ʱ�Զ������հ״��� 
cmdInfo.m_nShellCommand = CCommandLineInfo::FileNothing;	//scott
����ͨ��

2009-06-04 9:35:40 scott
��������MFC MDI listview��LiveFace������


2009-06-04 8:41:05 scott
����LiveFaceCoreĿ¼�������Ĺ��ܵ����Լ��ļ�����������


--------------------------------------------------------------------------------
								2009-06-03
--------------------------------------------------------------------------------


2009-06-03 15:52:18 scott
void FaceDocument::Initial()


2009-06-03 15:51:51 scott
void CLiveFaceDoc::OnFaceBuild() ����ͨ��


2009-06-03 15:05:06 scott
scriptparser getresinfo ��0��ʼ�źã������Ǵ�1��ʼ��
sunny�Ѿ����


2009-06-03 11:18:33 scott
FaceDocumentData Dump

--------------------------------------------------------------------------------
								2009-06-02
--------------------------------------------------------------------------------


2009-06-02 18:44:19 scott
u32 FaceDocument::StoreFaceDocumet(char *face_document)
������


2009-06-02 17:05:00 scott
BOOL CLiveFaceDoc::OnSaveDocument(LPCTSTR lpszPathName) 


2009-06-02 15:13:26 scott
commnet����Ϣ������UI����ʾ������


2009-06-02 15:07:14 scott
BOOL CLiveFaceDoc::OnOpenDocument(LPCTSTR lpszPathName) 


2009-06-02 15:07:08 scott
BOOL CLiveFaceDoc::OnNewDocument()


2009-06-02 15:03:09 scott
void CCellTypesGridCtrl::AddItem(char *index, char *comment, char *filepath)


2009-06-02 15:02:50 scott
void CLiveFaceView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 



2009-06-02 14:17:17 scott
FaceResource ��������ʵ�֣�����ͨ��


2009-06-02 14:16:49 scott
FaceDocument::LoadFaceDocumet
���res��Դ����ȡ�����ļ��ļ�⹦�ܣ�����ͨ��


2009-06-02 11:01:49 scott
class FaceResource   ������res��Դ����ȡ


2009-06-02 10:41:48 scott
FaceDocument::LoadFaceDocumet ����ͨ��


2009-06-02 10:10:01 scott
�޸�LoadFaceDocumet�е�bug


2009-06-02 9:45:40 scott
u32 FaceDocument::LoadFaceDocumet(char *face_document, BOOL bTemplate)
��д��ɣ���Ҫ���в���


2009-06-02 8:58:13 scott
class FaceDocumentBuilder  ��ɣ�����ͨ��
facedocbuilder ����psp.face�ļ��Լ��û��ķ�񷽰�

face.document �ű������ɷ���ļ���ʽ
��ʵ����img�ļ���ʽ������ֻ�ǲ���һ����ܼ��ɣ�����̫�����ƽ������


--------------------------------------------------------------------------------
								2009-05-27
--------------------------------------------------------------------------------

2009-05-27 17:52:06 scott
���������Ĳ���Ѿ���ɣ���������ͨ��������һ����Ҫ�����ĵط�

2009-05-27 11:03:15 scott
class FaceBuilder  ����face��Դ�Ĵ���

--------------------------------------------------------------------------------
								2009-05-26
--------------------------------------------------------------------------------

2009-05-26 16:42:08 scott
class FaceScriptData  ������ɣ��д�����


2009-05-26 16:41:44 scott
class FaceScript  ������ɣ��д�����


2009-05-26 16:41:11 scott
class FaceManager  �����࣬������������


2009-05-26 15:46:00 scott
facescript ���������Դ�ű�

2009-05-26 15:45:30 scott
����BCG���SDI����

2009-05-26 15:45:13 scott
���༭��

--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
