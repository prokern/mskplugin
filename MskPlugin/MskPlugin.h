// MskPlugin.h : main header file for the MskPlugin DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CMskPluginApp
// See MskPlugin.cpp for the implementation of this class
//

class CMskPluginApp : public CWinApp
{
public:
	CMskPluginApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
