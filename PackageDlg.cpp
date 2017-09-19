// PackageDlg.cpp : ��@��
//

#include "stdafx.h"
#include "AdbController.h"
#include "PackageDlg.h"
#include "afxdialogex.h"


#define DEF_PKG_COL 5
#define DEF_PKG_ROW 4


#define UWM_CLOSE_TAKE_THREAD (WM_USER + 100) 
#define UWM_TAKE_NEXT_THREAD (WM_USER + 101) 


#define PRJ_PKG 			_T("package")

#define PRJ_START_X			_T("start_x")
#define PRJ_START_Y			_T("start_y")
#define PRJ_INV_W			_T("interval_w")
#define PRJ_INV_H			_T("interval_h")


typedef struct _PACKAGE_MAP_IDS {
	UINT nBtnD_TakeOnePkg;
	UINT nChkID_Pkg;
} DEVICE_CONTROL_IDS;


static DEVICE_CONTROL_IDS G_PACKAGE_ID_MAP[DEF_PKG_COL*DEF_PKG_ROW] = {
	{
		IDC_PACKAGE_BTN1,
		IDC_PACKAGE_CHECK1
	},
	{
		IDC_PACKAGE_BTN2,
		IDC_PACKAGE_CHECK2
	},
	{
		IDC_PACKAGE_BTN3,
		IDC_PACKAGE_CHECK4
	},
	{
		IDC_PACKAGE_BTN4,
		IDC_PACKAGE_CHECK5
	},
	{
		IDC_PACKAGE_BTN5,
		IDC_PACKAGE_CHECK5
	},
	{
		IDC_PACKAGE_BTN6,
		IDC_PACKAGE_CHECK6
	},
	{
		IDC_PACKAGE_BTN7,
		IDC_PACKAGE_CHECK7
	},
	{
		IDC_PACKAGE_BTN8,
		IDC_PACKAGE_CHECK8
	},
	{
		IDC_PACKAGE_BTN9,
		IDC_PACKAGE_CHECK9
	},
	{
		IDC_PACKAGE_BTN10,
		IDC_PACKAGE_CHECK10
	},
	{
		IDC_PACKAGE_BTN11,
		IDC_PACKAGE_CHECK11
	},
	{
		IDC_PACKAGE_BTN12,
		IDC_PACKAGE_CHECK12
	},
	{
		IDC_PACKAGE_BTN13,
		IDC_PACKAGE_CHECK14
	},
	{
		IDC_PACKAGE_BTN14,
		IDC_PACKAGE_CHECK15
	},
	{
		IDC_PACKAGE_BTN15,
		IDC_PACKAGE_CHECK15
	},
	{
		IDC_PACKAGE_BTN16,
		IDC_PACKAGE_CHECK16
	},
	{
		IDC_PACKAGE_BTN17,
		IDC_PACKAGE_CHECK17
	},
	{
		IDC_PACKAGE_BTN18,
		IDC_PACKAGE_CHECK18
	},
	{
		IDC_PACKAGE_BTN19,
		IDC_PACKAGE_CHECK19
	},
	{
		IDC_PACKAGE_BTN20,
		IDC_PACKAGE_CHECK20
	},
};










// CPackageDlg ��ܤ��
IMPLEMENT_DYNAMIC(CPackageDlg, CDialogEx)

CPackageDlg::CPackageDlg(CWnd* pParent /*=NULL*/, int nDeviceIdx, CString sSerial, UINT nNotifyCloseDlgEvent, CString sWorkFolder)
	: CDialogEx(IDD_PACKAGE_DIALOG, pParent)
{
	m_Serial = sSerial;
	m_nDeviceIdx = nDeviceIdx;
	m_sWorkingFolder = sWorkFolder;
	m_pParent = pParent;
	m_nNotifyfCloseEvent = nNotifyCloseDlgEvent;
}

CPackageDlg::~CPackageDlg()
{

	OutputDebugString(_T("~CPackageDlg()\n"));

}


BOOL CPackageDlg::Create(void)
{
	return CDialogEx::Create(IDD_PACKIGE_DLG, m_pParent);
}

BOOL CPackageDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	CString title;
	title.Format(_T("Package - %s - %s"), DEVICE_NAME[m_nDeviceIdx], m_Serial);
	SetWindowText(title);

	InitializeCriticalSection(&m_LockTackOnePackage);
	
	LoadProfile();

	m_sPackageBatch.Format(_T("\"%s\\%s\\package.bat\" %s "), m_sWorkingFolder, DEF_SCRIPE_PATH, m_Serial);

	return TRUE;
}

void CPackageDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	CString temp;
	temp.Format(_T("OnSysCommand(0x%x, 0x%x)\n"), nID, lParam);
	OutputDebugString(temp);
	
	CDialogEx::OnSysCommand( nID, lParam);
	if(nID == SC_CLOSE){
		SaveProfile();
		m_pParent->PostMessage(m_nNotifyfCloseEvent, m_nDeviceIdx);
	}

}

void CPackageDlg::OnDestroy()
{
	DeleteCriticalSection(&m_LockTackOnePackage);
	
	OutputDebugString(_T("OnDestroy()\n"));
	
}

void CPackageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPackageDlg, CDialogEx)
	ON_WM_DESTROY()
	ON_WM_SYSCOMMAND()
	ON_MESSAGE(UWM_CLOSE_TAKE_THREAD, OnCloseSystemExecThread)
	ON_MESSAGE(UWM_TAKE_NEXT_THREAD, OnTakeNextSystemExecThread)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_PACKAGE_BTN1, IDC_PACKAGE_BTN20, &CPackageDlg::OnCtrlRgn_Take_one_package_btn)
	ON_BN_CLICKED(IDC_PACKAGE_TAKE_BTN, &CPackageDlg::OnBnClickedPackageTakeBtn)
	ON_BN_CLICKED(IDC_PACKAGE_CLEAR_BTN, &CPackageDlg::OnBnClickedPackageClearBtn)
END_MESSAGE_MAP()


// CPackageDlg �T���B�z�`��
void CPackageDlg::OnBnClickedPackageTakeBtn()
{
	CButton* btnCheck;
	m_lstPkgPts.RemoveAll();

	for(int idx = 0; idx < (DEF_PKG_COL*DEF_PKG_ROW); idx++) {
		btnCheck = (CButton*)GetDlgItem(G_PACKAGE_ID_MAP[idx].nChkID_Pkg);
		if(btnCheck->GetCheck() == 1){
			CPoint ptPkg;
			GetPackagePoint(idx, ptPkg);
			m_lstPkgPts.AddHead(ptPkg);
		}
	}

	if (m_lstPkgPts.GetCount() > 0) {
		GetDlgItem(IDC_PACKAGE_TAKE_BTN)->EnableWindow(FALSE);		
		PostMessage(UWM_TAKE_NEXT_THREAD);
	}

}

void CPackageDlg::OnBnClickedPackageClearBtn()
{
	CButton* btnCheck;

	for(int idx = 0; idx < (DEF_PKG_COL*DEF_PKG_ROW); idx++) {
		btnCheck = (CButton*)GetDlgItem(G_PACKAGE_ID_MAP[idx].nChkID_Pkg);
		btnCheck->SetCheck(0);
	}
	
}


void CPackageDlg::OnCtrlRgn_Take_one_package_btn(UINT nID)
{
	CString temp;
	temp.Format(_T("OnCtrlRgn_Take_one_package_btn(%d)\n"), nID);
	OutputDebugString(temp);
	
	EnterCriticalSection (&m_LockTackOnePackage);
	
	UpdateParameter();

	int nPkgId = FindPackageId( nID, offsetof(struct _PACKAGE_MAP_IDS, nBtnD_TakeOnePkg));
	if(nPkgId == -1) {
		OutputDebugString(_T("Invalid Pkg id.\n"));
		return;
	}

	CPoint ptPkg;
	GetPackagePoint( nPkgId, ptPkg);

	
	CString cmd;
	//cmd.Format(_T("adb -s %s shell input tap %d %d"), m_Serial, ptPkg.x, ptPkg.y);	
	cmd.Format(_T("%s %d %d"), m_sPackageBatch, ptPkg.x, ptPkg.y);
	
	PSystemExecParam pParam = new SystemExecParam;
	
	pParam->wnd 		= this;
	pParam->wStatus 	= NULL;
	pParam->btnPlay 	= (CButton*)GetDlgItem(nID);
	pParam->cmdLine 	= cmd;
	pParam->nNoityfCloseThreadEvent = UWM_CLOSE_TAKE_THREAD;
	pParam->hThread 	=
						CreateThread(
							NULL,					// default security attributes
							0,						// use default stack size
							SystemExecuteThread,	// thread function name
							pParam, 				// argument to thread function
							0,						// use default creation flags
							NULL);					// returns the thread identifier
	
	LeaveCriticalSection (&m_LockTackOnePackage);
}

LRESULT CPackageDlg::OnCloseSystemExecThread(WPARAM wParam, LPARAM lParam)
{

	PSystemExecParam pParam = (PSystemExecParam)wParam;
	
	OutputDebugString(_T("OnCloseSystemExecThread()\n"));

	// wait for thread exit.
	Sleep(100);
	CloseHandle(pParam->hThread);
	delete pParam;
	
	return 0;
}


LRESULT CPackageDlg::OnTakeNextSystemExecThread(WPARAM wParam, LPARAM lParam)
{
	PSystemExecParam pOldParam = (PSystemExecParam)wParam;
	// wParam is NULL if trigger first 
	if(pOldParam != NULL) {
		// wait for thread exit.
		Sleep(100);
		CloseHandle(pOldParam->hThread);
		delete pOldParam;
	}

	OutputDebugString(_T("OnTakeNextSystemExecThread()\n"));


	if (m_lstPkgPts.GetCount() == 0) {
		GetDlgItem(IDC_PACKAGE_TAKE_BTN)->EnableWindow(TRUE);
		return 0;
	}
		
	CPoint pt = m_lstPkgPts.RemoveHead();

	CString cmd;
	cmd.Format(_T("%s %d %d"), m_sPackageBatch, pt.x, pt.y);

	
	PSystemExecParam pParam = new SystemExecParam;
	
	pParam->wnd 		= this;
	pParam->wStatus 	= NULL;
	pParam->btnPlay 	= NULL;
	pParam->cmdLine 	= cmd;
	pParam->nNoityfCloseThreadEvent = UWM_TAKE_NEXT_THREAD;
	pParam->hThread 	=
						CreateThread(
							NULL,					// default security attributes
							0,						// use default stack size
							SystemExecuteThread,	// thread function name
							pParam, 				// argument to thread function
							0,						// use default creation flags
							NULL);					// returns the thread identifier	
		
	return 0;
}


int CPackageDlg::FindPackageId(UINT nControlId, int member)
{

	for(int idx = 0; idx < (DEF_PKG_COL*DEF_PKG_ROW); idx++) {

		UINT nCtrlId = *(UINT *)((char *)&G_PACKAGE_ID_MAP[idx] + member);

		if(nCtrlId == nControlId) {
			return idx;
		}
	}

	return -1;

}

void CPackageDlg::UpdateParameter()
{

	CEdit* editor;
	CString text;
	
	
	editor = (CEdit*)GetDlgItem(IDC_PKG_START_X);
	editor->GetWindowText(text);
	m_ptStart.x = _wtoi((LPCTSTR)text);
	
	editor = (CEdit*)GetDlgItem(IDC_PKG_START_Y);
	editor->GetWindowText(text);
	m_ptStart.y = _wtoi((LPCTSTR)text);

	editor = (CEdit*)GetDlgItem(IDC_PKG_INV_W);
	editor->GetWindowText(text);
	m_szInterval.cx = _wtoi((LPCTSTR)text);

	editor = (CEdit*)GetDlgItem(IDC_PKG_INV_H);
	editor->GetWindowText(text);
	m_szInterval.cy = _wtoi((LPCTSTR)text);

}

void CPackageDlg::GetPackagePoint(int pkdID, CPoint& outXY)
{

	CPoint ptPkg;

	outXY.x = m_ptStart.x + ((pkdID % DEF_PKG_COL) * m_szInterval.cx);
	outXY.y = m_ptStart.y + ((pkdID / DEF_PKG_COL) * m_szInterval.cy);	
	
} 


void CPackageDlg::LoadProfile()
{
	CWinApp* pApp = AfxGetApp();
	CEdit* editor;
	CString text;

	editor = (CEdit*)GetDlgItem(IDC_PKG_START_X);
	editor->SetWindowText(pApp->GetProfileString(PRJ_PKG, PRJ_START_X));
	editor = (CEdit*)GetDlgItem(IDC_PKG_START_Y);
	editor->SetWindowText(pApp->GetProfileString(PRJ_PKG, PRJ_START_Y));
	editor = (CEdit*)GetDlgItem(IDC_PKG_INV_W);
	editor->SetWindowText(pApp->GetProfileString(PRJ_PKG, PRJ_INV_W));
	editor = (CEdit*)GetDlgItem(IDC_PKG_INV_H);
	editor->SetWindowText(pApp->GetProfileString(PRJ_PKG, PRJ_INV_H));





}


void CPackageDlg::SaveProfile()
{
	CWinApp* pApp = AfxGetApp();
	CEdit* editor;
	CString text;

	editor = (CEdit*)GetDlgItem(IDC_PKG_START_X);
	editor->GetWindowText(text);
	pApp->WriteProfileString( PRJ_PKG, PRJ_START_X, text);

	editor = (CEdit*)GetDlgItem(IDC_PKG_START_Y);
	editor->GetWindowText(text);
	pApp->WriteProfileString( PRJ_PKG, PRJ_START_Y, text);

	editor = (CEdit*)GetDlgItem(IDC_PKG_INV_W);
	editor->GetWindowText(text);
	pApp->WriteProfileString( PRJ_PKG, PRJ_INV_W, text);

	editor = (CEdit*)GetDlgItem(IDC_PKG_INV_H);
	editor->GetWindowText(text);
	pApp->WriteProfileString( PRJ_PKG, PRJ_INV_H, text);




}



