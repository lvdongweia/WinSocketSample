
// SNBurning.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CSNBurningApp:
// �йش����ʵ�֣������ SNBurning.cpp
//

class CSNBurningApp : public CWinApp
{
public:
	CSNBurningApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CSNBurningApp theApp;