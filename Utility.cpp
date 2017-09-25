


#include "stdafx.h"
#include "Utility.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



/*
return: output found count
*/
int SplitString(CString& content, TCHAR token, CStringList& outList)
{
	CString field;
	int index = 0;

	while (AfxExtractSubString(field, content, index, token)) {
		field.Remove(_T('\r'));
		field.Remove(_T('\n'));
		outList.AddTail(field);
		index++;
	}

	return (int)outList.GetCount();
}


#define BUF_SIZE 255

void OutputString(CEdit* pEdit, LPCTSTR pszFormat, ...)
{


	CString str;
	va_list argList;

	va_start( argList, pszFormat );
	str.FormatV( pszFormat, argList );
	va_end( argList );

	if(pEdit == NULL) {
		DbgString(str + _T("\n"));
		return;
	}


	// Add the text in the end
	// SetSel(-1, -1) just cancels selection, we need exact length to force caret @ end
	UINT nLen = (UINT)pEdit->SendMessage(WM_GETTEXTLENGTH);
	// ensure that the control won't be overflowed
	UINT nMax = pEdit->GetLimitText();
	ATLASSERT(nMax > BUF_SIZE);

	// Perhaps switch to a max line# setting
	if(nLen > nMax - BUF_SIZE) {
		// waste some characters from the top, not necessarily complete lines
		pEdit->SetSel(0, BUF_SIZE, TRUE);
		pEdit->ReplaceSel(_T(""));
		nLen -= BUF_SIZE;
		ATLASSERT(pEdit->SendMessage(WM_GETTEXTLENGTH) == (LRESULT)nLen);
	}
	
	str += _T("\r\n");
	pEdit->SetSel(nLen, nLen, FALSE /*scroll*/);
	pEdit->ReplaceSel(str);

}


CString& DbgString(LPCTSTR pszFormat, ...)
{
	CString str;
	va_list argList;

	va_start( argList, pszFormat );
	str.FormatV( pszFormat, argList );
	va_end( argList );
	
	OutputDebugString(str);

	return str;
}


