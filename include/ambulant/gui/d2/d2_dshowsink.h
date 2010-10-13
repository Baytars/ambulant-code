/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2010 Stichting CWI,
 * Science Park 123, 1098 XG Amsterdam, The Netherlands.
 *
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

/*
 * @$Id$
 */

#ifndef AMBULANT_GUI_D2_DSHOWSINK_H
#define AMBULANT_GUI_D2_DSHOWSINK_H

#include "ambulant/config/config.h"
//Every windows application needs to include this
#include <windows.h>
#include <dshow.h>
// NOTE: The next include file comes from sdkdir\Samples\multimedia\directshow\baseclasses.
// If you don't have it you should re-install the SDK and include the sample code.
#include "streams.h"

class CVideoD2DBitmapRenderer : public CBaseVideoRenderer
{

public:

	//-----------------------------------------------------------------------------
	// Define GUID for Texture Renderer {AB1B2AB5-18A0-49D5-814F-E2CB454D5D28}
	//-----------------------------------------------------------------------------
//	struct __declspec(uuid("{AB1B2AB5-18A0-49D5-814F-E2CB454D5D28}")) CLSID_TextureRenderer;

	CVideoD2DBitmapRenderer::CVideoD2DBitmapRenderer(LPUNKNOWN pUnk, HRESULT *phr);

	CVideoTextureRenderer::~CVideoTextureRenderer();

	HRESULT CVideoTextureRenderer::CheckMediaType(const CMediaType *pmt);
	
	HRESULT SetMediaType(const CMediaType *pmt);

	HRESULT DoRenderSample(IMediaSample * pSample );

#ifdef JNK
	void SetVideoTexture(LPDIRECT3DTEXTURE9* ppTexture);
#endif // JNK

#ifdef JNK
	BOOL m_bUseDynamicTextures;
#endif // JNK
	LONG m_lVidWidth;   // Video width
	LONG m_lVidHeight;  // Video Height
	LONG m_lVidPitch;   // Video Pitch

#ifdef JNK
	//The rendertarget where our renderer renders the video to
	LPDIRECT3DTEXTURE9 m_lpVideoTargetTexture;
	LPDIRECT3DSURFACE9 m_lpVideoTargetSurface;

	//this one is the texture and surface where the Video will be copied to via StretchRect,
	//when using dynamic textures.
	LPDIRECT3DTEXTURE9* m_ppVideoDestTexture;
	LPDIRECT3DSURFACE9 m_lpVideoDestSurface;
#endif

};

#endif // AMBULANT_GUI_D2_DSHOWSINK_H
