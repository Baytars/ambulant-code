/*
 * 
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 * 
 * Copyright (C) 2003 Stiching CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
 * 
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * In addition, as a special exception, if you link Ambulant Player with
 * other files to produce an executable, this library does not by itself
 * cause the resulting executable to be covered by the GNU General Public
 * License. This exception does not however invalidate any other reason why
 * the executable file might be covered by the GNU General Public License.
 * 
 * As a special exception, the copyright holders of Ambulant Player give
 * you permission to link Ambulant Player with independent modules that
 * communicate with Ambulant Player solely through the region and renderer
 * interfaces, regardless of the license terms of these independent
 * modules, and to copy and distribute the resulting combined work under
 * terms of your choice, provided that every copy of the combined work is
 * accompanied by a complete copy of the source code of Ambulant Player
 * (the version of Ambulant Player used to produce the combined work),
 * being distributed under the terms of the GNU General Public License plus
 * this exception.  An independent module is a module which is not derived
 * from or based on Ambulant Player.
 * 
 * Note that people who make modified versions of Ambulant Player are not
 * obligated to grant this special exception for their modified versions;
 * it is their choice whether to do so.  The GNU General Public License
 * gives permission to release a modified version without this exception;
 * this exception also makes it possible to release a modified version
 * which carries forward this exception. 
 * 
 */

// AmbulantPlayer.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "AmbulantPlayer.h"
#include "MainFrm.h"

#include "MmDoc.h"
#include "MmView.h"
#include "OpenUrlDlg.h"
#include "PreferencesDlg.h"
#include "LogWindow.h"

#include "ambulant/version.h"
#include ".\ambulantplayer.h"
#include "mypreferences.h"

using namespace ambulant;

#pragma comment (lib,"libjpeg.lib")
#pragma comment (lib,"libexpat.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAmbulantPlayerApp

BEGIN_MESSAGE_MAP(CAmbulantPlayerApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_OPEN, CAmbulantPlayerApp::OnFileOpen)
	ON_COMMAND_EX_RANGE(ID_FILE_MRU_FILE1, ID_FILE_MRU_FILE1+16, OnOpenRecentFile)
	ON_COMMAND(ID_FILE_OPENURL, OnFileOpenurl)
	ON_COMMAND(ID_FILE_PREFERENCES, OnPreferences)
	ON_COMMAND(ID_VIEW_LOG, OnViewLog)
END_MESSAGE_MAP()


// CAmbulantPlayerApp construction

CAmbulantPlayerApp::CAmbulantPlayerApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CAmbulantPlayerApp object

CAmbulantPlayerApp theApp;

//////////////////////
// CAmbulantPlayerApp initialization

BOOL CAmbulantPlayerApp::InitInstance()
{
	CWinApp::InitInstance();

	// Initialize OLE libraries
	if (!AfxOleInit()) {
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();
	
	// Enable RichEdit control
	AfxInitRichEdit2();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Ambulant"));
	
	LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)
	// Install our preferences handler
	mypreferences::install_singleton();
	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	m_pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(MmDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(MmView));
	if (!m_pDocTemplate)
		return FALSE;
	AddDocTemplate(m_pDocTemplate);
	// Parse command line for standard shell commands, DDE, file open
	//CCommandLineInfo cmdInfo;
	CAmCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	
	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	//if(!ProcessShellCommand(cmdInfo))
	//	return FALSE;
	
	MmDoc *mmdoc = 0;
	switch (cmdInfo.m_nShellCommand) {
		case CCommandLineInfo::FileNew:
			if (!AfxGetApp()->OnCmdMsg(ID_FILE_NEW, 0, NULL, NULL))
				OnFileNew();
			if (m_pMainWnd == NULL)
				return FALSE;
			break;
		case CCommandLineInfo::FileOpen:
			mmdoc = (MmDoc *) OpenDocumentFile(cmdInfo.m_strFileName);
			if(!mmdoc) return FALSE;
			break;
	}

	// The one and only window has been initialized, so show and update it
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();
	m_pMainWnd->DragAcceptFiles(TRUE);
	// call DragAcceptFiles only if there's a suffix
	//  In an SDI app, this should occur after ProcessShellCommand
	
	if(mmdoc /*&& cmdInfo.m_autostart*/) {
		mmdoc->StartPlayback();
	}
		
	return TRUE;
}



// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
public:
	CString m_version;	
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD), m_version(_T(""))
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_VERSION, m_version);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// App command to run the dialog
void CAmbulantPlayerApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.m_version.Format(TEXT("Ambulant Player version %s\n"), get_version());	
	aboutDlg.DoModal();
}

// CAmbulantPlayerApp message handlers
CDocument* CAmbulantPlayerApp::OpenDocumentFile(LPCTSTR lpszFileName)
{
	OnFileNew();
	return m_pDocTemplate->OpenDocumentFile(lpszFileName);
}

void CAmbulantPlayerApp::OnFileOpen()
{
	CString newName;
	if(!DoPromptFileName(newName, AFX_IDS_OPENFILE,
	  OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, TRUE, NULL))
		return; // open cancelled
	MmDoc *mmdoc = (MmDoc *) OpenDocumentFile(newName);
	if(mmdoc) mmdoc->StartPlayback();
}

BOOL CAmbulantPlayerApp::OnOpenRecentFile(UINT nID)
{
	ASSERT_VALID(this);
	ASSERT(m_pRecentFileList != NULL);
	ASSERT(nID >= ID_FILE_MRU_FILE1);
	ASSERT(nID < ID_FILE_MRU_FILE1 + (UINT)m_pRecentFileList->GetSize());
	int nIndex = nID - ID_FILE_MRU_FILE1;
	ASSERT((*m_pRecentFileList)[nIndex].GetLength() != 0);
	MmDoc *mmdoc = (MmDoc *) OpenDocumentFile((*m_pRecentFileList)[nIndex]);
	if(!mmdoc) m_pRecentFileList->Remove(nIndex);
	if(mmdoc) mmdoc->StartPlayback();
	return TRUE;
}

void CAmbulantPlayerApp::OnFileOpenurl()
{
	COpenUrlDlg dlg;
	dlg.m_url = m_recentUrl;
	if(dlg.DoModal() != IDOK) return;
	
	// validate entered URL
	// ...
	if(dlg.m_url.IsEmpty())
		return;
			
	// Open URL
	m_recentUrl = dlg.m_url;
	MmDoc *mmdoc = (MmDoc *) OpenDocumentFile(dlg.m_url);
	if(mmdoc) mmdoc->StartPlayback();
}

void CAmbulantPlayerApp::OnPreferences()
{
	PreferencesDlg dlg;
	if(dlg.DoModal() != IDOK) return;
}

void CAmbulantPlayerApp::OnViewLog()
{
	if (m_logwindow == NULL) {
		m_logwindow = CLogWindow::GetLogWindowSingleton();
	}
	m_logwindow->ShowWindow(SW_SHOW);
}
