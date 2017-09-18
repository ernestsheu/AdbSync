#pragma once


// CPackageDlg 對話方塊

class CPackageDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CPackageDlg)

public:
	CPackageDlg(CWnd* pParent = NULL);   // 標準建構函式
	virtual ~CPackageDlg();

// 對話方塊資料
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PACKAGE_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支援

protected:
	void GetPackagePoint(UINT pkdID, CPoint& outXY);


	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedPackageTakeBtn();
	afx_msg void OnCtrlRgn_Take_one_package_btn(UINT nID);
};
