#pragma once

#include "CaseTreeData.h"
#include "Tester.h"

class CWgTester :	public CTester
{
	HANDLE m_hRxThread;
	HANDLE m_hWgThread;
public:
	CWgTester(void);
	~CWgTester(void);
public:
	//��������
};

extern CWgTester g_WgTester;