// PreferencesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AmbulantPlayer.h"
#include "LogWindow.h"


// PreferencesDlg dialog

IMPLEMENT_DYNAMIC(CLogWindow, CDialog)
CLogWindow::CLogWindow(CWnd* pParent /*=NULL*/)
	: CDialog(CLogWindow::IDD, pParent)
{
}

CLogWindow::~CLogWindow()
{
}

void CLogWindow::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CLogWindow, CDialog)
//	ON_CBN_SELCHANGE(IDC_COMBO1, OnCbnSelchangeCombo1)
//	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
//	ON_BN_CLICKED(IDOK, OnBnClickedOK)
END_MESSAGE_MAP()


// CLogWindow message handlers
