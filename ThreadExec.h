


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


typedef struct _SystemExecParam {
	CWnd*		wnd;
	CEdit*		wStatus;
	CButton*	btnPlay;
	CString		cmdLine;

	UINT	nNoityfCloseThreadEvent;
	
	HANDLE  hThread;
	
} SystemExecParam, *PSystemExecParam;



CString
ExecCmd(
    LPCTSTR cmd              // [in] command to execute
);

BOOL
ExecCmdSimple (
    LPCTSTR pProcessPath
);

DWORD 
AdbExecThread(
	LPVOID lpParam
);

DWORD
SystemExecuteThread(
	LPVOID lpParam
);




