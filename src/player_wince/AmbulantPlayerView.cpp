// AmbulantPlayerView.cpp : implementation of the CAmbulantPlayerView class
//

#include "stdafx.h"
#include "AmbulantPlayer.h"

#include "AmbulantPlayerDoc.h"
#include "AmbulantPlayerView.h"

// DG Player
#include "ambulant/gui/dg/dg_player.h"
#include "ambulant/gui/dg/dg_wmuser.h"

#include "ambulant/common/preferences.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/textptr.h"
#include "ambulant/lib/win32/win32_fstream.h"

#pragma comment (lib,"mp3lib.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// The handle of the single instance
static HWND s_hwnd;

HWND new_os_window() {
	// Return the handle of the single instance for now
	// This means paint bits of the new window
	// to the single instance
	return s_hwnd;
}

void destroy_os_window(HWND hwnd) {
	// none for now; keep the single instance
}

using namespace ambulant;

typedef gui::dg::dg_player gui_player;

static gui_player* 
create_player_instance(const TCHAR *url) {
	return new gui_player(lib::textptr(url).str());
}

static  gui_player *player = 0;
static needs_done_redraw = false;

/////////////////////////////////////////////////////////////////////////////
// CAmbulantPlayerView

IMPLEMENT_DYNCREATE(CAmbulantPlayerView, CView)

BEGIN_MESSAGE_MAP(CAmbulantPlayerView, CView)
	//{{AFX_MSG_MAP(CAmbulantPlayerView)
	ON_COMMAND(ID_PLAY, OnPlay)
	ON_UPDATE_COMMAND_UI(ID_PLAY, OnUpdatePlay)
	ON_COMMAND(ID_PAUSE, OnPause)
	ON_UPDATE_COMMAND_UI(ID_PAUSE, OnUpdatePause)
	ON_COMMAND(ID_STOP, OnStop)
	ON_UPDATE_COMMAND_UI(ID_STOP, OnUpdateStop)
	ON_WM_DESTROY()
	ON_WM_CHAR()
	ON_WM_LBUTTONDOWN()
	ON_WM_CREATE()
	ON_WM_TIMER()
	ON_COMMAND(ID_HELP_WELCOME, OnHelpWelcome)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAmbulantPlayerView construction/destruction

CAmbulantPlayerView::CAmbulantPlayerView()
{
	m_timer_id = 0;
	m_cursor_id = 0;
	m_autoplay = false;
}

CAmbulantPlayerView::~CAmbulantPlayerView()
{
}

BOOL CAmbulantPlayerView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

int CAmbulantPlayerView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	m_timer_id = SetTimer(1, 1000, 0);
	
	// Set static handle
	s_hwnd = GetSafeHwnd();
	

	lib::win32::fstream *fs = new lib::win32::fstream();
	if(fs->open_for_writing(TEXT("\\Windows\\Ambulant\\amlog.txt")))
		lib::logger::get_logger()->set_ostream(fs);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CAmbulantPlayerView drawing

void CAmbulantPlayerView::OnDraw(CDC* pDC)
{
	//CAmbulantPlayerDoc* pDoc = GetDocument();
	//ASSERT_VALID(pDoc);
	if(player)
		player->redraw(m_hWnd, pDC->m_hDC);
}

/////////////////////////////////////////////////////////////////////////////
// CAmbulantPlayerView diagnostics

#ifdef _DEBUG
void CAmbulantPlayerView::AssertValid() const
{
	CView::AssertValid();
}

void CAmbulantPlayerView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CAmbulantPlayerDoc* CAmbulantPlayerView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CAmbulantPlayerDoc)));
	return (CAmbulantPlayerDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CAmbulantPlayerView message handlers

void CAmbulantPlayerView::SetMMDocument(LPCTSTR lpszPathName) {
	gui_player *dummy = player;
	player = 0;
	if(dummy) {
		dummy->stop();
		delete dummy;
	}
	dummy = create_player_instance(lib::textptr(lpszPathName));
	m_curPathName = lpszPathName;
	player = dummy;
	if(m_autoplay)
		PostMessage(WM_COMMAND, ID_PLAY);
}

void CAmbulantPlayerView::OnPlay() 
{
	if(player) {
		player->start();
		needs_done_redraw = true;
	}
	
}

void CAmbulantPlayerView::OnUpdatePlay(CCmdUI* pCmdUI) 
{
	bool enable = player && !player->is_playing();
	pCmdUI->Enable(enable?TRUE:FALSE);
	
}

void CAmbulantPlayerView::OnPause() 
{
	if(player) player->pause();
	
}

void CAmbulantPlayerView::OnUpdatePause(CCmdUI* pCmdUI) 
{
	bool enable = player && (player->is_playing() || player->is_pausing());
	pCmdUI->Enable(enable?TRUE:FALSE);
	if(enable) pCmdUI->SetCheck(player->is_pausing()?1:0);
	
}

void CAmbulantPlayerView::OnStop() 
{
	if(player) {
		player->stop();
		player->on_done();
		InvalidateRect(NULL);
		PostMessage(WM_INITMENUPOPUP,0, 0); 
		needs_done_redraw = false;
	}
	
}

void CAmbulantPlayerView::OnUpdateStop(CCmdUI* pCmdUI) 
{
	bool enable = player && (player->is_playing() || player->is_pausing());
	pCmdUI->Enable(enable?TRUE:FALSE);
	
}

void CAmbulantPlayerView::OnDestroy() 
{
	if(player) {
		player->stop();
		delete player;
		player = 0;
	}
	lib::logger::get_logger()->set_ostream(0);
	if(m_timer_id) KillTimer(m_timer_id);
	CView::OnDestroy();
	
}

void CAmbulantPlayerView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if(player) player->on_char(nChar);
	CView::OnChar(nChar, nRepCnt, nFlags);
}

void CAmbulantPlayerView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if(player) player->on_click(point.x, point.y, GetSafeHwnd());
	CView::OnLButtonDown(nFlags, point);
}


void CAmbulantPlayerView::OnTimer(UINT nIDEvent) 
{
	if(player && needs_done_redraw && player->is_done()) {
		player->on_done();
		InvalidateRect(NULL);
		PostMessage(WM_INITMENUPOPUP,0, 0); 
		needs_done_redraw = false;
	}
	
	CView::OnTimer(nIDEvent);
}

void CAmbulantPlayerView::OnHelpWelcome() 
{
	CString welcomeFilename("\\Windows\\Ambulant\\Welcome.smil");
	SetMMDocument(welcomeFilename);
	if(!m_autoplay)
		PostMessage(WM_COMMAND, ID_PLAY);

}
