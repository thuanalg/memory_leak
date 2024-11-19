
// DemoCounting.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CDemoCountingApp:
// See DemoCounting.cpp for the implementation of this class
//

class CDemoCountingApp : public CWinApp
{
public:
	CDemoCountingApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CDemoCountingApp theApp;
