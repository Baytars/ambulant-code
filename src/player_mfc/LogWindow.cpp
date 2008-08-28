// PreferencesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AmbulantPlayer.h"
#include "LogWindow.h"
#include "ambulant/gui/dx/dx_wmuser.h"

// logwindow_ostream implementation

int
logwindow_ostream::write(const char *cstr)
{
#ifdef _DEBUG
	TRACE("%s", cstr);
#endif
	CLogWindow *lwin = CLogWindow::GetLogWindowSingleton();
	lwin->AppendText(cstr);
	return 1;
}

// CLogWindow dialog

IMPLEMENT_DYNAMIC(CLogWindow, CDialog)
CLogWindow::CLogWindow(CWnd* pParent /*=NULL*/)
	: CDialog(CLogWindow::IDD, pParent)
{
}

CLogWindow *CLogWindow::s_singleton = NULL;

CLogWindow *
CLogWindow::GetLogWindowSingleton()
{
	if (s_singleton == 0) {
		s_singleton = new CLogWindow(NULL);
		s_singleton->Create(IDD);
	}
	return s_singleton;
}

CLogWindow::~CLogWindow()
{
}

void CLogWindow::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RICHEDIT21, m_richedit);
}


BEGIN_MESSAGE_MAP(CLogWindow, CDialog)
//	ON_CBN_SELCHANGE(IDC_COMBO1, OnCbnSelchangeCombo1)
//	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
//	ON_BN_CLICKED(IDOK, OnBnClickedOK)
	ON_EN_CHANGE(IDC_RICHEDIT21, OnEnChangeRichedit21)
#ifdef ON_THREADSAFE_LOG
	ON_MESSAGE(WM_AMBULANT_MESSAGE, OnAmbulantMessage)
#endif
END_MESSAGE_MAP()


// CLogWindow message handlers

void
CLogWindow::AppendText(const char *data)
{
#ifdef THREAD_SAFE_LOG
	char *data_copy = strdup(data);
	PostMessage(WM_AMBULANT_MESSAGE, 0, (LPARAM)data_copy);
#else
	USES_CONVERSION;
	m_richedit.SetSel(-1, -1);
	m_richedit.ReplaceSel(A2CT(data));
#endif // THREAD_SAFE_LOG
}

#ifdef THREAD_SAFE_LOG
LPARAM
CLogWindow::OnAmbulantMessage(WPARAM wParam, LPARAM lParam)
{
	USES_CONVERSION;
	char *data = (char *)lParam;
	m_richedit.SetSel(-1, -1);
	m_richedit.ReplaceSel(A2CT(data));
	free(data);
	return 0;
}
#endif // THREAD_SAFE_LOG

void CLogWindow::OnEnChangeRichedit21()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}
