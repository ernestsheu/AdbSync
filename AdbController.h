
// AdbController.h : PROJECT_NAME ���ε{�����D�n���Y��
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�� PCH �]�t���ɮ׫e���]�t 'stdafx.h'"
#endif

#include "resource.h"		// �D�n�Ÿ�


// CAdbControllerApp: 
// �аѾ\��@�����O�� AdbController.cpp
//

class CAdbControllerApp : public CWinApp
{
public:
	CAdbControllerApp();

// �мg
public:
	virtual BOOL InitInstance();

// �{���X��@

	DECLARE_MESSAGE_MAP()
};

extern CAdbControllerApp theApp;