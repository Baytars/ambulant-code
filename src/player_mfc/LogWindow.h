#pragma once


// LogWindow dialog

class CLogWindow : public CDialog
{
	DECLARE_DYNAMIC(CLogWindow)

private:
	CLogWindow(CWnd* pParent = NULL);   // standard constructor
	static CLogWindow *s_singleton;
public:
	static CLogWindow *GetLogWindowSingleton();
	virtual ~CLogWindow();

// Dialog Data
	enum { IDD = IDD_LOG_WINDOW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
