


#include "stdafx.h"
#include "ThreadExec.h"
#include "Utility.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



CString ExecCmd(
    LPCTSTR cmd              // [in] command to execute
)
{
	CString strResult;
	HANDLE hPipeRead, hPipeWrite;

	SECURITY_ATTRIBUTES saAttr = { sizeof(SECURITY_ATTRIBUTES) };
	saAttr.bInheritHandle = TRUE;   //Pipe handles are inherited by child process.
	saAttr.lpSecurityDescriptor = NULL;

	// Create a pipe to get results from child's stdout.
	if (!CreatePipe(&hPipeRead, &hPipeWrite, &saAttr, 0))
		return strResult;


	STARTUPINFO si = { sizeof(STARTUPINFO) };
	si.dwFlags = STARTF_USESTDHANDLES;// STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.hStdOutput = hPipeWrite;
	si.hStdError = hPipeWrite;
	si.wShowWindow = SW_HIDE;       // Prevents cmd window from flashing. Requires STARTF_USESHOWWINDOW in dwFlags.

	PROCESS_INFORMATION pi;

	LPTSTR szCmdline = _tcsdup(cmd);
	BOOL bSuccess = CreateProcess(NULL, szCmdline, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);


	if (bSuccess == FALSE) {
		CloseHandle(hPipeWrite);
		CloseHandle(hPipeRead);
		return strResult;
	}

	bool bProcessEnded = false;

	for (; !bProcessEnded;) {
		// Give some timeslice (50ms), so we won't waste 100% cpu.
		bProcessEnded = WaitForSingleObject(pi.hProcess, 50) == WAIT_OBJECT_0;

		// Even if process exited - we continue reading, if there is some data available over pipe.
		for (;;) {
			char buf[1024];
			DWORD dwRead = 0;
			DWORD dwAvail = 0;

			if (!::PeekNamedPipe(hPipeRead, NULL, 0, NULL, &dwAvail, NULL))
				break;

			if (!dwAvail) // no data available, return
				break;

			if (!::ReadFile(hPipeRead, buf, min(sizeof(buf) - 1, dwAvail), &dwRead, NULL) || !dwRead)
				// error, the child process might ended
				break;

			buf[dwRead] = 0;
			strResult += buf;
		}
	} //for

	CloseHandle(hPipeWrite);
	CloseHandle(hPipeRead);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	free(szCmdline);

	return strResult;
} //ExecCmd

/*
	
*/
BOOL ExecCmdSimple (
    LPCTSTR pProcessPath
)
{
	DWORD retVal = ERROR_SUCCESS;
	LPWSTR printString = NULL;

	STARTUPINFO startupInfo = {0};
	LPWSTR pCommandLine = NULL;
	BOOL processCreated = FALSE;
	BOOL bProcessEnd = FALSE;

	startupInfo.cb = sizeof(startupInfo);

	startupInfo.dwFlags		|= STARTF_USESTDHANDLES;
	startupInfo.hStdInput	= NULL;
	startupInfo.hStdOutput 	= NULL;
	startupInfo.hStdError 	= NULL;

	PROCESS_INFORMATION ProcessInfo;

	pCommandLine = _wcsdup (pProcessPath);

	processCreated = CreateProcess (
	                     NULL,           // No module name (use command line)
	                     pCommandLine,   // Command line
	                     NULL,           // Process handle not inheritable
	                     NULL,           // Thread handle not inheritable
	                     TRUE,          // Set handle inheritance to FALSE
	                     CREATE_NO_WINDOW,              // No creation flags
	                     NULL,           // Use parent's environment block
	                     NULL,           // Use parent's starting directory
	                     &startupInfo,   // Pointer to STARTUPINFO structure
	                     &ProcessInfo    // Pointer to PROCESS_INFORMATION structure
	                 );


	//WaitForSingleObject(ProcessInfo.hProcess, delaytime);
	while(!bProcessEnd) {
		bProcessEnd = WaitForSingleObject(ProcessInfo.hProcess, 50) == WAIT_OBJECT_0;
	} //for


	CloseHandle(ProcessInfo.hProcess);
	CloseHandle(ProcessInfo.hThread);
	ProcessInfo.hProcess = NULL;
	ProcessInfo.hThread = NULL;

	if (pCommandLine) {
		free (pCommandLine);
	}

	return processCreated;
}



DWORD
AdbExecThread(LPVOID lpParam)
{
	PAdbThreadData pDevice;
	HANDLE ghEvents[3];
	DWORD dwEvent;

	pDevice = (PAdbThreadData)lpParam;

	ghEvents[0] = pDevice->hTouchEvent;
	ghEvents[1] = pDevice->hMoveEvent;
	ghEvents[2] = pDevice->hExitEvent;

	while(TRUE) {
		dwEvent =
		    WaitForMultipleObjects(
		        3,         // number of objects in array
		        ghEvents,  // array of objects
		        FALSE,     // wait for any object
		        5000       // five-second wait
		    );

		switch (dwEvent) {
			// nTouchEvent
			case WAIT_OBJECT_0 + 0: {
				//DbgString(_T("First event was signaled.\n"));

				if(pDevice->bSync) {
					CString cmd;
					cmd.Format(_T("adb -s %s shell input tap %d %d "), pDevice->Serial, pDevice->ptTapXY.x, pDevice->ptTapXY.y);
					DbgString(cmd + _T("\n"));					
					OutputString(
					    pDevice->wndStatus,
					    _T("[%s] Tap( %d, %d)"),
					    pDevice->Serial,
					    pDevice->ptTapXY.x, pDevice->ptTapXY.y
					);
					ExecCmdSimple((LPCTSTR)cmd);
				}
			}
			break;

			// hMoveEvent
			case WAIT_OBJECT_0 + 1: {
				if(pDevice->bMoveSync) {
					CString cmd;
					cmd.Format( _T("adb -s %s shell input swipe %d %d %d %d %d"),
					            pDevice->Serial,
					            pDevice->ptStartXY.x, pDevice->ptStartXY.y,
					            pDevice->ptEndXY.x, pDevice->ptEndXY.y,
					            pDevice->nMoveDuration
					          );
					OutputString(
					    pDevice->wndStatus,
					    _T("[%s] Swipe( %d, %d, %d, %d) duration:%d ms"),
					    pDevice->Serial,
					    pDevice->ptStartXY.x, pDevice->ptStartXY.y,
					    pDevice->ptEndXY.x, pDevice->ptEndXY.y,
					    pDevice->nMoveDuration
					);
					ExecCmdSimple((LPCTSTR)cmd);
				}
			}
			break;

			// hExitEvent
			case WAIT_OBJECT_0 + 2:
				DbgString(_T("Exit event was signaled.\n"));
				goto exit;

			case WAIT_TIMEOUT:
				//DbgString(_T("Wait timed out.\n"));
				break;

			// Return value is invalid.
			default:
			
				CString dbgString = DbgString(_T("[%s] Wait error: evt:%d err:%d\n"), pDevice->Serial, dwEvent, GetLastError());
				OutputString( pDevice->wndStatus, dbgString);
				ExitProcess(0);
		}
	}


exit:
	SetEvent(pDevice->hExitFinishedEvent);
	return 0;
	
}





DWORD
SystemExecuteThread(LPVOID lpParam)
{	
	PSystemExecParam pParam = (PSystemExecParam)lpParam;

	ASSERT(pParam->wnd != NULL);

	if(pParam->btnPlay != NULL) pParam->btnPlay->EnableWindow(FALSE);

	//int retval = ::_tsystem(pParam->cmdLine);
	int retval = ExecCmdSimple(pParam->cmdLine);
	OutputString( pParam->wStatus, _T("Play script[%s], ret:%d"), pParam->cmdLine, retval);

	if(pParam->btnPlay != NULL) pParam->btnPlay->EnableWindow(TRUE);

	pParam->wnd->PostMessage(pParam->nNoityfCloseThreadEvent, (WPARAM)pParam, 0);

	return 0;
}

