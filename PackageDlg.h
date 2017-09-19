#pragma once


#define IDD_PACKIGE_DLG IDD_PACKAGE_DIALOG

// CPackageDlg ��ܤ��
class CPackageDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CPackageDlg)

public:
	CPackageDlg(CWnd* pParent, int nDeviceIdx, CString serial, UINT nNotifyCloseDlgEvent);   // �зǫغc�禡
	virtual ~CPackageDlg();

// ��ܤ�����
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PACKIGE_DLG };
#endif

	BOOL Create(void);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �䴩

private:
	void GetPackagePoint(int pkdID, CPoint& outXY);
	int FindPackageId(UINT nControlId, int member);
	void UpdateParameter();


private:
	
	CPoint m_ptStart;
	CSize m_szInterval;
	CList<CPoint,CPoint&> m_lstPkgPts;
	
	CRITICAL_SECTION   m_LockTackOnePackage;

	CString m_Serial;
	int m_nDeviceIdx;
	CWnd* m_pParent;	
	UINT m_nNotifyfCloseEvent;

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg LRESULT OnCloseSystemExecThread(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnTakeNextSystemExecThread(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedPackageTakeBtn();
	afx_msg void OnCtrlRgn_Take_one_package_btn(UINT nID);
	afx_msg void OnBnClickedPackageClearBtn();
};