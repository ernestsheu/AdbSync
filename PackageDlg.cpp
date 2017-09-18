// PackageDlg.cpp : 實作檔
//

#include "stdafx.h"
#include "AdbController.h"
#include "PackageDlg.h"
#include "afxdialogex.h"


#define DEF_PKG_COL 5
#define DEF_PKG_ROW 4


// CPackageDlg 對話方塊

IMPLEMENT_DYNAMIC(CPackageDlg, CDialogEx)

CPackageDlg::CPackageDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_PACKAGE_DIALOG, pParent)
{

}

CPackageDlg::~CPackageDlg()
{
}

void CPackageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPackageDlg, CDialogEx)
	ON_BN_CLICKED(IDC_PACKAGE_TAKE_BTN, &CPackageDlg::OnBnClickedPackageTakeBtn)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_PACKAGE_BTN1, IDC_PACKAGE_BTN20, &CPackageDlg::OnCtrlRgn_Take_one_package_btn)
END_MESSAGE_MAP()


// CPackageDlg 訊息處理常式
void CPackageDlg::OnBnClickedPackageTakeBtn()
{

}


void CPackageDlg::GetPackagePoint(UINT pkdID, CPoint& outXY)
{

	CEdit* editor;
	CString text;
	
	CPoint ptStart;
	CSize ptSize;
	
	editor = (CEdit*)GetDlgItem(IDC_PKG_START_X);
	editor->GetWindowText(text);
	ptStart.x = _wtoi((LPCTSTR)text);
	
	editor = (CEdit*)GetDlgItem(IDC_PKG_START_Y);
	editor->GetWindowText(text);
	ptStart.y = _wtoi((LPCTSTR)text);

	editor = (CEdit*)GetDlgItem(IDC_PKG_INV_W);
	editor->GetWindowText(text);
	ptSize.cx = _wtoi((LPCTSTR)text);

	editor = (CEdit*)GetDlgItem(IDC_PKG_INV_H);
	editor->GetWindowText(text);
	ptSize.cy = _wtoi((LPCTSTR)text);


	CPoint ptPkg;

	outXY.x = ptStart.x + ((pkdID % DEF_PKG_COL) * ptSize.cx);
	outXY.y = ptStart.y + ((pkdID / DEF_PKG_COL) * ptSize.cy);

	
	
} 

void CPackageDlg::OnCtrlRgn_Take_one_package_btn(UINT nID)
{
	CString temp;
	temp.Format(_T("OnCtrlRgn_Take_one_package_btn(%d)\n"), nID);
	OutputDebugString(temp);


	CPoint ptPkg;
	GetPackagePoint(nID - IDC_PACKAGE_BTN1, ptPkg);
	CString cmd;
	cmd.Format("adb shell input tap %d %d", ptPkg.x, ptPkg.y);
	int retval = ::_tsystem(cmd);
	//OutputString( pParam->wStatus, _T("Play script[%s], ret:%d"), pParam->cmdLine, retval);
	
}

