
// AdbControllerDlg.h : 標頭檔
//

#pragma once



typedef struct _AdbThreadData {
	CEdit*	wndStatus;
    CString	Serial;
	
	HANDLE	hExitEvent;
	HANDLE	hExitFinishedEvent;


	HANDLE	hTouchEvent;
    CString	TouchEvt;
	BOOL	bSync;
	CPoint 	ptTapXY;


	HANDLE	hMoveEvent;
	BOOL	bMoveSync;
	CPoint  ptStartXY;
	CPoint 	ptEndXY;
	UINT 	nMoveDuration;
	
} AdbThreadData, *PAdbThreadData;




class CAdbDevice {

public:
	int			DeviceIdx;
	HANDLE 		hThread;
	PAdbThreadData	PAdbThreadParam;

	
public:
	CAdbDevice& operator=(const CAdbDevice& device)
	{
		DeviceIdx = device.DeviceIdx;
		hThread = device.hThread;
		PAdbThreadParam = device.PAdbThreadParam;

		return( *this );
	}
};

// CAdbControllerDlg 對話方塊
class CAdbControllerDlg : public CDialogEx
{
// 建構
public:
	CAdbControllerDlg(CWnd* pParent = NULL);	// 標準建構函式
	~CAdbControllerDlg();

// 對話方塊資料
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ADBCONTROLLER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支援


// 程式碼實作
protected:
	HICON m_hIcon;

	// 產生的訊息對應函式
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();	
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnEndChildProcess(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSyncTouch(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnCloseSystemExecThread(WPARAM wParam, LPARAM lParam);
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnRun();
	afx_msg void OnBreak();
	afx_msg void OnKill();
	afx_msg void OnReuseCmd();
	afx_msg void OnCancel();
	afx_msg void OnCtrlRgn_Sync_Checkbox(UINT nID);
	afx_msg void OnCtrlRgn_Connect_Device(UINT nID);
	afx_msg void OnCtrlRgn_Detect_Touch_Event(UINT nID);
	afx_msg void OnCtrlRgn_Play_Event(UINT nID);
	afx_msg void OnCtrlRgn_Sync_Change(UINT nID);
	afx_msg void OnCtrlRgn_Move_Event(UINT nID);
	afx_msg void OnBnClickedCmdOutClear();
	afx_msg void OnBnClickedStatusOutClear();
	afx_msg void OnBnClickedConnectDevices();
	afx_msg void OnCtrlRgn_Device_Id_Changed(UINT nID);
	afx_msg void OnCtrlRgn_Touch_Evt_Changed(UINT nID);
	afx_msg void OnBnClickedDisconnectDevices();
	afx_msg void OnBnClickedMoveDegree();
	afx_msg void OnBnClickedMoveXy();
	
	DECLARE_MESSAGE_MAP()


private:
	void OnTouchSyncReceived(LPCTSTR pszText);
	void OnSyncTouchToSend(CString strTouchEventData);
	void OnSyncOneTouchToSend(CString strTouchEventData);
	void UpdateSyncList();
	void LoadProfile();
	void SaveProfile();	
	void BuildAdbDevice();
	void UpdateMove();
	
	int FindDeviceIndex(UINT nControlId, int member);
	void EnableDeviceControl(BOOL bEnable, int member);
	void CheckDeviceControl(int nCheck, int member);

private:
	CString m_sWorkingFolder;
	CMyConsolePipe* m_pLastCommand;
	
	BOOL m_bIsWindowsNT;
	BOOL m_bReusable;
	BOOL m_bDelayedQuit;
	BOOL m_bNeedUpdateSyncDeviceList;
	BOOL m_bNeedUpdateMoveSync;

	CEdit* m_wCommand;	
	CEdit* m_wEdit;
	CEdit* m_wStatus;
	int m_nStartChar;

	CPoint m_TapXY;
	CString m_SyncToucnRemainData;
	int m_SyncTouchDataOrder = 0;
			
    CList<CAdbDevice,CAdbDevice&> m_SyncDevices;

	CRITICAL_SECTION   m_SyncReceivedLock;
	CRITICAL_SECTION   m_SyncTouchLock;

	CPoint m_ptMove;
	UINT m_MoveDuration;
	UINT m_MoveLength;


public:


	
};
