#pragma once


// LogWindow dialog

class CLogWindow : public CDialog
{
	DECLARE_DYNAMIC(CLogWindow)

public:
	CLogWindow(CWnd* pParent = NULL);   // standard constructor
	virtual ~CLogWindow();

// Dialog Data
	enum { IDD = IDD_LOG_WINDOW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
