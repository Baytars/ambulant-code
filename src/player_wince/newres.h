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

#ifndef __NEWRES_H__
#define __NEWRES_H__

#define  SHMENUBAR RCDATA
#if !(defined(_WIN32_WCE_PSPC) && (_WIN32_WCE >= 300))
	#undef HDS_HORZ  
	#undef HDS_BUTTONS 
	#undef HDS_HIDDEN 

	#include <commctrl.h>
	// for MenuBar
	#define I_IMAGENONE		(-2)
	#define NOMENU			0xFFFF
	#define IDS_SHNEW		1
	#define IDM_SHAREDNEW        10
	#define IDM_SHAREDNEWDEFAULT 11

	// for Tab Control
	#define TCS_SCROLLOPPOSITE      0x0001   // assumes multiline tab
	#define TCS_BOTTOM              0x0002
	#define TCS_RIGHT               0x0002
	#define TCS_VERTICAL            0x0080
	#define TCS_MULTISELECT         0x0004  // allow multi-select in button mode
	#define TCS_FLATBUTTONS         0x0008	
#endif //_WIN32_WCE_PSPC


#endif //__NEWRES_H__