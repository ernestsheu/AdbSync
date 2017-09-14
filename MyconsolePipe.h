#include "resource.h"

#define UWM_PIPEBROKEN (WM_APP+1) /* this could have been registered */

typedef void (CWnd::*WND_RECV_PMSG)(LPCTSTR pszText);

//#include "consolePipe.h"
class CMyConsolePipe : public CConsolePipe
{
public:
	CMyConsolePipe(BOOL bWinNT, CEdit* wOutput, CWnd* pParent, WND_RECV_PMSG wOnRecvMsg, DWORD flags) :
		CConsolePipe(bWinNT, wOutput, flags)
	{
		WndOnReceivedOutput = wOnRecvMsg;
		m_pParent = pParent;
	}

	virtual void OnReceivedOutput(LPCTSTR pszText)
	{


		if(!pszText) { // child has terminated
			//CWindow dlg(m_wndOutput->GetParent());
			//CWnd* pParent = m_wndOutput->GetParent();
			//ATLASSERT(dlg.IsWindow());

			// notify the main dialog that the process has ended
			m_pParent->SendMessage(UWM_PIPEBROKEN, (WPARAM)this);
			// note that this is called from the background thread context
			// soon this class will self-destruct
		}

		CConsolePipe::OnReceivedOutput(pszText); // normal screen echo
		(*m_pParent.*WndOnReceivedOutput)(pszText);
	}

private:
	WND_RECV_PMSG WndOnReceivedOutput = NULL;
	CWnd* m_pParent = NULL;
	
	
};
