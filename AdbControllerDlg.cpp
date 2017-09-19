
#include "stdafx.h"
#include "AdbController.h"
#include "AdbControllerDlg.h"
#include "afxdialogex.h"

#include <iostream>
#include <string>
#include <regex>
#include <stddef.h>
#include <stdarg.h>

#include "PackageDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif



#define UWM_SYNC_TOUPCH (WM_USER + 1)
#define UWM_CLOSE_SYS_EXEC_THREAD (WM_USER + 2)
#define UWM_CLOSE_PACKAGE_DLG (WM_USER + 3)
#define TOUCH_EVENT_LENGTH 37
#define TOUCH_DATA_LENGTH 18



typedef struct _DEVICE_CONTROL_IDS {
	UINT nEditID_Device;
	UINT nEditID_TouchEvt;
	UINT nChkID_Sync;
	UINT nBtnD_FindDevice;
	UINT nBtnD_DetectTouchEvt;
	UINT nBtnD_MoveSync;
	UINT nBtnD_Play_Script;
	UINT nEditD_Script_File;
	UINT nBtnD_OpenPkgDlg;
} DEVICE_CONTROL_IDS;


static DEVICE_CONTROL_IDS G_DEVICE_CONTROL_MAP[TOTAL_DEVICE] = {
	// Master
	{
		IDC_M_DEVICE_ID,
		IDC_M_TOUCH_EVT,
		IDC_M_SYNC_CHK,
		IDC_M_DEVICE_BTN,
		IDC_M_TOUCH_EVT_BTN,
		IDC_SYNC_MOVE_MASTER,
		IDC_M_PLAY_SCRIPT_BTN,
		IDC_M_SCRIPT_FILE,		
		IDC_PACKAGE_M
	},
	// Slave 1
	{
		IDC_S1_DEVICE_ID,
		IDC_S1_TOUCH_EVT,
		IDC_S1_SYNC_CHK,
		IDC_S1_DEVICE_BTN,
		IDC_S1_TOUCH_EVT_BTN,
		IDC_SYNC_MOVE_SLAVE_1,
		IDC_S1_PLAY_SCRIPT_BTN,
		IDC_S1_SCRIPT_FILE,		
		IDC_PACKAGE_S1
	},
	// Slave 2
	{
		IDC_S2_DEVICE_ID,
		IDC_S2_TOUCH_EVT,
		IDC_S2_SYNC_CHK,
		IDC_S2_DEVICE_BTN,
		IDC_S2_TOUCH_EVT_BTN,
		IDC_SYNC_MOVE_SLAVE_2,
		IDC_S1_PLAY_SCRIPT_BTN,
		IDC_S1_SCRIPT_FILE,		
		IDC_PACKAGE_S2
	},
	// Slave 3
	{
		IDC_S3_DEVICE_ID,
		IDC_S3_TOUCH_EVT,
		IDC_S3_SYNC_CHK,
		IDC_S3_DEVICE_BTN,
		IDC_S3_TOUCH_EVT_BTN,
		IDC_SYNC_MOVE_SLAVE_3,
		IDC_S2_PLAY_SCRIPT_BTN,
		IDC_S2_SCRIPT_FILE,		
		IDC_PACKAGE_S3
	},
	// Slave 4
	{
		IDC_S4_DEVICE_ID,
		IDC_S4_TOUCH_EVT,
		IDC_S4_SYNC_CHK,
		IDC_S4_DEVICE_BTN,
		IDC_S4_TOUCH_EVT_BTN,
		IDC_SYNC_MOVE_SLAVE_4,
		IDC_S2_PLAY_SCRIPT_BTN,
		IDC_S2_SCRIPT_FILE,		
		IDC_PACKAGE_S4
	}
};








class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()




CAdbControllerDlg::CAdbControllerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_ADBCONTROLLER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_bNeedUpdateSyncDeviceList = FALSE;
	m_bNeedUpdateMoveSync = FALSE;
}

CAdbControllerDlg::~CAdbControllerDlg()
{
	DeleteCriticalSection(&m_SyncReceivedLock);
	DeleteCriticalSection(&m_SyncTouchLock);

	if(m_pLastCommand != NULL) {
		delete m_pLastCommand;
		m_pLastCommand = NULL;
	}
}


void CAdbControllerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAdbControllerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_GETDLGCODE()
	ON_WM_KEYDOWN()
	ON_WM_DESTROY()

	ON_MESSAGE(UWM_PIPEBROKEN, OnEndChildProcess)
	ON_MESSAGE(UWM_SYNC_TOUPCH, OnSyncTouch)
	ON_MESSAGE(UWM_CLOSE_SYS_EXEC_THREAD, OnCloseSystemExecThread)
	ON_MESSAGE(UWM_CLOSE_PACKAGE_DLG, OnClosePackageDialog)

	

	ON_BN_CLICKED(IDOK, &CAdbControllerDlg::OnRun)
	ON_BN_CLICKED(IDC_BREAK, &CAdbControllerDlg::OnBreak)
	ON_BN_CLICKED(IDC_KILL, &CAdbControllerDlg::OnKill)
	ON_BN_CLICKED(IDC_REUSE, &CAdbControllerDlg::OnReuseCmd)
	ON_BN_CLICKED(IDCANCEL, &CAdbControllerDlg::OnCancel)

	ON_CONTROL_RANGE(EN_CHANGE, IDC_M_DEVICE_ID, IDC_S4_DEVICE_ID, &CAdbControllerDlg::OnCtrlRgn_Device_Id_Changed)
	ON_CONTROL_RANGE(EN_CHANGE, IDC_M_TOUCH_EVT, IDC_S4_TOUCH_EVT, &CAdbControllerDlg::OnCtrlRgn_Touch_Evt_Changed)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_M_SYNC_CHK, IDC_S4_SYNC_CHK, &CAdbControllerDlg::OnCtrlRgn_Sync_Checkbox)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_M_DEVICE_BTN, IDC_S4_DEVICE_BTN, &CAdbControllerDlg::OnCtrlRgn_Connect_Device)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_M_TOUCH_EVT_BTN, IDC_S4_TOUCH_EVT_BTN, &CAdbControllerDlg::OnCtrlRgn_Detect_Touch_Event)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_M_PLAY_SCRIPT_BTN, IDC_S4_PLAY_SCRIPT_BTN, &CAdbControllerDlg::OnCtrlRgn_Play_Event)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_SYNC_MOVE_MASTER, IDC_SYNC_MOVE_SLAVE_4, &CAdbControllerDlg::OnCtrlRgn_Sync_Change)
	ON_CONTROL_RANGE(EN_CHANGE, IDC_CENTER_X, IDC_MOVE_DURATION, &CAdbControllerDlg::OnCtrlRgn_Sync_Change)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_MOVE_UP, IDC_MOVE_DOWN, &CAdbControllerDlg::OnCtrlRgn_Move_Event)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_PACKAGE_M, IDC_PACKAGE_S4, &CAdbControllerDlg::OnCtrlRgn_Open_PackageDlg_Event)

	ON_BN_CLICKED(IDC_CMD_OUT_CLEAR, &CAdbControllerDlg::OnBnClickedCmdOutClear)
	ON_BN_CLICKED(IDC_STATUS_OUT_CLEAR, &CAdbControllerDlg::OnBnClickedStatusOutClear)
	ON_BN_CLICKED(IDC_CONNECT_DEVICES, &CAdbControllerDlg::OnBnClickedConnectDevices)
	ON_BN_CLICKED(IDC_DISCONNECT_DEVICES, &CAdbControllerDlg::OnBnClickedDisconnectDevices)
	ON_BN_CLICKED(IDC_MOVE_DEGREE, &CAdbControllerDlg::OnBnClickedMoveDegree)
	ON_BN_CLICKED(IDC_MOVE_XY, &CAdbControllerDlg::OnBnClickedMoveXy)
END_MESSAGE_MAP()



#define PRJ_DEVICE 			_T("device ")
#define PRJ_DEVICE_SERIAL 	_T("serial")
#define PRJ_TOUCH_EVT 		_T("touch_evt")
#define PRJ_SYNC_CHECK 		_T("sync_chk")

#define PRJ_MAIN 			_T("main")
#define PRJ_CMD_LINE		_T("cmd_line")

#define PRJ_SYNC_MOVE 			_T("sync_move")
#define PRJ_CENTER_X 			_T("center_x")
#define PRJ_CENTER_Y 			_T("center_y")
#define PRJ_MOVE_LEN 			_T("move_length")
#define PRJ_MOVE_DURATION 		_T("move_duration")
#define PRJ_SYNC_MOVE_MASTER	_T("move_master")
#define PRJ_SYNC_MOVE_SLAVE1	_T("move_slave1")
#define PRJ_SYNC_MOVE_SLAVE2	_T("move_sllave2")
#define PRJ_SYNC_MOVE_SLAVE3	_T("move_sllave3")
#define PRJ_SYNC_MOVE_SLAVE4	_T("move_sllave4")

#define PRJ_SCRIPT_FILE 		_T("script_file")

#define PRJ_MASTER				_T("master")
#define PRJ_SLAVE1				_T("sliave1")
#define PRJ_SLAVE2				_T("sliave2")
#define PRJ_SLAVE3				_T("sliave3")
#define PRJ_SLAVE4				_T("sliave4")





BOOL CAdbControllerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	SetWindowText(_T("AdbController Ver.0.02 beta"));

	CMenu* pSysMenu = GetSystemMenu(FALSE);

	if (pSysMenu != NULL) {
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);

		if (!strAboutMenu.IsEmpty()) {
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	ShowWindow(SW_MINIMIZE);

	// figure out the windows version

	OSVERSIONINFO osInfo;
	osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
#pragma warning(disable : 4996)
	GetVersionEx(&osInfo);
	m_bIsWindowsNT = osInfo.dwPlatformId == VER_PLATFORM_WIN32_NT;

	m_bDelayedQuit = FALSE;
	m_pLastCommand = NULL;
	m_bReusable = 0; // default==unchecked

	m_wEdit = (CEdit*)GetDlgItem(IDC_OUTPUT);
	m_wEdit->LimitText(32766); // enough for 1000 lines or so

	m_wStatus = (CEdit*)GetDlgItem(IDC_STATUS_OUTPUT);
	m_wStatus->LimitText(32766); // enough for 1000 lines or so

	m_wCommand = (CEdit*)GetDlgItem(IDC_COMMAND);

	CButton* button;
	// turn the break/kill buttons off & run button on
	button = (CButton*)GetDlgItem(IDC_BREAK);
	button->EnableWindow(FALSE);

	button = (CButton*)GetDlgItem(IDC_KILL);
	button->EnableWindow(FALSE);

	button = (CButton*)GetDlgItem(IDOK);
	button->EnableWindow(TRUE);

	button = (CButton*)GetDlgItem(IDC_M_SYNC_CHK);
	button->EnableWindow(FALSE);
	button->SetCheck(FALSE);


	InitializeCriticalSection(&m_SyncReceivedLock);
	InitializeCriticalSection(&m_SyncTouchLock);

	m_SyncTouchDataOrder = 0;

	LoadProfile();
	BuildAdbDevice();
	/*
	OutputString(m_wStatus, _T("AAA"));
	OutputString(m_wStatus, _T("CCC"));
	OutputString(m_wStatus, _T("x:%d, y:%d"), 100, 200);
	*/

	TCHAR NPath[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, NPath);
#ifdef _DEBUG
	m_sWorkingFolder = _T("V:\\_Application\\¤Ñ°ó\\AdbSyncApp");
#else
	m_sWorkingFolder = NPath;
#endif
	OutputString( m_wStatus, _T("Current Dir: %s"), m_sWorkingFolder);

	for(int idx = 0; idx < TOTAL_DEVICE;idx++){
		m_dlgPackage[idx] = NULL;
	}

	return TRUE;
}


void CAdbControllerDlg::OnDestroy()
{
	SaveProfile();

}



void CAdbControllerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX) {
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else {
		if(nID == SC_CLOSE){

			POSITION pos = m_SyncDevices.GetHeadPosition();

			while(pos) {
				CAdbDevice device = m_SyncDevices.GetNext(pos);
				SetEvent(device.PAdbThreadParam->hExitEvent);
				WaitForSingleObject(device.PAdbThreadParam->hExitFinishedEvent, 3000);
				CloseHandle(device.PAdbThreadParam->hTouchEvent);
				CloseHandle(device.PAdbThreadParam->hExitEvent);
				CloseHandle(device.PAdbThreadParam->hExitFinishedEvent);
				CloseHandle(device.PAdbThreadParam->hMoveEvent);
				CloseHandle(device.hThread);
				delete device.PAdbThreadParam;
			}
		}

		
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

void CAdbControllerDlg::OnPaint()
{
	if (IsIconic()) {
		CPaintDC dc(this);
		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		dc.DrawIcon(x, y, m_hIcon);
	}
	else {
		CDialogEx::OnPaint();
	}
}

HCURSOR CAdbControllerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CAdbControllerDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{

	//bHandled = FALSE; // allow the character to be processed/displayed

	if(!m_pLastCommand)
		return;

	// if this is the beginning of a "new" input, save the cursor position
	if(-1 == m_nStartChar) {
		int nEndChar;
		m_wEdit->GetSel(m_nStartChar, nEndChar);
	}

	// no need to handle WM_KEYDOWN, i can catch <return> here
	if(VK_RETURN == nChar) {
		// kludge: [m_nStartChar, thisChar] is considered the intended input
		// @@@ of course this "logic" can fail in many ways, eg arrow-moved to new line etc
		int nCur, nEndChar;
		m_wEdit->GetSel(nCur, nEndChar);
		ATLASSERT(m_nStartChar >= 0);

		if(nCur >= m_nStartChar) {
			// i had forgotten what a pain plain edit controls are!
			// EM_GETSELTEXT works only for richedits
			if(nCur != nEndChar)
				m_wEdit->SetSel(nCur, nCur); // cancel selection so that EM_LINELENGTH works ok

			int nStartLine = m_wEdit->LineIndex();
			ATLASSERT(nStartLine <= nCur);
			int nLen = m_wEdit->LineLength(nCur); // could be empty
			ATLASSERT(m_nStartChar <= nStartLine + nLen);

			ATLASSERT(sizeof(TCHAR)*nLen < 4096);
			LPTSTR pbuf = (LPTSTR)_alloca(sizeof(TCHAR) * (nLen + 3/*room for CR*/) );
			m_wEdit->GetLine(m_wEdit->LineFromChar(nCur), pbuf, nLen);
#ifdef DEBUG
			pbuf[nLen] = 0; // we don't get a terminator for free
			ATLASSERT(lstrlen(pbuf) == nLen);
#endif

			// i could have called Execute for convenient newline addition
			// but that isn't what we're actually doing, just passing info to the running proggy
			pbuf[nLen] = _T('\r');
			pbuf[nLen + 1] = _T('\n');
			pbuf[nLen + 2] = 0;

			if(m_nStartChar < nStartLine)
				m_nStartChar = nStartLine; // consecutive <Ret>s or other anomaly

			m_pLastCommand->SendChildInput(pbuf + m_nStartChar - nStartLine);
		}

		// else this is suspect, ignore!

		m_nStartChar = -1; // reset for next input cycle
	}
	else if(VK_CANCEL == nChar) {
		// whereas Pause on its own sends VK_PAUSE, with Ctrl it sends Cancel
		// basically no need to check for Control here
		m_pLastCommand->SendCtrlBrk();
	}

}

UINT CAdbControllerDlg::OnGetDlgCode()
{
	return DLGC_WANTALLKEYS; // i just need <return>
}

LRESULT CAdbControllerDlg::OnEndChildProcess(WPARAM wParam, LPARAM lParam)
{
	ATLASSERT(wParam == (WPARAM)m_pLastCommand); // hint, but we don't actually use it

	CButton* button;
	// turn the break/kill buttons off & run button on
	button = (CButton*)GetDlgItem(IDC_BREAK);
	//ATLASSERT(button.IsWindow());
	button->EnableWindow(FALSE);

	button = (CButton*)GetDlgItem(IDC_KILL);
	button->EnableWindow(FALSE);

	button = (CButton*)GetDlgItem(IDOK);
	//ATLASSERT(button.IsWindow());
	button->EnableWindow(TRUE);

	//delete m_pLastCommand;
	m_pLastCommand = NULL; // shouldn't be accessed any further
	// the listener thread frees the object, since CPF_NOAUTODELETE wasn't specified

	if(m_bDelayedQuit) {
		ATLASSERT(m_bReusable);
		PostMessage(WM_COMMAND, IDCANCEL);
	}

	return 0;
}


void CAdbControllerDlg::OnSyncTouchToSend(CString data)
{

	CString temp;
	int pos = data.Find(_T(": "));

	if(pos == -1) {
		OutputDebugString(_T("Parse Touch Event data fail.\n"));
		return;
	}

	data.Delete(0, pos + 2);
	//OutputDebugString(data + _T("\n"));


	if(data.GetLength() != TOUCH_DATA_LENGTH) {
		OutputDebugString(_T("Invalid touch data\n"));
		return;
	}

	// sendevent device type code value
	CStringList touch_data;
	SplitString(data, _T(' '), touch_data);

	if(touch_data.GetCount() != 3) {
		OutputDebugString(_T("Invalid touch data field\n"));
		return;
	}

	POSITION list_pos = touch_data.GetHeadPosition();
	TCHAR *end = NULL;
	long type = _tcstol (touch_data.GetNext(list_pos), &end, 16);
	long code = _tcstol (touch_data.GetNext(list_pos), &end, 16);
	long value = _tcstol (touch_data.GetNext(list_pos), &end, 16);

	list_pos = m_SyncDevices.GetHeadPosition();

	while(list_pos) {
		CAdbDevice device = m_SyncDevices.GetNext(list_pos);
		// adb -s NBQGLE25B1234567 shell sendevent /dev/input/event0 0003 0001 00000
		//CString cmd;
		//cmd.Format(_T("adb -s %s shell sendevent %s %d %d %d "), device.Serial, device.TouchEvt, type, code, value);
		//OutputDebugString(cmd + _T("\n"));
		//ExecCmdSimple((LPCTSTR)cmd);
		//ShellExecute(NULL, _T("open"), _T("adb"), cmd, NULL, SW_HIDE);
		//Sleep(100);
	}

}




void CAdbControllerDlg::OnSyncOneTouchToSend(CString data)
{
	if(data.Find(_T(": 0003 0035")) == -1 && data.Find(_T(": 0003 0036")) == -1) {
		return;
	}

	CString temp;
	int pos = data.Find(_T(": "));

	if(pos == -1) {
		OutputDebugString(_T("Parse Touch Event data fail.\n"));
		return;
	}

	data.Delete(0, pos + 2);
	//OutputDebugString(data + _T("\n"));

	if(data.GetLength() != TOUCH_DATA_LENGTH) {
		//OutputDebugString(_T("Invalid touch data\n"));
		return;
	}

	// sendevent device type code value
	CStringList touch_data;
	SplitString(data, _T(' '), touch_data);

	if(touch_data.GetCount() != 3) {
		//OutputDebugString(_T("Invalid touch data field\n"));
		return;
	}

	POSITION list_pos = touch_data.GetHeadPosition();
	TCHAR *end = NULL;
	long type = _tcstol (touch_data.GetNext(list_pos), &end, 16);
	long code = _tcstol (touch_data.GetNext(list_pos), &end, 16);
	long value = _tcstol (touch_data.GetNext(list_pos), &end, 16);

	if(value > 5000) {
		return;
	}

	/*
		sendevent device type code value

		<< HEX value >>
		/dev/input/event2: 0003 0035 0000048b << HEX value >>
		/dev/input/event2: 0003 0036 00000461 << HEX value >>
		/dev/input/event2: EV_ABS		ABS_MT_POSITION_X	 0000048b
		/dev/input/event2: EV_ABS		ABS_MT_POSITION_Y	 00000461
	*/
	// X
	if(type == 0x0003 && code == 0x0035) {
		m_TapXY.x = value;
		m_TapXY.y = 0;
		return;
	}
	// Y
	else if(type == 0x0003 && code == 0x0036) {
		if( m_TapXY.x == 0) {
			OutputDebugString(_T("Invalid tap x-y\n"));
			return;
		}

		m_TapXY.y = value;
	}
	else {
		return;
	}

	OutputString( m_wStatus, _T("Touch %d %d"), m_TapXY.x, m_TapXY.y);

	list_pos = m_SyncDevices.GetHeadPosition();

	while(list_pos) {
		CAdbDevice device = m_SyncDevices.GetNext(list_pos);
		device.PAdbThreadParam->ptTapXY = m_TapXY;
		SetEvent(device.PAdbThreadParam->hTouchEvent);
	}

}




LRESULT CAdbControllerDlg::OnSyncTouch(WPARAM wParam, LPARAM lParam)
{
	EnterCriticalSection (&m_SyncTouchLock);

	CString* pContent = (CString*)wParam;

	CString temp;
	temp.Format(_T("SyncTouchData: Recv [%d] %d words\n"), (int)lParam, pContent->GetLength());
	OutputDebugString(temp);

	CStringList lines;
	SplitString(*pContent, _T('\n'), lines);


	if(m_bNeedUpdateSyncDeviceList) {
		UpdateSyncList();
		m_bNeedUpdateSyncDeviceList = FALSE;
	}


	POSITION pos = lines.GetHeadPosition();
	CString line;

	if(m_SyncToucnRemainData.GetLength() > 0) {
		line = lines.GetHead();

		if(line.GetLength() != TOUCH_EVENT_LENGTH
		        && (line.GetLength() + m_SyncToucnRemainData.GetLength()) == TOUCH_EVENT_LENGTH) {
			line = m_SyncToucnRemainData + line;
			//OutputDebugString(line + _T("\n"));
			lines.GetNext(pos);
			//OnSyncTouchToSend(line);
			OnSyncOneTouchToSend(line);
		}
	}

	while(pos) {
		line = lines.GetNext(pos);

		if(line.GetLength() == TOUCH_EVENT_LENGTH) {
			//OutputDebugString(line + _T("\n"));
			//OnSyncTouchToSend(line);
			OnSyncOneTouchToSend(line);
		}
		else if(pos == NULL) {
			m_SyncToucnRemainData = line;
		}
	}


	delete pContent;

	LeaveCriticalSection (&m_SyncTouchLock);

	return 0;
}


LRESULT CAdbControllerDlg::OnCloseSystemExecThread(WPARAM wParam, LPARAM lParam)
{

	PSystemExecParam pParam = (PSystemExecParam)wParam;

	// wait for thread exit.
	Sleep(100);
	CloseHandle(pParam->hThread);
	delete pParam;

	return 0;
}




void CAdbControllerDlg::OnRun()
{
	//CDialogEx::OnOK();
	// reset the "input buffer" info
	SaveProfile();

	m_nStartChar = -1;

	CEdit* wndCmd = (CEdit*)GetDlgItem(IDC_COMMAND);

	CString cmd;
	wndCmd->GetWindowText(cmd);

	if( cmd.IsEmpty()) {
		MessageBox(_T("Please enter a command to run"), _T("Console"));
		wndCmd->SetFocus();
		return ;
	}

	ATLASSERT(m_pLastCommand == 0 || m_bReusable);

	// the new reusable mode has made a shambles out of my GUI structure...
	if(m_pLastCommand) {
		ATLASSERT(m_pLastCommand->m_dwFlags & CPF_REUSECMDPROC);
		// queue command to the existing command processor, like as if someone was typing it
		// SendChildInput could also have been used
		m_pLastCommand->Execute(cmd);
		return ;
	}

	CEdit* out = (CEdit*)GetDlgItem(IDC_OUTPUT);
	// create redirection plumbings
	DWORD flags = m_bReusable ? CPF_REUSECMDPROC : 0;
	m_pLastCommand = new CMyConsolePipe(m_bIsWindowsNT, out, this, (WND_RECV_PMSG)&CAdbControllerDlg::OnTouchSyncReceived, flags);

	if(!m_pLastCommand)
		return ; // rare: memory is tight

	int status = m_pLastCommand->Execute(cmd);

	if(CPEXEC_OK == status) {
		// turn the break/kill button on & run button off
		CButton* button = (CButton*)GetDlgItem(IDC_BREAK);
		//ATLASSERT(button.IsWindow());
		button->EnableWindow();
		//button = (CButton*)GetDlgItem(IDC_KILL);
		//button->EnableWindow();

		if(!m_bReusable) {
			button = (CButton*)GetDlgItem(IDOK/*run*/);
			//ATLASSERT(button.IsWindow());
			button->EnableWindow(FALSE);
		}

		// in this particular sample we limit at one active command at a time
		// but the CConsolePipe class has no such limitations
		// (however you need a new instance for each new command -- unless in reusable mode)
	}
	else {
		// error conditions are rare since the intermediate command processor is always started
		// however if status==CPEXEC_MISC_ERROR you can try a no-frills CreateProcess

		// if the thread didn't start we have to cleanup manually
		ATLASSERT(!m_pLastCommand->IsChildRunning());
		delete m_pLastCommand;
		m_pLastCommand = 0;

		MessageBox(_T("Can't execute that!"), _T("Console"));
		wndCmd->SetSel(0, -1);
		wndCmd->SetFocus();
	}


}


void CAdbControllerDlg::OnBreak()
{
	OutputDebugString(_T("OnBreak()\n"));

	if (m_pLastCommand == NULL) {
		return;
	}

	ATLASSERT(m_pLastCommand);
	m_pLastCommand->SendCtrlBrk();
}


void CAdbControllerDlg::OnKill()
{
	OutputDebugString(_T("OnKill()\n"));
	ATLASSERT(m_pLastCommand);

	CMyConsolePipe* save = m_pLastCommand;
	// unfortunately this won't terminate the listener thread cleanly
	SendMessage(UWM_PIPEBROKEN, (WPARAM)save); // simulate event

	ATLASSERT(0 == m_pLastCommand);
	save->Break();
}


void CAdbControllerDlg::OnReuseCmd()
{
	CButton* button = (CButton*)GetDlgItem(IDC_REUSE);

	ATLASSERT(!m_bReusable);
	ATLASSERT(button->GetCheck());

	m_bReusable = TRUE;
	// once in this mode, we can't get out of it for simplicity of interface/logic
	button->EnableWindow(FALSE);
}


void CAdbControllerDlg::OnCancel()
{
	// for reusable mode, send the exit command
	if(m_pLastCommand && (m_pLastCommand->m_dwFlags & CPF_REUSECMDPROC) ) {
		ATLASSERT(m_bReusable);
		m_pLastCommand->StopCmd();

		// to avoid leaks, wait for the termination event (message)
		// but obviously since it's _sent_ i'll never know about it with a local message pump

		// wait for the bg thread to quit before ending the dialog
		ATLASSERT(!m_bDelayedQuit);
		m_bDelayedQuit = TRUE;
		return ;
	}

	// any command left running... won't be living for long!

	if(m_bDelayedQuit)
		Sleep(100); // allow bg thread to cleanup

	CDialogEx::OnCancel();

	return;
}



void CAdbControllerDlg::OnBnClickedDisconnectDevices()
{
	int retval = ::_tsystem( _T("taskkill /F /T /IM adb.exe") );
	OutputString( m_wStatus, _T("Kill all adb servers, ret:%d"), retval);
}


void CAdbControllerDlg::OnCtrlRgn_Device_Id_Changed(UINT nID)
{
	CString temp;
	temp.Format(_T("OnCtrlRgn_Device_Id_Changed(%d)\n"), nID);
	OutputDebugString(temp);
	m_bNeedUpdateSyncDeviceList = TRUE;
}

void CAdbControllerDlg::OnCtrlRgn_Touch_Evt_Changed(UINT nID)
{
	CString temp;
	temp.Format(_T("OnCtrlRgn_Touch_Evt_Changed(%d)\n"), nID);
	OutputDebugString(temp);
	m_bNeedUpdateSyncDeviceList = TRUE;
}



void CAdbControllerDlg::OnCtrlRgn_Sync_Checkbox(UINT nID)
{
	CString temp;
	temp.Format(_T("OnCtrlRgn_Sync_Checkbox(%d)\n"), nID);
	OutputDebugString(temp);
	m_bNeedUpdateSyncDeviceList = TRUE;
}

void CAdbControllerDlg::OnCtrlRgn_Connect_Device(UINT nID)
{
	OnBreak();

	CString temp;
	temp.Format(_T("OnCtrlRgn_Find_Device(%d)\n"), nID);
	OutputDebugString(temp);
	int nDeviceIdx = FindDeviceIndex( nID, offsetof(struct _DEVICE_CONTROL_IDS, nBtnD_FindDevice));

	if(nDeviceIdx == -1) {
		return;
	}

	CString serial;
	GetDlgItem(G_DEVICE_CONTROL_MAP[nDeviceIdx].nEditID_Device)->GetWindowText(serial);
	CString cmd;
	cmd.Format(_T("adb connect %s"), serial);
	OutputString( m_wStatus, _T("connect %s"), serial);
	m_wCommand->SetWindowText(cmd);

	OnRun();

	m_bNeedUpdateSyncDeviceList = TRUE;

}

void CAdbControllerDlg::OnCtrlRgn_Detect_Touch_Event(UINT nID)
{
	OnBreak();

	CString temp;
	temp.Format(_T("OnCtrlRgn_Detect_Touch_Event(%d)\n"), nID);
	OutputDebugString(temp);

	int nDeviceIdx = FindDeviceIndex( nID, offsetof(struct _DEVICE_CONTROL_IDS, nBtnD_DetectTouchEvt));

	if(nDeviceIdx == -1) {
		return;
	}

	CString serial;
	GetDlgItem(G_DEVICE_CONTROL_MAP[nDeviceIdx].nEditID_Device)->GetWindowText(serial);
	CString cmd;
	cmd.Format(_T("adb -s %s shell getevent"), serial);
	m_wCommand->SetWindowText(cmd);

	EnableDeviceControl(TRUE, offsetof(struct _DEVICE_CONTROL_IDS, nChkID_Sync));
	CheckDeviceControl(1, offsetof(struct _DEVICE_CONTROL_IDS, nChkID_Sync));
	CButton* button = (CButton*)GetDlgItem(G_DEVICE_CONTROL_MAP[nDeviceIdx].nChkID_Sync);
	button->SetCheck(FALSE);
	button->EnableWindow(FALSE);

	m_bNeedUpdateSyncDeviceList = TRUE;

}

void CAdbControllerDlg::OnCtrlRgn_Play_Event(UINT nID)
{
	CString temp;
	temp.Format(_T("OnCtrlRgn_Play_Event(%d)\n"), nID);
	OutputDebugString(temp);

	int nDeviceIdx = FindDeviceIndex( nID, offsetof(struct _DEVICE_CONTROL_IDS, nBtnD_Play_Script));

	if(nDeviceIdx == -1) {
		return;
	}

	CString text;
	GetDlgItem(G_DEVICE_CONTROL_MAP[nDeviceIdx].nEditD_Script_File)->GetWindowText(text);


	PSystemExecParam pParam = new SystemExecParam;

	pParam->wnd = this;
	pParam->wStatus = m_wStatus;
	pParam->btnPlay = (CButton*)GetDlgItem(G_DEVICE_CONTROL_MAP[nDeviceIdx].nBtnD_Play_Script);
	pParam->cmdLine = text;
	pParam->nNoityfCloseThreadEvent = UWM_CLOSE_SYS_EXEC_THREAD;
	pParam->hThread =
	    CreateThread(
	        NULL,					// default security attributes
	        0,						// use default stack size
	        SystemExecuteThread,	// thread function name
	        pParam, 				// argument to thread function
	        0,						// use default creation flags
	        NULL);					// returns the thread identifier

}


void CAdbControllerDlg::OnCtrlRgn_Sync_Change(UINT nID)
{
	CString temp;
	temp.Format(_T("OnCtrlRgn_Sync_Change(%d)\n"), nID);
	OutputDebugString(temp);
	m_bNeedUpdateMoveSync = TRUE;
}

#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif

#define R_X( r, degree) ((double)r*cos((double)degree*M_PI/180.0f))
#define R_Y( r, degree) ((double)r*sin((double)degree*M_PI/180.0f))

#define Joy_X(center_x, r, degree) (center_x + (int)R_X(r, degree))
#define Joy_Y(center_y, r, degree) (center_y - (int)R_Y(r, degree))


void CAdbControllerDlg::OnCtrlRgn_Move_Event(UINT nID)
{
	CString temp;
	temp.Format(_T("OnCtrlRgn_Move_Event(%d)\n"), nID);
	OutputDebugString(temp);

	int degree = 0;

	if(m_bNeedUpdateMoveSync) {
		UpdateMove();
		m_bNeedUpdateMoveSync = FALSE;
	}

	CPoint pt2;

	/*
		ref: https://market.cloud.edu.tw/content/senior/math/tn_t2/math01/3th/4-1/4-1.htm
		center point: p0(x0,y0)
		target point: p(x, y)
		length: r
		(x-x0)^2+(y-y0)^2 = r^2

		x = r*cos(@)
		y = r*sin(@)
	*/
	if (nID == IDC_MOVE_UP) 				degree = 90;
	else if (nID == IDC_MOVE_LEFT) 			degree = 180;
	else if (nID == IDC_MOVE_LEFT_UP) 		degree = 135;
	else if (nID == IDC_MOVE_LEFT_UP_UP)	degree = 112;
	else if (nID == IDC_MOVE_LEFT_UP_DW) 	degree = 158;
	else if (nID == IDC_MOVE_LEFT_DW) 		degree = 225;
	else if (nID == IDC_MOVE_LEFT_DW_UP)	degree = 203;
	else if (nID == IDC_MOVE_LEFT_DW_DW)	degree = 248;
	else if (nID == IDC_MOVE_RIGHT) 		degree = 0;
	else if (nID == IDC_MOVE_RIGHT_UP) 		degree = 45;
	else if (nID == IDC_MOVE_RIGHT_UP_UP) 	degree = 68;
	else if (nID == IDC_MOVE_RIGHT_UP_DW) 	degree = 23;
	else if (nID == IDC_MOVE_RIGHT_DW) 		degree = 315;
	else if (nID == IDC_MOVE_RIGHT_DW_UP)	degree = 348;
	else if (nID == IDC_MOVE_RIGHT_DW_DW)	degree = 293;
	else if (nID == IDC_MOVE_DOWN)			degree = 270;
	else return;

	pt2.x = Joy_X( m_ptMove.x, m_MoveLength, degree);
	pt2.y = Joy_Y( m_ptMove.y, m_MoveLength, degree);

	POSITION pos = m_SyncDevices.GetHeadPosition();

	while(pos) {
		CAdbDevice device = m_SyncDevices.GetNext(pos);
		// input swipe 150 565 150 450 10000
		device.PAdbThreadParam->ptStartXY = m_ptMove;
		device.PAdbThreadParam->ptEndXY = pt2;
		device.PAdbThreadParam->nMoveDuration = m_MoveDuration;

		SetEvent(device.PAdbThreadParam->hMoveEvent);
	}
}


void CAdbControllerDlg::OnBnClickedMoveDegree()
{
	CEdit* editor;
	CString text;
	int degree = 0;
	CPoint pt2;

	if(m_bNeedUpdateMoveSync) {
		UpdateMove();
		m_bNeedUpdateMoveSync = FALSE;
	}

	editor = (CEdit*)GetDlgItem(IDC_DEGREE);
	editor->GetWindowText(text);
	degree = _wtoi((LPCTSTR)text);

	pt2.x = Joy_X( m_ptMove.x, m_MoveLength, degree);
	pt2.y = Joy_Y( m_ptMove.y, m_MoveLength, degree);


	OutputString( m_wStatus, _T("cos %f, sin %f"), cos(degree), sin(degree));
	OutputString( m_wStatus, _T("R_X %d, R_Y %d"), R_X( m_MoveLength, degree), R_Y( m_MoveLength, degree));



	POSITION pos = m_SyncDevices.GetHeadPosition();

	while(pos) {
		CAdbDevice device = m_SyncDevices.GetNext(pos);
		// input swipe 150 565 150 450 10000
		device.PAdbThreadParam->ptStartXY = m_ptMove;
		device.PAdbThreadParam->ptEndXY = pt2;
		device.PAdbThreadParam->nMoveDuration = m_MoveDuration;

		SetEvent(device.PAdbThreadParam->hMoveEvent);
	}

}

void CAdbControllerDlg::OnBnClickedMoveXy()
{
	CEdit* editor;
	CString text;
	int degree = 0;
	CPoint pt2;

	if(m_bNeedUpdateMoveSync) {
		UpdateMove();
		m_bNeedUpdateMoveSync = FALSE;
	}

	editor = (CEdit*)GetDlgItem(IDC_POINT_X);
	editor->GetWindowText(text);
	pt2.x = _wtoi((LPCTSTR)text);

	editor = (CEdit*)GetDlgItem(IDC_POINT_Y);
	editor->GetWindowText(text);
	pt2.y = _wtoi((LPCTSTR)text);

	OutputString( m_wStatus, _T("Touch %d %d"), m_TapXY.x, m_TapXY.y);

	POSITION pos = m_SyncDevices.GetHeadPosition();

	while(pos) {
		CAdbDevice device = m_SyncDevices.GetNext(pos);
		device.PAdbThreadParam->ptTapXY = pt2;
		SetEvent(device.PAdbThreadParam->hTouchEvent);
	}

}



void CAdbControllerDlg::OnTouchSyncReceived(LPCTSTR pszText)
{
	EnterCriticalSection (&m_SyncReceivedLock);

	CString* pContent = new CString(pszText);
	CString temp;
	temp.Format(_T("SyncTouchData: Send [%d] %d words\n"), (int)m_SyncTouchDataOrder, pContent->GetLength());
	OutputDebugString(temp);
	PostMessage(UWM_SYNC_TOUPCH, (WPARAM)pContent, (LPARAM)m_SyncTouchDataOrder);
	m_SyncTouchDataOrder++;

	LeaveCriticalSection (&m_SyncReceivedLock);

}

void CAdbControllerDlg::OnBnClickedCmdOutClear()
{
	CEdit* edit = (CEdit*)GetDlgItem(IDC_OUTPUT);
	edit->SetWindowText(_T(""));
}

void CAdbControllerDlg::OnBnClickedStatusOutClear()
{
	CEdit* edit = (CEdit*)GetDlgItem(IDC_STATUS_OUTPUT);
	edit->SetWindowText(_T(""));
}

void CAdbControllerDlg::OnBnClickedConnectDevices()
{
	OnBreak();

	for(int idx = DEV_MASTER; idx < TOTAL_DEVICE; idx++) {
		CString serial;
		GetDlgItem(G_DEVICE_CONTROL_MAP[idx].nEditID_Device)->GetWindowText(serial);
		CString cmd;
		cmd.Format(_T("adb connect %s"), serial);
		OutputString( m_wStatus, _T("connect %s"), serial);
		ExecCmdSimple(cmd);
	}

	m_wCommand->SetWindowText(_T("adb devices"));
	OnRun();

}



LRESULT CAdbControllerDlg::OnClosePackageDialog(WPARAM wParam, LPARAM lParam)
{

	int nDeviceIdx = (int)wParam;

	CString temp;
	temp.Format(_T("OnClosePackageDialog(%d)\n"), nDeviceIdx);
	OutputDebugString(temp);
	
	ASSERT(nDeviceIdx >= 0 && nDeviceIdx < TOTAL_DEVICE);

	delete m_dlgPackage[nDeviceIdx];
	m_dlgPackage[nDeviceIdx] = NULL;

	return 0;
}

void CAdbControllerDlg::OnCtrlRgn_Open_PackageDlg_Event(UINT nID)
{
	CString temp;
	temp.Format(_T("OnCtrlRgn_Open_PackageDlg_Event(%d)\n"), nID);
	OutputDebugString(temp);


	int nDeviceIdx = FindDeviceIndex( nID, offsetof(struct _DEVICE_CONTROL_IDS, nBtnD_OpenPkgDlg));
	if(nDeviceIdx == -1) {
		return;
	}

	
	CEdit* editorDevice = (CEdit*)GetDlgItem(G_DEVICE_CONTROL_MAP[nDeviceIdx].nEditID_Device);
	CString serial;
	editorDevice->GetWindowText(serial);


	//m_pSimpleDialog initialized to NULL in the constructor of CMyDialog class
	m_dlgPackage[nDeviceIdx] = new CPackageDlg( this, nDeviceIdx, serial, UWM_CLOSE_PACKAGE_DLG, m_sWorkingFolder);

	//Check if new succeeded and we got a valid pointer to a dialog object
	if(m_dlgPackage[nDeviceIdx] != NULL) {
		BOOL ret = ((CPackageDlg*)m_dlgPackage[nDeviceIdx])->Create();

		if(!ret) AfxMessageBox(_T("Error creating Dialog"));

		m_dlgPackage[nDeviceIdx]->ShowWindow(SW_SHOW);
	}
	else {
		AfxMessageBox(_T("Error Creating Dialog Object"));
	}

}


void CAdbControllerDlg::UpdateSyncList()
{
	POSITION pos = m_SyncDevices.GetHeadPosition();
	POSITION targetPos = pos;

	while(pos) {
		CAdbDevice device = m_SyncDevices.GetNext(pos);
		CButton* checkBox = (CButton*)GetDlgItem(G_DEVICE_CONTROL_MAP[device.DeviceIdx].nChkID_Sync);
		CEdit* editorDevice = (CEdit*)GetDlgItem(G_DEVICE_CONTROL_MAP[device.DeviceIdx].nEditID_Device);
		CEdit* editorTouchEvt = (CEdit*)GetDlgItem(G_DEVICE_CONTROL_MAP[device.DeviceIdx].nEditID_TouchEvt);

		editorDevice->GetWindowText(device.PAdbThreadParam->Serial);
		editorTouchEvt->GetWindowText(device.PAdbThreadParam->TouchEvt);
		device.PAdbThreadParam->bSync = checkBox->GetCheck() > 0;
		//m_SyncDevices.SetAt(targetPos, device);
		//targetPos = pos;
	}

}

void CAdbControllerDlg::UpdateMove()
{
	POSITION pos = m_SyncDevices.GetHeadPosition();
	POSITION targetPos = pos;

	while(pos) {
		CAdbDevice device = m_SyncDevices.GetNext(pos);
		CButton* checkBox = (CButton*)GetDlgItem(G_DEVICE_CONTROL_MAP[device.DeviceIdx].nBtnD_MoveSync);
		device.PAdbThreadParam->bMoveSync = checkBox->GetCheck() > 0;
	}

	CEdit* editor;
	CString text;

	editor = (CEdit*)GetDlgItem(IDC_CENTER_X);
	editor->GetWindowText(text);
	m_ptMove.x = _wtoi((LPCTSTR)text);

	editor = (CEdit*)GetDlgItem(IDC_CENTER_Y);
	editor->GetWindowText(text);
	m_ptMove.y = _wtoi((LPCTSTR)text);

	editor = (CEdit*)GetDlgItem(IDC_MOVE_OFFSET);
	editor->GetWindowText(text);
	m_MoveLength = _wtoi((LPCTSTR)text);

	editor = (CEdit*)GetDlgItem(IDC_MOVE_DURATION);
	editor->GetWindowText(text);
	m_MoveDuration = _wtoi((LPCTSTR)text);

}

void CAdbControllerDlg::BuildAdbDevice()
{

	for(int idx = DEV_MASTER; idx < TOTAL_DEVICE; idx++) {
		CButton* checkBox = (CButton*)GetDlgItem(G_DEVICE_CONTROL_MAP[idx].nChkID_Sync);
		CEdit* editorDevice = (CEdit*)GetDlgItem(G_DEVICE_CONTROL_MAP[idx].nEditID_Device);
		CEdit* editorTouchEvt = (CEdit*)GetDlgItem(G_DEVICE_CONTROL_MAP[idx].nEditID_TouchEvt);

		CAdbDevice device;
		device.PAdbThreadParam = new AdbThreadData;
		editorDevice->GetWindowText(device.PAdbThreadParam->Serial);
		editorTouchEvt->GetWindowText(device.PAdbThreadParam->TouchEvt);
		device.PAdbThreadParam->wndStatus = m_wStatus;
		device.PAdbThreadParam->bSync = checkBox->GetCheck() > 0;
		device.PAdbThreadParam->hExitEvent =
		    CreateEvent(
		        NULL,	// default security attributes
		        FALSE,	// manual-reset event
		        FALSE,	// initial state is nonsignaled
		        NULL  	// object name
		    );
		device.PAdbThreadParam->hTouchEvent =
		    CreateEvent(
		        NULL,	// default security attributes
		        FALSE,	// manual-reset event
		        FALSE,	// initial state is nonsignaled
		        NULL  	// object name
		    );
		device.PAdbThreadParam->hExitFinishedEvent =
		    CreateEvent(
		        NULL,	// default security attributes
		        FALSE,	// manual-reset event
		        FALSE,	// initial state is nonsignaled
		        NULL  	// object name
		    );
		device.PAdbThreadParam->hMoveEvent =
		    CreateEvent(
		        NULL,	// default security attributes
		        FALSE,	// manual-reset event
		        FALSE,	// initial state is nonsignaled
		        NULL  	// object name
		    );

		device.DeviceIdx = idx;
		device.hThread =
		    CreateThread(
		        NULL,			// default security attributes
		        0,				// use default stack size
		        AdbExecThread,	// thread function name
		        device.PAdbThreadParam,	// argument to thread function
		        0,				// use default creation flags
		        NULL);			// returns the thread identifier

		m_SyncDevices.AddTail(device);
	}

}



// ref: https://msdn.microsoft.com/en-us/library/62txabd8(v=vs.80).aspx
void CAdbControllerDlg::LoadProfile()
{
	CWinApp* pApp = AfxGetApp();
	CEdit* editor;
	CButton* checkBox;
	CString text;


	for(int idx = DEV_MASTER; idx < TOTAL_DEVICE; idx++) {
		CString section;
		CString item;
		CString text;

		section.Format(_T("%s%d"), PRJ_DEVICE, idx);

		editor = (CEdit*)GetDlgItem(G_DEVICE_CONTROL_MAP[idx].nEditID_Device);
		editor->SetWindowText(pApp->GetProfileString(section, PRJ_DEVICE_SERIAL));

		editor = (CEdit*)GetDlgItem(G_DEVICE_CONTROL_MAP[idx].nEditID_TouchEvt);
		editor->SetWindowText(pApp->GetProfileString(section, PRJ_TOUCH_EVT));

		checkBox = (CButton*)GetDlgItem(G_DEVICE_CONTROL_MAP[idx].nChkID_Sync);
		checkBox->SetCheck(pApp->GetProfileInt(section, PRJ_SYNC_CHECK, 0));
	}

	/*
		Main
	*/
	editor = (CEdit*)GetDlgItem(IDC_COMMAND);
	editor->SetWindowText(pApp->GetProfileString(PRJ_MAIN, PRJ_CMD_LINE));

	/*
		Sync Move
	*/
	editor = (CEdit*)GetDlgItem(IDC_CENTER_X);
	editor->SetWindowText(pApp->GetProfileString(PRJ_SYNC_MOVE, PRJ_CENTER_X));

	editor = (CEdit*)GetDlgItem(IDC_CENTER_Y);
	editor->SetWindowText(pApp->GetProfileString(PRJ_SYNC_MOVE, PRJ_CENTER_Y));

	editor = (CEdit*)GetDlgItem(IDC_MOVE_OFFSET);
	editor->SetWindowText(pApp->GetProfileString(PRJ_SYNC_MOVE, PRJ_MOVE_LEN));

	editor = (CEdit*)GetDlgItem(IDC_MOVE_DURATION);
	editor->SetWindowText(pApp->GetProfileString(PRJ_SYNC_MOVE, PRJ_MOVE_DURATION));

	checkBox = (CButton*)GetDlgItem(IDC_SYNC_MOVE_MASTER);
	checkBox->SetCheck(pApp->GetProfileInt(PRJ_SYNC_MOVE, PRJ_MASTER, 0));

	checkBox = (CButton*)GetDlgItem(IDC_SYNC_MOVE_SLAVE_1);
	checkBox->SetCheck(pApp->GetProfileInt(PRJ_SYNC_MOVE, PRJ_SLAVE1, 0));

	checkBox = (CButton*)GetDlgItem(IDC_SYNC_MOVE_SLAVE_2);
	checkBox->SetCheck(pApp->GetProfileInt(PRJ_SYNC_MOVE, PRJ_SLAVE2, 0));

	checkBox = (CButton*)GetDlgItem(IDC_SYNC_MOVE_SLAVE_3);
	checkBox->SetCheck(pApp->GetProfileInt(PRJ_SYNC_MOVE, PRJ_SLAVE3, 0));

	checkBox = (CButton*)GetDlgItem(IDC_SYNC_MOVE_SLAVE_4);
	checkBox->SetCheck(pApp->GetProfileInt(PRJ_SYNC_MOVE, PRJ_SLAVE4, 0));

	/*
		Script file
	*/
	editor = (CEdit*)GetDlgItem(IDC_M_SCRIPT_FILE);
	editor->SetWindowText(pApp->GetProfileString(PRJ_SCRIPT_FILE, PRJ_MASTER));

	editor = (CEdit*)GetDlgItem(IDC_S1_SCRIPT_FILE);
	editor->SetWindowText(pApp->GetProfileString(PRJ_SCRIPT_FILE, PRJ_SLAVE1));

	editor = (CEdit*)GetDlgItem(IDC_S2_SCRIPT_FILE);
	editor->SetWindowText(pApp->GetProfileString(PRJ_SCRIPT_FILE, PRJ_SLAVE2));

	editor = (CEdit*)GetDlgItem(IDC_S3_SCRIPT_FILE);
	editor->SetWindowText(pApp->GetProfileString(PRJ_SCRIPT_FILE, PRJ_SLAVE3));

	editor = (CEdit*)GetDlgItem(IDC_S4_SCRIPT_FILE);
	editor->SetWindowText(pApp->GetProfileString(PRJ_SCRIPT_FILE, PRJ_SLAVE4));





}


void CAdbControllerDlg::SaveProfile()
{
	CWinApp* pApp = AfxGetApp();
	CEdit* editor;
	CButton* checkBox;
	CString text;


	for(int idx = DEV_MASTER; idx < TOTAL_DEVICE; idx++) {
		CString section;
		CString item;

		section.Format(_T("%s%d"), PRJ_DEVICE, idx);

		editor = (CEdit*)GetDlgItem(G_DEVICE_CONTROL_MAP[idx].nEditID_Device);
		editor->GetWindowText(text);
		pApp->WriteProfileString(section, PRJ_DEVICE_SERIAL, text);

		editor = (CEdit*)GetDlgItem(G_DEVICE_CONTROL_MAP[idx].nEditID_TouchEvt);
		editor->GetWindowText(text);
		pApp->WriteProfileString(section, PRJ_TOUCH_EVT, text);

		checkBox = (CButton*)GetDlgItem(G_DEVICE_CONTROL_MAP[idx].nChkID_Sync);
		pApp->WriteProfileInt(section, PRJ_SYNC_CHECK, checkBox->GetCheck());
	}

	/*
		Main
	*/
	editor = (CEdit*)GetDlgItem(IDC_COMMAND);
	editor->GetWindowText(text);
	pApp->WriteProfileString( PRJ_MAIN, PRJ_CMD_LINE, text);

	/*
		Sync Move
	*/
	editor = (CEdit*)GetDlgItem(IDC_CENTER_X);
	editor->GetWindowText(text);
	pApp->WriteProfileString( PRJ_SYNC_MOVE, PRJ_CENTER_X, text);

	editor = (CEdit*)GetDlgItem(IDC_CENTER_Y);
	editor->GetWindowText(text);
	pApp->WriteProfileString( PRJ_SYNC_MOVE, PRJ_CENTER_Y, text);

	editor = (CEdit*)GetDlgItem(IDC_MOVE_OFFSET);
	editor->GetWindowText(text);
	pApp->WriteProfileString( PRJ_SYNC_MOVE, PRJ_MOVE_LEN, text);

	editor = (CEdit*)GetDlgItem(IDC_MOVE_DURATION);
	editor->GetWindowText(text);
	pApp->WriteProfileString( PRJ_SYNC_MOVE, PRJ_MOVE_DURATION, text);

	checkBox = (CButton*)GetDlgItem(IDC_SYNC_MOVE_MASTER);
	pApp->WriteProfileInt(PRJ_SYNC_MOVE, PRJ_MASTER, checkBox->GetCheck());

	checkBox = (CButton*)GetDlgItem(IDC_SYNC_MOVE_SLAVE_1);
	pApp->WriteProfileInt(PRJ_SYNC_MOVE, PRJ_SLAVE1, checkBox->GetCheck());

	checkBox = (CButton*)GetDlgItem(IDC_SYNC_MOVE_SLAVE_2);
	pApp->WriteProfileInt(PRJ_SYNC_MOVE, PRJ_SLAVE2, checkBox->GetCheck());

	checkBox = (CButton*)GetDlgItem(IDC_SYNC_MOVE_SLAVE_3);
	pApp->WriteProfileInt(PRJ_SYNC_MOVE, PRJ_SLAVE3, checkBox->GetCheck());

	checkBox = (CButton*)GetDlgItem(IDC_SYNC_MOVE_SLAVE_4);
	pApp->WriteProfileInt(PRJ_SYNC_MOVE, PRJ_SLAVE4, checkBox->GetCheck());

	/*
		Script file
	*/
	editor = (CEdit*)GetDlgItem(IDC_M_SCRIPT_FILE);
	editor->GetWindowText(text);
	pApp->WriteProfileString( PRJ_SCRIPT_FILE, PRJ_MASTER, text);

	editor = (CEdit*)GetDlgItem(IDC_S1_SCRIPT_FILE);
	editor->GetWindowText(text);
	pApp->WriteProfileString( PRJ_SCRIPT_FILE, PRJ_SLAVE1, text);

	editor = (CEdit*)GetDlgItem(IDC_S2_SCRIPT_FILE);
	editor->GetWindowText(text);
	pApp->WriteProfileString( PRJ_SCRIPT_FILE, PRJ_SLAVE2, text);

	editor = (CEdit*)GetDlgItem(IDC_S3_SCRIPT_FILE);
	editor->GetWindowText(text);
	pApp->WriteProfileString( PRJ_SCRIPT_FILE, PRJ_SLAVE3, text);

	editor = (CEdit*)GetDlgItem(IDC_S4_SCRIPT_FILE);
	editor->GetWindowText(text);
	pApp->WriteProfileString( PRJ_SCRIPT_FILE, PRJ_SLAVE4, text);



}


int CAdbControllerDlg::FindDeviceIndex(UINT nControlId, int member)
{

	for(int idx = DEV_MASTER; idx < TOTAL_DEVICE; idx++) {

		UINT nCtrlId = *(UINT *)((char *)&G_DEVICE_CONTROL_MAP[idx] + member);

		if(nCtrlId == nControlId) {
			return idx;
		}
	}

	return -1;

}

void CAdbControllerDlg::EnableDeviceControl(BOOL bEnable, int member)
{
	for(int idx = DEV_MASTER; idx < TOTAL_DEVICE; idx++) {

		UINT nCtrlId = *(UINT *)((char *)&G_DEVICE_CONTROL_MAP[idx] + member);
		GetDlgItem(nCtrlId)->EnableWindow(bEnable);
	}
}

void CAdbControllerDlg::CheckDeviceControl(int nCheck, int member)
{
	for(int idx = DEV_MASTER; idx < TOTAL_DEVICE; idx++) {
		UINT nCtrlId = *(UINT *)((char *)&G_DEVICE_CONTROL_MAP[idx] + member);
		CButton* button = (CButton*)GetDlgItem(nCtrlId);

		if (button != NULL) button->SetCheck(nCheck);
	}
}





