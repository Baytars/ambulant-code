/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: NPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is 
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or 
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If >you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the NPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the NPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

//////////////////////////////////////////////////
//
// CPlugin class implementation
//
#ifdef XP_WIN
#include <windows.h>
#include <windowsx.h>
#endif

#ifdef XP_MAC
#include <TextEdit.h>
#endif

#ifdef XP_UNIX
#include <string.h>
#endif

#include "plugin.h"
#include "npupp.h"


NPIdentifier sStartPlayer_id;
NPIdentifier sStopPlayer_id;
NPIdentifier sPausePlayer_id;
NPIdentifier sResumePlayer_id;
NPIdentifier sRestartPlayer_id;
NPIdentifier sIsDone_id;

NPIdentifier sFoo_id;
NPIdentifier sBar_id;
NPIdentifier sDocument_id;
NPIdentifier sBody_id;
NPIdentifier sCreateElement_id;
NPIdentifier sCreateTextNode_id;
NPIdentifier sAppendChild_id;
NPIdentifier sPluginType_id;
NPObject *sWindowObj;

#ifdef XP_WIN
static LRESULT CALLBACK PluginWinProc(HWND, UINT, WPARAM, LPARAM);
static WNDPROC lpOldProc = NULL;
#endif
#ifdef XP_WIN
static LRESULT CALLBACK PluginWinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg) {
    case WM_PAINT:
      {
        // draw a frame and display the string
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        RECT rc;
        GetClientRect(hWnd, &rc);
        FrameRect(hdc, &rc, GetStockBrush(BLACK_BRUSH));
        CPlugin * p = (CPlugin *)GetWindowLong(hWnd, GWL_USERDATA);
        if(p) {
          if (p->m_String[0] == 0) {
            strcpy("foo", p->m_String);
          }

          DrawText(hdc, p->m_String, strlen(p->m_String), &rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
        }

        EndPaint(hWnd, &ps);
      }
      break;
    default:
      break;
  }

  return DefWindowProc(hWnd, msg, wParam, lParam);
}
#endif
