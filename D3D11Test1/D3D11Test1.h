
// D3D11Test1.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CD3D11Test1App: 
// �йش����ʵ�֣������ D3D11Test1.cpp
//

class CD3D11Test1App : public CWinApp
{
public:
	CD3D11Test1App();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CD3D11Test1App theApp;