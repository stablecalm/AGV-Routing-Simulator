// iTLC_AHV.h : iTLC_AHV ���� ���α׷��� ���� �� ��� ����
//
#pragma once

#ifndef __AFXWIN_H__
	#error PCH���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����Ͻʽÿ�.
#endif

#include "resource.h"       // �� ��ȣ


// CiTLC_AHVApp:
// �� Ŭ������ ������ ���ؼ��� iTLC_AHV.cpp�� �����Ͻʽÿ�.
//

class CiTLC_AHVApp : public CWinApp
{
public:
	CiTLC_AHVApp();


// ������
public:
	virtual BOOL InitInstance();

// ����
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CiTLC_AHVApp theApp;
